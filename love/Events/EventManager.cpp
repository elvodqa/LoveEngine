// #include "EventManager.h"
//
//
// void love::events::EventManager::Dispatch(EventList &&events) {
//     for (auto && event : events.events) {
//         ((void(*)(void*))net_event_handlers[event.hash_code])(event.data);
//     }
// }