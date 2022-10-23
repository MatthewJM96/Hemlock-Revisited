#ifndef __hemlock_io_yaml_converters_union_hpp
#define __hemlock_io_yaml_converters_union_hpp


enum class MyUnionKind : ui8 {
    FIRST,
    SECOND,
    SENTINEL
};
struct MyUnion {
    MyUnionKind kind;
    union {
        f32v2 scaling;
        struct {
            f32 scale_x;
            f32 target_height;
        };
    };
};

namespace YAML {
    template <>
    struct convert<TYPE> {
        static Node encode(const TYPE& val) {
            YAML::Node node(YAML::NodeType::Map);

            node["kind"] = YAML::Node(val.kind);

            switch (val.kind) {
                case MyUnionKind::FIRST:
                    node["scaling"] = YAML::Node(val.scaling);
                case MyUnionKind::SECOND:
                    node["scale_x"] = YAML::Node(val.scale_x);
                    node["target_height"] = YAML::Node(val.target_height);
                default:
                    debug_printf(
                        "Could not encode MyUnion with kind %i",
                        static_cast<std::underlying_type<MyUnionKind>::type>(val.kind)
                    );
                    return YAML::Node();
            }

            return node;
        }      

        static bool decode(const Node& node, TYPE& result) {
            if (!convert<MyUnionKind>::decode(node["kind"], result.kind) {
                return false;
            }

            switch (result.kind) {
                case MyUnionKind::FIRST:
                    {
                        if (
                            !convert<f32v2>::decode(
                                node["scaling"], result.scaling
                            )
                        ) {
                            return false;
                        }
                    }
                case MyUnionKind::SECOND:
                    {
                        if (
                            !convert<f32>::decode(
                                node["scale_x"], result.scale_x
                            )
                        ) {
                            return false;
                        }
                        if (
                            !convert<f32>::decode(
                                node["target_height"], result.target_height
                            )
                        ) {
                            return false;
                        }
                    }
                default:
                    debug_printf(
                        "Could not encode MyUnion with kind %i",
                        static_cast<std::underlying_type<MyUnionKind>::type>(val.kind)
                    );
                    return false;
            }

            return true;
        }
    };
}


#endif // __hemlock_io_yaml_converters_union_hpp
