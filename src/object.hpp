#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include "model.hpp"

namespace pk
{
    struct object
    {
        pk::model *model;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        object(
            pk::model *model,
            glm::vec3 position,
            glm::vec3 rotation,
            glm::vec3 scale);

        glm::mat4 calc_model() const;
    };
} // namespace pk

#endif
