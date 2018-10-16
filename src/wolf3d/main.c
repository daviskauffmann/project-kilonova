#include <cglm/cglm.h>
#include <engine/engine.h>
#include <GL/glew.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_TITLE "Wolf3D"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define FPS_CAP 60

#define VERTICES_PER_PIXEL 4
#define FLOATS_PER_VERTEX 8
#define INDICES_PER_PIXEL 2
#define INTS_PER_INDEX 3

int main(int argc, char *argv[])
{
    engine_init();
    window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    audio_init();
    struct program *phong_program = program_create(
        "assets/shaders/phong.vs",
        "assets/shaders/phong.fs");
    struct bitmap *level_bitmap = bitmap_create("assets/images/level_bitmap.png");
    const unsigned int num_vertices = level_bitmap->width * level_bitmap->height * VERTICES_PER_PIXEL * FLOATS_PER_VERTEX * sizeof(float);
    float *vertices = malloc(num_vertices);
    const unsigned int num_indices = level_bitmap->width * level_bitmap->height * INDICES_PER_PIXEL * INTS_PER_INDEX * sizeof(unsigned int);
    unsigned int *indices = malloc(num_indices);
    int i = 0;
    for (int x = 0; x < level_bitmap->width; x++)
    {
        for (int y = 0; y < level_bitmap->height; y++)
        {
            float x_high = 1.0f;
            float x_low = 0.0f;
            float y_high = 1.0f;
            float y_low = 0.0f;

            vertices[(x + y * level_bitmap->width) + 0] = x;
            vertices[(x + y * level_bitmap->width) + 1] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 2] = y;
            vertices[(x + y * level_bitmap->width) + 3] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 4] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 5] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 6] = x_low;
            vertices[(x + y * level_bitmap->width) + 7] = y_low;

            vertices[(x + y * level_bitmap->width) + 8] = x + 1;
            vertices[(x + y * level_bitmap->width) + 9] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 10] = y;
            vertices[(x + y * level_bitmap->width) + 11] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 12] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 13] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 14] = x_high;
            vertices[(x + y * level_bitmap->width) + 15] = y_low;

            vertices[(x + y * level_bitmap->width) + 16] = x + 1;
            vertices[(x + y * level_bitmap->width) + 17] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 18] = y + 1;
            vertices[(x + y * level_bitmap->width) + 19] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 20] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 21] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 22] = x_high;
            vertices[(x + y * level_bitmap->width) + 23] = y_high;

            vertices[(x + y * level_bitmap->width) + 24] = x;
            vertices[(x + y * level_bitmap->width) + 25] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 26] = y + 1;
            vertices[(x + y * level_bitmap->width) + 27] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 28] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 29] = 0.0f;
            vertices[(x + y * level_bitmap->width) + 30] = x_low;
            vertices[(x + y * level_bitmap->width) + 31] = y_high;

            indices[(x + y * level_bitmap->width) + 0] = i + 2;
            indices[(x + y * level_bitmap->width) + 1] = i + 1;
            indices[(x + y * level_bitmap->width) + 2] = i + 0;

            indices[(x + y * level_bitmap->width) + 3] = i + 3;
            indices[(x + y * level_bitmap->width) + 4] = i + 2;
            indices[(x + y * level_bitmap->width) + 5] = i + 0;

            i += VERTICES_PER_PIXEL;
        }
    }
    struct mesh *level_mesh = mesh_create(vertices, num_vertices, indices, num_indices);
    struct texture *level_texture = texture_create("assets/images/level_texture.png");
    struct material *level_material = material_create(
        level_texture,
        NULL,
        NULL,
        (vec3){1.0f, 1.0f, 1.0f},
        32.0f,
        1.0f);
    struct object *objects[] = {
        object_create(
            level_mesh,
            level_material,
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){1.0f, 1.0f, 1.0f})};
    const unsigned int num_objects = sizeof(objects) / sizeof(struct object *);
    struct directional_light *directional_light = directional_light_create(
        (vec3){-0.2f, -1.0f, -0.3f},
        (vec3){0.1f, 0.1f, 0.1f},
        (vec3){0.8f, 0.8f, 0.8f},
        (vec3){1.0f, 1.0f, 1.0f});
    struct point_light *point_lights[] = {
        point_light_create(
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        point_light_create(
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        point_light_create(
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        point_light_create(
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f)};
    const unsigned int num_point_lights = sizeof(point_lights) / sizeof(struct point_light *);
    struct spot_light *spot_light = spot_light_create(
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){0.0f, 0.0f, 0.0f},
        1.0f,
        0.09f,
        0.032f,
        cosf(glm_rad(12.5f)),
        cosf(glm_rad(15.0f)));
    struct camera *camera = camera_create(
        (vec3){0.0f, 0.0f, 3.0f},
        (vec3){0.0f, 0.0f, -1.0f},
        (vec3){0.0f, 1.0f, 0.0f},
        0.0f,
        -90.0f,
        0.0f,
        45.0f);

    time_cap_fps(FPS_CAP);

    window_toggle_mouse();

    GLint phong_program_time = program_get_location(phong_program, "time");
    GLint phong_program_material_diffuse = program_get_location(phong_program, "material.diffuse");
    GLint phong_program_material_specular = program_get_location(phong_program, "material.specular");
    GLint phong_program_material_emission = program_get_location(phong_program, "material.emission");
    GLint phong_program_material_color = program_get_location(phong_program, "material.color");
    GLint phong_program_material_shininess = program_get_location(phong_program, "material.shininess");
    GLint phong_program_material_glow = program_get_location(phong_program, "material.glow");
    GLint phong_program_object_model = program_get_location(phong_program, "object.model");
    GLint phong_program_camera_projection = program_get_location(phong_program, "camera.projection");
    GLint phong_program_camera_view = program_get_location(phong_program, "camera.view");
    GLint phong_program_camera_position = program_get_location(phong_program, "camera.position");
    GLint phong_program_directional_light_direction = program_get_location(phong_program, "directional_light.direction");
    GLint phong_program_directional_light_ambient = program_get_location(phong_program, "directional_light.ambient");
    GLint phong_program_directional_light_diffuse = program_get_location(phong_program, "directional_light.diffuse");
    GLint phong_program_directional_light_specular = program_get_location(phong_program, "directional_light.specular");
    GLint phong_program_point_lights_0_position = program_get_location(phong_program, "point_lights[0].position");
    GLint phong_program_point_lights_0_ambient = program_get_location(phong_program, "point_lights[0].ambient");
    GLint phong_program_point_lights_0_diffuse = program_get_location(phong_program, "point_lights[0].diffuse");
    GLint phong_program_point_lights_0_specular = program_get_location(phong_program, "point_lights[0].specular");
    GLint phong_program_point_lights_0_constant = program_get_location(phong_program, "point_lights[0].constant");
    GLint phong_program_point_lights_0_linear = program_get_location(phong_program, "point_lights[0].linear");
    GLint phong_program_point_lights_0_quadratic = program_get_location(phong_program, "point_lights[0].quadratic");
    GLint phong_program_point_lights_1_position = program_get_location(phong_program, "point_lights[1].position");
    GLint phong_program_point_lights_1_ambient = program_get_location(phong_program, "point_lights[1].ambient");
    GLint phong_program_point_lights_1_diffuse = program_get_location(phong_program, "point_lights[1].diffuse");
    GLint phong_program_point_lights_1_specular = program_get_location(phong_program, "point_lights[1].specular");
    GLint phong_program_point_lights_1_constant = program_get_location(phong_program, "point_lights[1].constant");
    GLint phong_program_point_lights_1_linear = program_get_location(phong_program, "point_lights[1].linear");
    GLint phong_program_point_lights_1_quadratic = program_get_location(phong_program, "point_lights[1].quadratic");
    GLint phong_program_point_lights_2_position = program_get_location(phong_program, "point_lights[2].position");
    GLint phong_program_point_lights_2_ambient = program_get_location(phong_program, "point_lights[2].ambient");
    GLint phong_program_point_lights_2_diffuse = program_get_location(phong_program, "point_lights[2].diffuse");
    GLint phong_program_point_lights_2_specular = program_get_location(phong_program, "point_lights[2].specular");
    GLint phong_program_point_lights_2_constant = program_get_location(phong_program, "point_lights[2].constant");
    GLint phong_program_point_lights_2_linear = program_get_location(phong_program, "point_lights[2].linear");
    GLint phong_program_point_lights_2_quadratic = program_get_location(phong_program, "point_lights[2].quadratic");
    GLint phong_program_point_lights_3_position = program_get_location(phong_program, "point_lights[3].position");
    GLint phong_program_point_lights_3_ambient = program_get_location(phong_program, "point_lights[3].ambient");
    GLint phong_program_point_lights_3_diffuse = program_get_location(phong_program, "point_lights[3].diffuse");
    GLint phong_program_point_lights_3_specular = program_get_location(phong_program, "point_lights[3].specular");
    GLint phong_program_point_lights_3_constant = program_get_location(phong_program, "point_lights[3].constant");
    GLint phong_program_point_lights_3_linear = program_get_location(phong_program, "point_lights[3].linear");
    GLint phong_program_point_lights_3_quadratic = program_get_location(phong_program, "point_lights[3].quadratic");
    GLint phong_program_spot_light_position = program_get_location(phong_program, "spot_light.position");
    GLint phong_program_spot_light_direction = program_get_location(phong_program, "spot_light.direction");
    GLint phong_program_spot_light_ambient = program_get_location(phong_program, "spot_light.ambient");
    GLint phong_program_spot_light_diffuse = program_get_location(phong_program, "spot_light.diffuse");
    GLint phong_program_spot_light_specular = program_get_location(phong_program, "spot_light.specular");
    GLint phong_program_spot_light_constant = program_get_location(phong_program, "spot_light.constant");
    GLint phong_program_spot_light_linear = program_get_location(phong_program, "spot_light.linear");
    GLint phong_program_spot_light_quadratic = program_get_location(phong_program, "spot_light.quadratic");
    GLint phong_program_spot_light_cutOff = program_get_location(phong_program, "spot_light.cutOff");
    GLint phong_program_spot_light_outerCutOff = program_get_location(phong_program, "spot_light.outerCutOff");

    program_bind(phong_program);
    program_set_int(phong_program_material_diffuse, 0);
    program_set_int(phong_program_material_specular, 1);
    program_set_int(phong_program_material_emission, 2);
    program_unbind();

    struct program *current_program = phong_program;

    bool quit = false;
    while (!quit)
    {
        time_frame_start();

        char title[256];
        sprintf(title, "%s - FPS: %d", WINDOW_TITLE, time_fps());
        window_set_title(title);

        int num_keys;
        const unsigned char *keys = window_keyboard(&num_keys);

        int mouse_x, mouse_y;
        unsigned int mouse = window_mouse(&mouse_x, &mouse_y);

        SDL_Event event;
        while (window_event(&event))
        {
            switch (event.type)
            {
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_F4:
                {
                    if (keys[SDL_SCANCODE_LALT])
                    {
                        quit = true;
                    }
                }
                break;
                case SDLK_RETURN:
                {
                    if (keys[SDL_SCANCODE_LALT])
                    {
                        window_toggle_fullscreen();
                    }
                }
                break;
                case SDLK_TAB:
                {
                    window_toggle_mouse();
                }
                break;
                }
            }
            break;
            case SDL_MOUSEMOTION:
            {
                camera->pitch -= event.motion.yrel * 0.1f;
                camera->yaw += event.motion.xrel * 0.1f;

                if (camera->pitch > 89.0f)
                {
                    camera->pitch = 89.0f;
                }
                if (camera->pitch < -89.0f)
                {
                    camera->pitch = -89.0f;
                }

                vec3 front = {
                    cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch)),
                    sinf(glm_rad(camera->pitch)),
                    sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch))};
                glm_normalize(front);
                glm_vec_copy(front, camera->front);
            }
            break;
            case SDL_MOUSEWHEEL:
            {
                if (camera->fov >= 1.0f && camera->fov <= 90.0f)
                {
                    camera->fov -= event.wheel.y;
                }
                if (camera->fov <= 1.0f)
                {
                    camera->fov = 1.0f;
                }
                if (camera->fov >= 90.0f)
                {
                    camera->fov = 90.0f;
                }
            }
            break;
            case SDL_QUIT:
            {
                quit = true;
            }
            break;
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    int width = event.window.data1;
                    int height = event.window.data2;

                    window_resize(width, height);
                }
                break;
                }
            }
            break;
            }
        }

        float speed = 5.0f * time_delta();

        if (keys[SDL_SCANCODE_LSHIFT])
        {
            speed *= 2.0f;
        }

        if ((keys[SDL_SCANCODE_W] && keys[SDL_SCANCODE_D]) ||
            (keys[SDL_SCANCODE_W] && keys[SDL_SCANCODE_A]) ||
            (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_D]) ||
            (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_A]))
        {
            speed *= 0.71f;
        }

        if (keys[SDL_SCANCODE_W])
        {
            vec3 movement;
            glm_vec_scale(camera->front, speed, movement);
            glm_vec_add(camera->position, movement, camera->position);
        }

        if (keys[SDL_SCANCODE_A])
        {
            vec3 direction;
            glm_cross(camera->front, camera->up, direction);
            glm_normalize(direction);
            vec3 movement;
            glm_vec_scale(direction, -speed, movement);
            glm_vec_add(camera->position, movement, camera->position);
        }

        if (keys[SDL_SCANCODE_S])
        {
            vec3 movement;
            glm_vec_scale(camera->front, -speed, movement);
            glm_vec_add(camera->position, movement, camera->position);
        }

        if (keys[SDL_SCANCODE_D])
        {
            vec3 direction;
            glm_cross(camera->front, camera->up, direction);
            glm_normalize(direction);
            vec3 movement;
            glm_vec_scale(direction, speed, movement);
            glm_vec_add(camera->position, movement, camera->position);
        }

        glm_vec_copy(camera->position, spot_light->position);
        glm_vec_copy(camera->front, spot_light->direction);

        mat4 camera_projection;
        camera_calc_projection(camera, camera_projection);

        mat4 camera_view;
        camera_calc_view(camera, camera_view);

        program_bind(phong_program);
        program_set_int(phong_program_time, time_current());
        program_set_mat4(phong_program_camera_projection, camera_projection);
        program_set_mat4(phong_program_camera_view, camera_view);
        program_set_vec3(phong_program_camera_position, camera->position);
        program_set_vec3(phong_program_directional_light_direction, directional_light->direction);
        program_set_vec3(phong_program_directional_light_ambient, directional_light->ambient);
        program_set_vec3(phong_program_directional_light_diffuse, directional_light->diffuse);
        program_set_vec3(phong_program_directional_light_specular, directional_light->specular);
        program_set_vec3(phong_program_point_lights_0_position, point_lights[0]->position);
        program_set_vec3(phong_program_point_lights_0_ambient, point_lights[0]->ambient);
        program_set_vec3(phong_program_point_lights_0_diffuse, point_lights[0]->diffuse);
        program_set_vec3(phong_program_point_lights_0_specular, point_lights[0]->specular);
        program_set_float(phong_program_point_lights_0_constant, point_lights[0]->constant);
        program_set_float(phong_program_point_lights_0_linear, point_lights[0]->linear);
        program_set_float(phong_program_point_lights_0_quadratic, point_lights[0]->quadratic);
        program_set_vec3(phong_program_point_lights_1_position, point_lights[1]->position);
        program_set_vec3(phong_program_point_lights_1_ambient, point_lights[1]->ambient);
        program_set_vec3(phong_program_point_lights_1_diffuse, point_lights[1]->diffuse);
        program_set_vec3(phong_program_point_lights_1_specular, point_lights[1]->specular);
        program_set_float(phong_program_point_lights_1_constant, point_lights[1]->constant);
        program_set_float(phong_program_point_lights_1_linear, point_lights[1]->linear);
        program_set_float(phong_program_point_lights_1_quadratic, point_lights[1]->quadratic);
        program_set_vec3(phong_program_point_lights_2_position, point_lights[2]->position);
        program_set_vec3(phong_program_point_lights_2_ambient, point_lights[2]->ambient);
        program_set_vec3(phong_program_point_lights_2_diffuse, point_lights[2]->diffuse);
        program_set_vec3(phong_program_point_lights_2_specular, point_lights[2]->specular);
        program_set_float(phong_program_point_lights_2_constant, point_lights[2]->constant);
        program_set_float(phong_program_point_lights_2_linear, point_lights[2]->linear);
        program_set_float(phong_program_point_lights_2_quadratic, point_lights[2]->quadratic);
        program_set_vec3(phong_program_point_lights_3_position, point_lights[3]->position);
        program_set_vec3(phong_program_point_lights_3_ambient, point_lights[3]->ambient);
        program_set_vec3(phong_program_point_lights_3_diffuse, point_lights[3]->diffuse);
        program_set_vec3(phong_program_point_lights_3_specular, point_lights[3]->specular);
        program_set_float(phong_program_point_lights_3_constant, point_lights[3]->constant);
        program_set_float(phong_program_point_lights_3_linear, point_lights[3]->linear);
        program_set_float(phong_program_point_lights_3_quadratic, point_lights[3]->quadratic);
        program_set_vec3(phong_program_spot_light_position, spot_light->position);
        program_set_vec3(phong_program_spot_light_direction, spot_light->direction);
        program_set_vec3(phong_program_spot_light_ambient, spot_light->ambient);
        program_set_vec3(phong_program_spot_light_diffuse, spot_light->diffuse);
        program_set_vec3(phong_program_spot_light_specular, spot_light->specular);
        program_set_float(phong_program_spot_light_constant, spot_light->constant);
        program_set_float(phong_program_spot_light_linear, spot_light->linear);
        program_set_float(phong_program_spot_light_quadratic, spot_light->quadratic);
        program_set_float(phong_program_spot_light_cutOff, spot_light->cutOff);
        program_set_float(phong_program_spot_light_outerCutOff, spot_light->outerCutOff);
        program_unbind();

        window_clear();

        for (int i = 0; i < num_objects; i++)
        {
            mat4 model = GLM_MAT4_IDENTITY_INIT;
            object_calc_model(objects[i], model);

            program_bind(phong_program);
            program_set_mat4(phong_program_object_model, model);
            program_set_vec3(phong_program_material_color, objects[i]->material->color);
            program_set_float(phong_program_material_shininess, objects[i]->material->shininess);
            program_set_float(phong_program_material_glow, objects[i]->material->glow);
            program_unbind();

            program_bind(current_program);
            object_draw(objects[i]);
            program_unbind();
        }

        window_render();

        time_frame_end();
    }

    for (int i = 0; i < num_objects; i++)
    {
        object_destroy(objects[i]);
    }
    material_destroy(level_material);
    texture_destroy(level_texture);
    mesh_destroy(level_mesh);
    bitmap_destroy(level_bitmap);
    program_destroy(phong_program);
    audio_quit();
    window_quit();
    engine_quit();

    return 0;
}
