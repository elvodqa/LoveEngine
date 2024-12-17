//
// Created by YAHAY on 17/12/2024.
//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#include <functional>
#include <vector>

#include "renderer_constants.h"
static std::pmr::vector<std::function<void()>> cleanups[MAX_INFLIGHT_FRAMES];
static int currentFrame = 0;
static void default_cleanup() {

}
/**
 * defer for frame lifetime
 * @param fn cleanup function to run after all proccessing with current frame finishes
 */
inline void deferffl(std::function<void()> &&fn) {
    cleanups[currentFrame%MAX_INFLIGHT_FRAMES].push_back(std::move(fn));
}
inline void advance_frame_and_execute_cleanups() {
    currentFrame++;
    for (auto f : cleanups[currentFrame%MAX_INFLIGHT_FRAMES]) {
        f();
    }
    cleanups[currentFrame%MAX_INFLIGHT_FRAMES].clear();
    deferffl(default_cleanup);
}

#endif //RESOURCEMANAGER_H
