//
// Created by YAHAY on 17/01/2025.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include "glm/gtx/quaternion.hpp"

struct Transform {
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    [[nodiscard]] glm::mat4 getTransformMatrix() const {
        return glm::translate(glm::mat4(1.0f), translation) *
            (glm::toMat4(rotation) *
                glm::scale(glm::mat4(1.0f), scale));
    }

    [[nodiscard]] glm::vec3 forward() const {
        return glm::rotate(rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};
#endif //TRANSFORM_H
