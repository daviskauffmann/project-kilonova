#ifndef ENGINE_WATER_H
#define ENGINE_WATER_H

struct water
{
    vec3 position;
    vec2 scale;
};

struct water *water_create(vec3 position, vec2 scale);
void water_destroy(struct water *water);

#endif