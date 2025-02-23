#pragma once

#include <cib/tuple.hpp>

#include <utility>

namespace interrupt {
struct status_clear_policy {};

struct clear_status_first {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &clear_status,
                    RunCallable const &run) {
        clear_status();
        run();
    }
};

struct clear_status_last {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &clear_status,
                    RunCallable const &run) {
        run();
        clear_status();
    }
};

struct dont_clear_status {
    using PolicyType = status_clear_policy;

    template <typename ClearStatusCallable, typename RunCallable>
    static void run(ClearStatusCallable const &, RunCallable const &run) {
        run();
    }
};

struct required_resources_policy {};

template <typename... ResourcesT> struct required_resources {
    using PolicyType = required_resources_policy;

    constexpr static cib::tuple<ResourcesT...> resources{};
};

template <typename... PoliciesT> class policies {
    template <typename Key, typename Value> struct type_pair {};
    template <typename... Ts> struct type_map : Ts... {};

    template <typename K, typename Default>
    constexpr static auto lookup(...) -> Default;
    template <typename K, typename Default, typename V>
    constexpr static auto lookup(type_pair<K, V>) -> V;

  public:
    template <typename PolicyType, typename DefaultPolicy>
    constexpr static auto get() {
        using M =
            type_map<type_pair<typename PoliciesT::PolicyType, PoliciesT>...>;
        return decltype(lookup<PolicyType, DefaultPolicy>(std::declval<M>())){};
    }

    template <typename PolicyType, typename DefaultPolicy>
    using type = decltype(get<PolicyType, DefaultPolicy>());
};
} // namespace interrupt
