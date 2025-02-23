#pragma once

#include <sc/lazy_string_format.hpp>
#include <sc/string_constant.hpp>
#include <sc/to_string_constant.hpp>

namespace sc::detail {
struct fast_format_spec {
    std::string_view::size_type size{};

    std::string_view::size_type lhs_{};
    std::string_view::size_type rhs_{};

    bool has_name{};
    std::string_view name{};

    bool has_id{};
    int id{};

    bool zero_pad{};
    std::size_t padding_width{};

    char type{};

    template <typename T>
    constexpr static auto is_in_range(T value, T lower, T upper) -> bool {
        return value >= lower && value <= upper;
    }

    constexpr static auto is_alpha(char value) -> bool {
        return is_in_range(value, 'A', 'Z') || is_in_range(value, 'a', 'z');
    }

    constexpr static auto is_digit(char value) -> bool {
        return is_in_range(value, '0', '9');
    }

    constexpr static auto is_id_start(char value) -> bool {
        return is_alpha(value) || value == '_';
    }

    constexpr static auto is_id_continue(char value) -> bool {
        return is_id_start(value) || is_digit(value);
    }

    constexpr fast_format_spec(std::string_view spec,
                               std::string_view::size_type lhs)
        : size{spec.size() + 2}, lhs_{lhs}, rhs_{lhs + size} {
        auto i = spec.begin();

        // check for an id
        if (is_id_start(*i)) {
            has_name = true;
            auto name_begin = i;
            while (is_id_continue(*i)) {
                i++;
            }

            name = std::string_view(name_begin,
                                    static_cast<std::string_view::size_type>(
                                        std::distance(name_begin, i)));

        } else if (is_digit(*i)) {
            has_id = true;
            while (is_digit(*i)) {
                int const d = *i - '0';
                id = (id * 10) + d;
                i++;
            }

        } else if (*i == ':') {
            i++;
        }

        // parse zero-pad
        if (*i == '0') {
            zero_pad = true;
            i++;
        }

        // parse padding width
        while (is_digit(*i)) {
            auto const d = *i - '0';
            padding_width = (padding_width * 10) + static_cast<std::size_t>(d);
            i++;
        }

        // parse type
        if (is_alpha(*i)) {
            type = *i;
        }
    }
};
} // namespace sc::detail
