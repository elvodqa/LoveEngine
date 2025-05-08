// #pragma once
// #include <algorithm>
//
// #include "Input/EventList.h"
// #include "Input/EventManager.h"
//
// class Orchestration {
//     template<typename F>
//     static void RunNetworked() {
//         auto eventList = getInput();
//
//         love::Networking::PushEvents(std::move(eventList));
//         if (auto receivedList = love::Networking::TryReceive()) {
//             love::events::EventManager::Dispatch(receivedList);
//             runBaseSimulation();
//             prediction.baseChanged();
//         }
//         runprediction();
//         render();
//     }
//     template<typename F>
//     static void RunLocalOnly() {
//
//         love::events::EventManager::Dispatch(getInput());
//         runBaseSimulation();
//         render();
//     }
//     static void RunNetworkServer() {
//         auto eventListList = collectNetworkEvents(timeBudget);
//         auto finalEventList = coalessEventLists(eventListList);
//         auto future = AsyncPushEventsServer(finalEventList);
//         love::events::EventManager::Dispatch(finalEventList);
//         runBaseSimulation();
//     }
// };
