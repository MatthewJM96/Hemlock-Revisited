#ifndef __hemlock_io_yaml_converters_date_hpp
#define __hemlock_io_yaml_converters_date_hpp

namespace YAML {
    template <>
    struct convert<date::year_month_day> {
        static Node encode(const date::year_month_day& date) {
            std::stringstream ss;

            ss << date;

            return Node{ ss.str() };
        }

        static bool decode(const Node& node, date::year_month_day& date) {
            if (!node.IsScalar()) {
                return false;
            }

            std::stringstream ss{ node.as<std::string>() };

            ss >> date::parse("%F", date);

            return !!ss;
        }
    };

    template <>
    struct convert<date::hh_mm_ss<std::chrono::seconds>> {
        static Node encode(const date::hh_mm_ss<std::chrono::seconds>& time) {
            std::stringstream ss;

            ss << time;

            return Node{ ss.str() };
        }

        static bool
        decode(const Node& node, date::hh_mm_ss<std::chrono::seconds>& time) {
            if (!node.IsScalar()) {
                return false;
            }

            std::stringstream ss{ node.as<std::string>() };

            std::chrono::seconds tmp;
            ss >> date::parse("%T", tmp);

            time = date::hh_mm_ss<std::chrono::seconds>{ tmp };

            return !!ss;
        }
    };

    template <>
    struct convert<std::chrono::time_point<std::chrono::system_clock>> {
        static Node
        encode(const std::chrono::time_point<std::chrono::system_clock>& time) {
            std::stringstream ss;

            ss << std::format("%FT%T%z", time);

            return Node{ ss.str() };
        }

        static bool decode(
            const Node& node, std::chrono::time_point<std::chrono::system_clock>& time
        ) {
            if (!node.IsScalar()) {
                return false;
            }

            std::stringstream ss{ node.as<std::string>() };

            ss >> date::parse("%FT%T%z", time);

            return !ss.fail();
        }
    };
}  // namespace YAML

#endif  // __hemlock_io_yaml_converters_date_hpp
