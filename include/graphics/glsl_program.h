#ifndef __hemlock_graphics_glsl_program_h
#define __hemlock_graphics_glsl_program_h

#include "io/serialisation.hpp"
#include "io/yaml.hpp"

#if defined(HEMLOCK_USING_OPENGL)
/**
 * @brief Enumerates the types of shader.
 */
H_DECL_VENUM_WITH_SERIALISATION(
    hemlock::graphics,
    ShaderType,
    ui32,
    (FRAGMENT, GL_FRAGMENT_SHADER),
    (VERTEX, GL_VERTEX_SHADER)
)
#endif  // defined(HEMLOCK_USING_OPENGL)

/**
 * @brief Information needed to compile a shader.
 */
H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::graphics,
    ShaderInfo,
    (type, hemlock::graphics::ShaderType),
    (filepath, std::string)
)

/**
 * @brief Success status of creating a shader.
 */
H_DECL_ENUM_WITH_SERIALISATION(
    hemlock::graphics,
    ShaderCreationResult,
    ui8,
    SUCCESS,
    NON_EDITABLE,
    VERTEX_EXISTS,
    FRAG_EXISTS,
    INVALID_STAGE,
    CREATE_FAIL,
    READ_FAIL,
    COMPILE_FAIL
)

/**
 * @brief Success status of creating a vertex-fragment
 * shader pair.
 */
H_DECL_STRUCT_WITH_SERIALISATION(
    hemlock::graphics,
    ShaderCreationResults,
    (vertex, hemlock::graphics::ShaderCreationResult),
    (fragment, hemlock::graphics::ShaderCreationResult)
)

/**
 * @brief Success status of linking a vertex-fragment
 * shader pair.
 */
H_DECL_ENUM_WITH_SERIALISATION(
    hemlock::graphics,
    ShaderLinkResult,
    ui8,
    SUCCESS,
    NON_EDITABLE,
    VERTEX_MISSING,
    FRAG_MISSING,
    LINK_FAIL
)

namespace hemlock {
    namespace graphics {
        using ShaderAttribute    = std::pair<std::string, GLuint>;
        using ShaderAttributes   = std::vector<ShaderAttribute>;
        using ShaderAttributeMap = std::map<std::string, GLuint>;

        class ShaderCache :
            public hio::
                Cache<std::string, std::unordered_map<std::string, std::string>> { };

        class GLSLProgram {
        public:
            GLSLProgram();

            ~GLSLProgram() { /* Empty */
            }

            /**
             * @brief Initialises a shader program.
             *
             * @param shader_cache The cache through which to obtain
             * shader code.
             */
            void init(ShaderCache* shader_cache);
            /**
             * @brief Disposes of a shader program.
             */
            void dispose();

            /**
             * @brief Returns the ID of the shader program.
             *
             * @return The ID of the shader program.
             */
            GLuint id() const { return m_id; }

            bool initialised() const { return m_id != 0; }

            bool linked() const { return m_is_linked; }

            bool editable() const { return !linked() && initialised(); }

            bool in_use() const { return m_id == GLSLProgram::current; }

            /**
             * @brief Adds a shader to the program.
             *
             * @param shader The information regarding the shader to be added.
             *
             * @return True if the shader is successfully added, false otherwise.
             */
            ShaderCreationResult add_shader(const ShaderInfo& shader);
            /**
             * @brief Adds both a vertex and a fragment shader to the program.
             *
             * @param vertex_path The filepath of the vertex shader to be added.
             * @param fragment_path The filepath of the fragment shader to be added.
             *
             * @return True if the shaders are successfully added, false otherwise.
             */
            ShaderCreationResults add_shaders(
                const std::string& vertex_path, const std::string& fragment_path
            );

            /**
             * @brief Links the shaders to the shader program.
             *
             * @return True if the shaders are successfully linked, false otherwise.
             */
            ShaderLinkResult link();

            /**
             * @brief Sets an attribute with the given name to the given index.
             */
            bool set_attribute(const std::string& name, GLuint index);
            /**
             * @brief Sets an attribute with the given name to the given index.
             */
            bool set_attribute(const ShaderAttribute& attribute);
            /**
             * @brief Sets all attributes provided.
             */
            bool set_attributes(const ShaderAttributes& attributes);

            // TODO(Matthew): If we add shaders from source instead of filepath, could
            // parse location of uniforms that explicitly set it using
            // "layout(location = X)".
            GLuint attribute_location(const std::string& name) const {
                return m_attributes.at(name);
            }

            GLuint uniform_location(const std::string& name) const;

            void enable_vertex_attrib_arrays(GLuint vao) const;
            void disable_vertex_attrib_arrays(GLuint vao) const;
            bool enable_vertex_attrib_array(GLuint vao, const std::string& name) const;
            bool disable_vertex_attrib_array(GLuint vao, const std::string& name) const;

            /**
             * @brief Uses this shader program.
             */
            void use();
            /**
             * @brief Unuses the currently used shader program.
             */
            static void unuse();

            static GLuint current;

            Event<ShaderCreationResult> on_shader_add_fail;
            Event<const char*>          on_shader_compilation_fail;
            Event<const char*>          on_shader_link_fail;
        protected:
            GLuint m_id;
            GLuint m_vertex_id, m_frag_id;
            bool   m_is_linked;

            ShaderAttributeMap m_attributes;

            ShaderCache* m_shader_cache;
        };
    }  // namespace graphics
}  // namespace hemlock
namespace hg = hemlock::graphics;

#endif  // __hemlock_graphics_glsl_program_h
