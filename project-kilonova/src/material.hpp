#ifndef GAME_MATERIAL_HPP
#define GAME_MATERIAL_HPP

#include <glm/vec3.hpp>

#include "texture.hpp"

namespace pk
{
struct material
{
    pk::texture *albedo_map;
    pk::texture *normal_map;
    pk::texture *metallic_map;
    pk::texture *roughness_map;
    pk::texture *ao_map;

    material(
        pk::texture *albedo_map,
        pk::texture *normal_map,
        pk::texture *metallic_map,
        pk::texture *roughness_map,
        pk::texture *ao_map);
    ~material();

    void bind(
        unsigned int albedo_map_index,
        unsigned int normal_map_index,
        unsigned int metallic_map_index,
        unsigned int roughness_map_index,
        unsigned int ao_map_index) const;
};
} // namespace pk

#endif
