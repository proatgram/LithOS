#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <random>
#include <typeindex>
#include <set>
#include <vector>
#include <algorithm>

namespace Event {
    class Event {
        public:
            using CallbackId_t = uint64_t;

            template <class Event, typename Callable>
            requires std::is_invocable_v<Callable, Event>
            static inline auto RegisterCallback(Callable &&callable) -> CallbackId_t {
                CallbackId_t id = GenerateCallbackId();
                
                s_callbacks[id] = [callable](std::any args) -> void {
                    callable(std::any_cast<Event>(args));
                };

                s_events[typeid(Event)].insert(id);

                return id;
            }

            static inline auto UnregisterCallback(CallbackId_t callbackId) -> void {
                for (auto &[event, callbacks] : s_events) {
                    std::erase_if(callbacks, [callbackId](const auto &id) -> bool {
                        return id == callbackId;
                    });
                }

                std::erase_if(s_callbacks, [callbackId](const auto &pair) -> bool {
                    return pair.first == callbackId;
                });
            }

            template <class Event>
            static inline auto Emit(Event &&event) -> void {
                decltype(s_events)::iterator it = std::find_if(std::begin(s_events), std::end(s_events), [](const auto &pair) -> bool {
                    return pair.first == typeid(Event);
                });

                if (it == std::end(s_events)) {
                    return;
                }

                for (const CallbackId_t &id : it->second) {
                    // Make sure the container contains the callback ID before attempting to call it.
                    if (s_callbacks.contains(id)) {
                        s_callbacks[id](std::make_any<Event>(std::forward<Event>(event)));
                    }
                }
            }

        private:

            static inline auto GenerateCallbackId() -> CallbackId_t {
                static std::random_device rd;
                static std::mt19937_64 generator(rd());
                static std::uniform_int_distribution<uint64_t> distribution;

                return distribution(generator);
            }

            inline static std::map<CallbackId_t, std::function<void(std::any)>> s_callbacks{};
            inline static std::map<std::type_index, std::set<CallbackId_t>> s_events;
    };
}  // namespace Event
