#include <cib/tuple.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace std {
template <typename... Ts>
struct tuple_size<cib::tuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};
template <std::size_t I, typename... Ts>
struct tuple_element<I, cib::tuple<Ts...>>
    : std::type_identity<std::remove_cvref_t<
          decltype(std::declval<cib::tuple<Ts...>>()[cib::index<I>])>> {};

template <typename IL, typename... Ts>
struct tuple_size<cib::indexed_tuple<IL, Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};
template <std::size_t I, typename IL, typename... Ts>
struct tuple_element<I, cib::indexed_tuple<IL, Ts...>>
    : std::type_identity<
          std::remove_cvref_t<decltype(std::declval<cib::indexed_tuple<
                                           IL, Ts...>>()[cib::index<I>])>> {};
} // namespace std
