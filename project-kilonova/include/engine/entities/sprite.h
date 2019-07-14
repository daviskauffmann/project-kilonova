#ifndef ENGINE_SPRITE_H
#define ENGINE_SPRITE_H

struct sprite
{
    vec3 color;
    struct texture *color_map;
    vec2 position;
    float rotation;
    vec2 scale;
};

struct sprite *sprite_create(
    vec3 color,
    struct texture *color_map,
    vec2 position,
    float rotation,
    vec2 scale);
void sprite_calc_model(struct sprite *sprite, mat4 model);
void sprite_destroy(struct sprite *sprite);

#endif