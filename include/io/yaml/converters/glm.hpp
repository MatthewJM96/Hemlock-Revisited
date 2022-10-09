#ifndef __hemlock_io_yaml_converters_glm_hpp
#define __hemlock_io_yaml_converters_glm_hpp

namespace YAML {

    /***************\
     * GLM Vectors *
    \***************/

    template <glm::length_t Length, typename Type>
    struct convert<glm::vec<Length, Type, glm::precision::defaultp>> {
        static Node encode(const glm::vec<Length, Type, glm::precision::defaultp>& vec) {
            Node result;
            for (glm::length_t i = 0; i < Length; ++i) {
                result[i] = vec[i];
            }
            return result;
        }

        static bool decode(const Node& node, glm::vec<Length, Type, OUT glm::precision::defaultp>& result) {
            if(!node.IsSequence() || node.size() != Length) {
                return false;
            }

            for (glm::length_t i = 0; i < Length; ++i) {
                if (!node[i].IsScalar()) {
                    return false;
                }

                result[i] = node[i].as<Type>();
            }

            return true;
        }
    };

    /***********************\
     * GLM Square Matrices *
    \***********************/

    template <glm::length_t Size, typename Type>
    struct convert<glm::mat<Size, Size, Type, glm::precision::defaultp>> {
        static Node encode(const glm::mat<Size, Size, Type, glm::precision::defaultp>& mat) {
            Node result;
            for (glm::length_t i = 0; i < Size; ++i) {
                result[i] = Node(NodeType::Sequence);
                for (glm::length_t j = 0; j < Size; ++j) {
                    // GLM matrices are column-major, but YAML will express
                    // row-major more elegantly. Performance shouldn't depend
                    // on this being "optimal".
                    result[i][j] = mat[j][i];
                }
            }
            return result;
        }

        static bool decode(const Node& node, glm::mat<Size, Size, Type, OUT glm::precision::defaultp>& result) {
            if(!node.IsSequence() || node.size() != Size) {
                return false;
            }

            for (glm::length_t i = 0; i < Size; ++i) {
                if (!node[i].IsSequence() || node[i].size() != Size) {
                    return false;
                }

                for (glm::length_t j = 0; j < Size; ++j) {
                    if (!node[i][j].IsScalar()) {
                        return false;
                    }

                    result[j][i] = node[i][j].as<Type>();
                }
            }

            return true;
        }
    };

    /***************************\
     * GLM Non-Square Matrices *
    \***************************/

    template <glm::length_t Columns, glm::length_t Rows, typename Type>
        requires ( Columns != Rows )
    struct convert<glm::mat<Columns, Rows, Type, glm::precision::defaultp>> {
        static Node encode(const glm::mat<Columns, Rows, Type, glm::precision::defaultp>& mat) {
            Node result;
            for (glm::length_t i = 0; i < Rows; ++i) {
                result[i] = Node(NodeType::Sequence);
                for (glm::length_t j = 0; j < Columns; ++j) {
                    // GLM matrices are column-major, but YAML will express
                    // row-major more elegantly. Performance shouldn't depend
                    // on this being "optimal".
                    result[i][j] = mat[j][i];
                }
            }
            return result;
        }

        static bool decode(const Node& node, glm::mat<Columns, Rows, Type, OUT glm::precision::defaultp>& result) {
            if(!node.IsSequence() || node.size() != Rows) {
                return false;
            }

            for (glm::length_t i = 0; i < Rows; ++i) {
                if (!node[i].IsSequence() || node[i].size() != Columns) {
                    return false;
                }

                for (glm::length_t j = 0; j < Columns; ++j) {
                    if (!node[i][j].IsScalar()) {
                        return false;
                    }

                    result[j][i] = node[i][j].as<Type>();
                }
            }

            return true;
        }
    };

}

#endif // __hemlock_io_yaml_converters_glm_hpp
