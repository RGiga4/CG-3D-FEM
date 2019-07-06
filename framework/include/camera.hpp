#pragma once

#include "common.hpp"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

struct camera {
    camera(GLFWwindow* window);

    virtual ~camera();

    glm::mat4
    view_matrix() const;

    glm::vec3
    position() const;
};
