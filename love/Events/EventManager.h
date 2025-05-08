// #pragma once
// #include <queue>
// #include <unordered_map>
//
// #include "debug_panic.h"
// #include "EventList.h"
//
// namespace love::events{
//     class EventManager {
//     private:
//         EventManager()=delete;
//         inline static std::pmr::unordered_map<uint64_t, void(*)()> net_event_handlers;
//         inline static std::queue<EventList> event_queue;
//         template<typename T>
//         static void RegisterNetEventHandler(void(*handler)(T* data)) {
//             if (net_event_handlers.contains(typeid(T).hash_code())) {
//                 panic_dbg("trying to register handler for event type {}, but one already exists",typeid(T).name());
//             }
//             net_event_handlers[typeid(T).hash_code()] = handler;
//         }
//     public:
//
//         static void Dispatch(EventList &&events);
//     };
// }