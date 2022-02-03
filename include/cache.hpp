#ifndef __hemlock_cache_hpp
#define __hemlock_cache_hpp

namespace hemlock {
    namespace io {
        class IOManager;
    }

    template <typename CacheCandidateType>
    concept Cacheable = std::move_constructible<CacheCandidateType>;

    template <Cacheable CachedType, MapContainer ContainerType>
        requires std::is_same_v<ContainerType::mapped_type, CachedType>
    class Cache {
    public:
            using Parser = Delegate<CachedType(const hio::fs::path&)>;

            Cache()  { /* Empty. */ }
            ~Cache() { /* Empty. */ }

            void init(io::IOManager* iomanager, Parser parser) {
                if (m_initialised) return;
                m_initialised = true;

                m_iomanager = iomanager;
                m_parser    = parser;
            }
            void dispose() {
                if (!m_initialised) return;
                m_initialised = false;

                if constexpr (Disposable<CachedType>) {
                    for (const auto& [_, value] : m_assets) {
                        value.dispose();
                    }
                }
                ContainerType().swap(m_assets);

                m_iomanager = nullptr;
                Parser().swap(m_parser);
            }

            /**
             * @brief Load an asset into this cache.
             *
             * @param filepath The filepath to the asset.
             *
             * @return True if the asset was loaded, false if not.
             */
            bool preload(const hio::fs::path& filepath) {
                auto& it = std::find(m_assets.begin(), m_assets.end(), filepath.string());
                if (it != m_assets.end()) return false;

                m_assets.emplace(m_parser(filepath));

                return true;
            }

            /**
             * @brief Load assets into this cache.
             *
             * @param filepaths The filepaths to the assets.
             *
             * @return The number of assets preloaded.
             */
            ui32 preload(std::span<hio::fs::path> filepaths) {
                ui32 count = 0;
                for (auto& filepath : filepaths) {
                    if (preload(filepath)) ++count;
                }
                return count;
            }

            /**
             * @brief Load assets into this cache.
             *
             * @param globpath The filepath to the asset(s).
             * Preload_glob allows filepath to be a glob string
             * templating for multiple assets.
             *
             * @return The number of assets preloaded.
             */
            ui32 preload_glob(const hio::fs::path& globpath) {
                return m_iomanager->apply_to_globpath(
                    globpath,
                    [&](const hio::fs::path& filepath) {
                        return this->preload(filepath);
                    }
                );
            }

            /**
             * @brief Load an asset into this cache.
             *
             * @param globpaths The filepaths to the assets.
             * Preload_glob allows filepath to be a glob string
             * templating for multiple assets.
             *
             * @return The number of assets preloaded.
             */
            ui32 preload_glob(std::span<hio::fs::path> globpaths) {
                ui32 count = 0;
                for (auto& globpath : globpaths) {
                    count += preload_glob(globpath);
                }
                return count;
            }

            /**
             * @brief Attempt to find asset in cache with the given
             * filepath. If not found, load the asset from disk and
             * place it into the cache.
             *
             * @param filepath The filepath to the asset.
             *
             * @return The asset fetched.
             */
            CachedType& fetch(const hio::fs::path& filepath) {
                auto& it = std::find(m_assets.begin(), m_assets.end(), filepath.string());
                if (it != m_assets.end()) return (*it).second;

                auto& [asset, fetched] = m_assets.emplace(filepath.string(), m_parser(filepath));

#ifdef DEBUG
                assert(fetched);
#endif

                return (*asset).second;
            }
    protected:
        bool m_initialised;

        io::IOManager* m_iomanager;
        Parser         m_parser;

        ContainerType m_assets;
    };
}

#endif // __hemlock_cache_hpp
