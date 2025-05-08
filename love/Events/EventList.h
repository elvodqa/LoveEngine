// #pragma once
// #include <cstdint>
// #include <stdlib.h>
// #include <string.h>
// #include <typeinfo>
// #include <vector>
//
// #include "debug_panic.h"
//
//
// namespace love::events {
// class EventManager;
//
// struct Event {
//     void* data;
//     uint64_t hash_code;
//     uint32_t size;
// };
//
// class EventList {
//     private:
//     std::vector<Event> events;
//     uint64_t dataSize = 0;
//     void* backingData = nullptr;
//     friend love::events::EventManager;
//
//     public:
//     EventList(const EventList&) noexcept = delete; // no copy move only
//     void operator= (const EventList& src) noexcept = delete;
//     EventList(EventList&&) noexcept = default;
//     EventList& operator= (EventList&&) noexcept = default;
//     EventList() = default;
//
//     template<typename T>
//     void AddEvent(T* event) {
//         if (backingData) {
//             panic_dbg("EventList::AddEvent: Cannot add events to a network sourced list");
//         }
//         void* ptr = malloc(sizeof(T));
//         memcpy(ptr,event,sizeof(T));
//         events.emplace_back(ptr,typeid(T).hash_code(),sizeof(T));
//         dataSize += sizeof(T);
//     }
//     ~EventList() {
//         if (backingData) {
//             free(backingData);
//         }
//         else {
//             for (const auto & event : events) {
//                 free(event.data);
//             }
//         }
//     }
//     private:
//     struct SerializedEvent {
//         uint32_t dataoffset;
//         uint32_t size;
//         uint64_t hash_code;
//     };
//     void* Serialize() const {
//         auto totalSize = dataSize + 2*sizeof(uint64_t) + sizeof(SerializedEvent)*events.size();
//         auto ptr = malloc(totalSize);
//         *((uint64_t*)ptr)=totalSize;
//         *(((uint64_t*)ptr)+1)=events.size();
//         auto metaptr = (SerializedEvent*)(ptr+2*sizeof(uint64_t));
//         auto dataptr = (void*)metaptr+sizeof(SerializedEvent)*events.size();
//         uint32_t runningTot=0;
//         for (uint32_t i=0;i<events.size();i++) {
//             auto & event = events[i];
//             *metaptr={.dataoffset = runningTot,.size = event.size,.hash_code = event.hash_code};
//             metaptr++;
//             memcpy(dataptr,(event.data+runningTot),event.size);
//             runningTot+=event.size;
//         }
//
//         return ptr;
//     }
//     void Deserialize(void* ptr) {
//         if (!events.empty()) {
//             panic_dbg("EventList::Deserialize: Event list is not empty");
//         }
//         uint64_t totalSize = *(uint64_t*)ptr;
//         uint32_t itemCount= (*(uint64_t*)(ptr+sizeof(uint64_t)));
//         auto metaptr = (SerializedEvent*)(ptr+2*sizeof(uint64_t));
//         auto dataptr = (void*)metaptr+sizeof(SerializedEvent)*itemCount;
//         for (uint32_t i=0;i<itemCount;i++) {
//             auto [dataoffset, size, hash_code] = metaptr[i];
//             events.emplace_back(dataptr+dataoffset,hash_code,size);
//         }
//     }
// };
//
// } // events
// // love
