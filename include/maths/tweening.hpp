#ifndef __hemlock_maths_tweening_hpp
#define __hemlock_maths_tweening_hpp

namespace hemlock {
    namespace maths {
        /**
         * @brief Computes a linear tweening.
         */
        inline auto linear(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return startVal + range * alpha;
        }

        template <std::floating_point FXX>
        inline FXX linear(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            if (stageCount) == 0) return;
            return linear(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_quad tweening.
         */
        inline auto ease_in_quad(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range * alpha * alpha + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_quad(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_quad(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_quad tweening.
         */
        inline auto ease_out_quad(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return -1.0f * range * alpha * (alpha - 2.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_quad(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_quad(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_quad tweening.
         */
        inline auto ease_in_out_quad(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return range / 2.0f * alpha2 * alpha2 + startVal;
            }
            std::floating_point auto alpha2Minus1 = alpha2 - 1.0f;
            return -1.0f * range / 2.0f * (alpha2Minus1 * (alpha2Minus1 - 2.0f) - 1.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_quad(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_quad(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_cubic tweening.
         */
        inline auto ease_in_cubic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range * alpha * alpha * alpha + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_cubic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_cubic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_cubic tweening.
         */
        inline auto ease_out_cubic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range       = finalVal - startVal;
            std::floating_point auto alphaMinus1 = alpha - 1.0f;
            return range * (alphaMinus1 * alphaMinus1 * alphaMinus1 + 1.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_cubic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_cubic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_cubic tweening.
         */
        inline auto ease_in_out_cubic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return range / 2.0f * alpha2 * alpha2 * alpha2 + startVal;
            }
            std::floating_point auto alpha2Minus2 = alpha2 - static_cast<FXX>(2.0f;
            return range / 2.0f * (alpha2Minus2 * alpha2Minus2 * alpha2Minus2 + 2.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_cubic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_cubic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_quart tweening.
         */
        inline auto ease_in_quart(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range * alpha * alpha * alpha * alpha + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_quart(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_quart(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_quart tweening.
         */
        inline auto ease_out_quart(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range       = finalVal - startVal;
            std::floating_point auto alphaMinus1 = alpha - 1.0f;
            return -1.0f * range
                       * (alphaMinus1 * alphaMinus1 * alphaMinus1 * alphaMinus1 - 1.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_quart(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_quart(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_quart tweening.
         */
        inline auto ease_in_out_quart(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return range / 2.0f * alpha2 * alpha2 * alpha2 * alpha2 + startVal;
            }
            std::floating_point auto alpha2Minus2 = alpha2 - 2.0f;
            return -1.0f * range / 2.0f
                       * (alpha2Minus2 * alpha2Minus2 * alpha2Minus2 * alpha2Minus2
                          - 2.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_quart(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_quart(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_quint tweening.
         */
        inline auto ease_in_quint(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range * alpha * alpha * alpha * alpha * alpha + startVal;
        }

        template <typename f32>
        template <std::floating_point FXX>
        inline FXX ease_in_quint(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_quint(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_quint tweening.
         */
        inline auto ease_out_quint(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range       = finalVal - startVal;
            std::floating_point auto alphaMinus1 = alpha - 1.0f;
            return range * (alphaMinus1 * alphaMinus1 * alphaMinus1 * alphaMinus1 * alphaMinus1 + static_cast<FXX>(1.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_quint(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_quint(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_quint tweening.
         */
        inline auto ease_in_out_quint(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return range / 2.0f * alpha2 * alpha2 * alpha2 * alpha2 * alpha2
                       + startVal;
            }
            std::floating_point auto alpha2Minus2 = alpha2 - static_cast<FXX>(2.0f;
            return range / 2.0f * (alpha2Minus2 * alpha2Minus2 * alpha2Minus2 * alpha2Minus2 * alpha2Minus2 + static_cast<FXX>(2.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_quint(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_quint(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_sine tweening.
         */
        inline auto ease_in_sine(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return static_cast<FXX>(-1.0f * range * glm::cos(alpha * M_TAUF) + range + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_sine(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_sine(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_sine tweening.
         */
        inline auto ease_out_sine(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range * glm::sin(alpha * M_TAUF) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_sine(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_sine(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_sine tweening.
         */
        inline auto ease_in_out_sine(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return -1.0f * range / 2.0f * (glm::cos(alpha * M_PIF) - 1.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_sine(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_sine(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_expo tweening.
         */
        inline auto ease_in_expo(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return (alpha == 0.0f) ?
                       startVal :
                       range * glm::pow(2.0f, 10.0f * (alpha - 1.0f)) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_expo(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_expo(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_expo tweening.
         */
        inline auto ease_out_expo(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return (alpha == 1.0f) ?
                       finalVal :
                       range * (-1.0f * glm::pow(2.0f, -10.0f * alpha) + 1.0f)
                           + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_expo(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_expo(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_expo tweening.
         */
        inline auto ease_in_out_expo(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            // if (alpha == 0.0f) return startVal;
            // if (alpha == 1.0f) return finalVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return range / 2.0f * glm::pow(2.0f, 10.0f * (alpha2 - 1.0f))
                       + startVal;
            }
            return range / 2.0f * -1.0f
                       * glm::pow(2.0f, -10.0f * (alpha2 - 1.0f) + 2.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_expo(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_expo(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_circ tweening.
         */
        inline auto ease_in_circ(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return -1.0f * range * (glm::sqrt(1.0f - alpha * alpha) - 1.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_circ(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_circ(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_circ tweening.
         */
        inline auto ease_out_circ(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range       = finalVal - startVal;
            std::floating_point auto alphaMinus1 = alpha - 1.0f;
            return range * glm::sqrt(1.0f - alphaMinus1 * alphaMinus1) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_circ(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_circ(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_circ tweening.
         */
        inline auto ease_in_out_circ(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            if (alpha2 < 1.0f) {
                return -1.0f * range / 2.0f * (glm::sqrt(1.0f - alpha2 * alpha2) - 1.0f)
                       + startVal;
            }
            std::floating_point auto alpha2Minus2 = alpha2 - 2.0f;
            return range / 2.0f * (glm::sqrt(1.0f - alpha2Minus2 * alpha2Minus2) + 1.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_circ(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_circ(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_elastic tweening.
         */
        inline auto ease_in_elastic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            // if (alpha == 0.0f) return startVal;
            // if (alpha == 1.0f) return finalVal;
            std::floating_point auto alphaMinus1 = alpha - 1;
            return -1.0f * range * glm::pow(2.0f, 10.0f * alphaMinus1)
                       * glm::sin(M_PIF * (20.0f / 3.0f * alphaMinus1 - 0.5f))
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_elastic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_elastic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_elastic tweening.
         */
        inline auto ease_out_elastic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            // if (alpha == 0.0f) return startVal;
            // if (alpha == 1.0f) return finalVal;
            return range * glm::pow(2.0f, -10.0f * alpha)
                       * glm::sin(M_PIF * (20.0f / 3.0f * alpha - 0.5f)) * 0.5f
                   + finalVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_elastic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_elastic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_elastic tweening.
         */
        inline auto ease_in_out_elastic(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            // if (alpha == 0.0f) return startVal;
            // if (alpha == 1.0f) return finalVal;
            std::floating_point auto alpha2       = 2.0f * alpha;
            std::floating_point auto alpha2Minus1 = alpha2 - 1.0f;
            if (alpha2 < 1.0f) {
                return -0.5f * range * glm::pow(2.0f, 10.0f * alpha2Minus1)
                           * glm::sin(M_PIF * (40.0f / 9.0f * alpha2Minus1 - 0.5f))
                       + startVal;
            }
            return range * glm::pow(2.0f, -10.0f * alpha2Minus1)
                       * glm::sin(M_PIF * (40.0f / 9.0f * alpha2Minus1 - 0.5f)) * 0.5f
                   + finalVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_elastic(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_elastic(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_back tweening.
         */
        inline auto ease_in_back(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha,
            auto                     s = 1.70158f
        ) {
            auto range = finalVal - startVal;
            return range * alpha * alpha * ((s + 1.0f) * alpha - s) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_back(
            FXX          startVal,
            FXX          finalVal,
            unsigned int stageCount,
            unsigned int stage,
            auto         s = 1.70158f
        ) {
            return ease_in_back(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount),
                s
            );
        }

        /**
         * @brief Computes an ease_outBack tweening.
         */
        inline auto ease_out_back(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha,
            auto                     s = 1.70158f
        ) {
            auto                     range       = finalVal - startVal;
            std::floating_point auto alphaMinus1 = alpha - 1;
            return range * (alphaMinus1 * alphaMinus1 * ((s + static_cast<FXX>(1.0f) * alphaMinus1 + s) + 1.0f) + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_out_back(
            FXX          startVal,
            FXX          finalVal,
            unsigned int stageCount,
            unsigned int stage,
            auto         s = 1.70158f
        ) {
            return ease_out_back(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount),
                s
            );
        }

        /**
         * @brief Computes an ease_in_out_back tweening.
         */
        inline auto ease_in_out_back(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha,
            auto                     s = 1.70158f
        ) {
            auto                     range  = finalVal - startVal;
            std::floating_point auto alpha2 = alpha * 2.0f;
            auto                     sPrime = s * 1.525f;
            if (alpha2 < 1.0f) {
                return range / 2.0f
                           * (alpha2 * alpha2 * ((sPrime + 1.0f) * alpha2 - sPrime))
                       + startVal;
            }
            std::floating_point auto alpha2Minus2 = alpha2 - 2.0f;
            return range / 2.0f
                       * (alpha2Minus2 * alpha2Minus2
                              * ((sPrime + 1.0f) * alpha2Minus2 + sPrime)
                          + 2.0f)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_back(
            FXX          startVal,
            FXX          finalVal,
            unsigned int stageCount,
            unsigned int stage,
            auto         s = 1.70158f
        ) {
            return ease_in_out_back(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount),
                s
            );
        }

        /**
         * @brief Computes an ease_in_bounce tweening.
         */
        inline auto ease_in_bounce(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            return range - ease_out_bounce(0, finalVal - startVal, 1 - alpha)
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_bounce(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_bounce(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_out_bounce tweening.
         */
        inline auto ease_out_bounce(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            if (alpha < 1.0f / 2.75f) {
                return range * (7.5625f * alpha * alpha) + startVal;
            } else if (alpha < 2.0f / 2.75f) {
                std::floating_point auto alphaPrime = alpha - (1.5f / 2.75f);
                return range * (7.5625f * alphaPrime * alphaPrime * 0.75f) + startVal;
            } else if (alpha < 2.5f / 2.75f) {
                std::floating_point auto alphaPrime = alpha - (2.25f / 2.75f);
                return range * (7.5625f * alphaPrime * alphaPrime * 0.9375f) + startVal;
            } else {
                std::floating_point auto alphaPrime = alpha - (2.625f / 2.75f);
                return range * (7.5625f * alphaPrime * alphaPrime * 0.984375f)
                       + startVal;
            }
        }

        template <std::floating_point FXX>
        inline FXX ease_out_bounce(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_out_bounce(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }

        /**
         * @brief Computes an ease_in_out_bounce tweening.
         */
        inline auto ease_in_out_bounce(
            std::floating_point auto startVal,
            std::floating_point auto finalVal,
            std::floating_point auto alpha
        ) {
            auto range = finalVal - startVal;
            if (alpha < 0.5f) {
                return ease_in_bounce(0, finalVal - startVal, 2.0f * alpha) * 0.5f
                       + startVal;
            }
            return ease_out_bounce(0, finalVal - startVal, 2.0f * alpha - 1.0f) * range
                       * 0.25f
                   + startVal;
        }

        template <std::floating_point FXX>
        inline FXX ease_in_out_bounce(
            FXX startVal, FXX finalVal, unsigned int stageCount, unsigned int stage
        ) {
            return ease_in_out_bounce(
                startVal,
                finalVal,
                static_cast<FXX>(stage) / static_cast<FXX>(stageCount)
            );
        }
    }  // namespace maths
}  // namespace hemlock
namespace hmaths = hemlock::maths;

#endif  // __hemlock_maths_tweening_hpp
