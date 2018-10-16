#include <cglm/cglm.h>
#include <engine/engine.h>
#include <game/shared/data.h>
#include <GL/glew.h>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_TITLE "Test Game v0.1"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define NETWORK 1
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 1000

#define FPS_CAP 60

int main(int argc, char *argv[])
{
    if (engine_init())
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    if (window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    if (audio_init())
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct program *basic_program = program_create(
        "assets/shaders/basic.vs",
        "assets/shaders/basic.fs");

    if (!basic_program)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct program *phong_program = program_create(
        "assets/shaders/phong.vs",
        "assets/shaders/phong.fs");

    if (!phong_program)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }
    float quad_vertices[] = {
        // position          // normal            // uv
        +1.0f, +1.0f, +0.0f, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f, // top right
        +1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f  // top left
    };

    unsigned int quad_indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    struct mesh *quad_mesh = mesh_create(
        quad_vertices,
        sizeof(quad_vertices),
        quad_indices,
        sizeof(quad_indices));

    if (!quad_mesh)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    float cube_vertices[] = {
        -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, +1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 1.0f, 1.0f,
        -1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f, 1.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, +0.0f, +0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, +1.0f, -1.0f, +0.0f, +0.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, +1.0f, +0.0f, +0.0f, 1.0f, 1.0f,
        +1.0f, -1.0f, -1.0f, +1.0f, +0.0f, +0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, -1.0f, +1.0f, +0.0f, +0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, +1.0f, +0.0f, +0.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, +0.0f, -1.0f, +0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, -1.0f, +0.0f, -1.0f, +0.0f, 1.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, +0.0f, -1.0f, +0.0f, 1.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, +0.0f, -1.0f, +0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, +0.0f, -1.0f, +0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, +0.0f, -1.0f, +0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f};

    unsigned int cube_indices[] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35};

    struct mesh *cube_mesh = mesh_create(
        cube_vertices,
        sizeof(cube_vertices),
        cube_indices,
        sizeof(cube_indices));

    if (!cube_mesh)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct mesh *monkey_mesh = mesh_create_obj("assets/models/monkey.obj");

    if (!monkey_mesh)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct texture *box_diffuse_texture = texture_create("assets/images/box_diffuse.png");

    if (!box_diffuse_texture)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct texture *box_specular_texture = texture_create("assets/images/box_specular.png");

    if (!box_specular_texture)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct texture *matrix_texture = texture_create("assets/images/matrix.jpg");

    if (!matrix_texture)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct material *box_material = material_create(
        box_diffuse_texture,
        box_specular_texture,
        NULL,
        (vec3){1.0f, 1.0f, 1.0f},
        32.0f,
        1.0f);

    if (!box_material)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct object *object = object_create(
        cube_mesh,
        box_material,
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){0.0f, 0.0f, 0.0f},
        (vec3){1.0f, 1.0f, 1.0f});

    if (!object)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    struct directional_light *directional_light = directional_light_create(
        (vec3){-0.2f, -1.0f, -0.3f},
        (vec3){0.05f, 0.05f, 0.05f},
        (vec3){0.4f, 0.4f, 0.4f},
        (vec3){1.0f, 0.0f, 0.0f});

    struct point_light *point_lights[] = {
        // red
        point_light_create(
            (vec3){2.0f, 0.0f, 2.0f},
            (vec3){0.1f, 0.0f, 0.0f},
            (vec3){0.8f, 0.0f, 0.0f},
            (vec3){1.0f, 0.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        // yellow
        point_light_create(
            (vec3){-2.0f, 0.0f, -2.0f},
            (vec3){0.1f, 0.1f, 0.0f},
            (vec3){0.8f, 0.8f, 0.0f},
            (vec3){1.0f, 1.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        // green
        point_light_create(
            (vec3){2.0f, 0.0f, -2.0f},
            (vec3){0.0f, 0.1f, 0.0f},
            (vec3){0.0f, 0.8f, 0.0f},
            (vec3){0.0f, 1.0f, 0.0f},
            1.0f,
            0.09f,
            0.032f),
        // blue
        point_light_create(
            (vec3){-2.0f, 0.0f, 2.0f},
            (vec3){0.0f, 0.0f, 0.1f},
            (vec3){0.0f, 0.0f, 0.8f},
            (vec3){0.0f, 0.0f, 1.0f},
            1.0f,
            0.09f,
            0.032f)};

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

    if (!camera)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

#if NETWORK
    IPaddress server_address;

    if (net_resolve_host(&server_address, SERVER_HOST, SERVER_PORT))
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    TCPsocket tcp_socket = net_tcp_open(&server_address);

    if (!tcp_socket)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    TCPpacket *tcp_packet = net_tcp_alloc_packet(PACKET_SIZE);

    if (!tcp_packet)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    UDPsocket udp_socket = net_udp_open(0);

    if (!udp_socket)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    UDPpacket *udp_packet = net_udp_alloc_packet(PACKET_SIZE);

    if (!udp_packet)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    SDLNet_SocketSet socket_set = net_alloc_socket_set(2);

    if (!socket_set)
    {
        printf("Error: %s\n", error_get());

        return 1;
    }

    net_tcp_add_socket(socket_set, tcp_socket);
    net_udp_add_socket(socket_set, udp_socket);

    int client_id = -1;

    if (net_tcp_recv(tcp_socket, tcp_packet) == 1)
    {
        struct data *data = (struct data *)tcp_packet->data;

        switch (data->type)
        {
        case DATA_CONNECT_OK:
        {
            struct id_data *id_data = (struct id_data *)data;

            printf("Server assigned ID: %d\n", id_data->id);

            client_id = id_data->id;
        }
        break;
        case DATA_CONNECT_FULL:
        {
            printf("Server is full\n");

            return 1;
        }
        break;
        default:
        {
            printf("TCP: Unknown packet type\n");

            return 1;
        }
        break;
        }
    }

    {
        struct id_data id_data = id_data_create(DATA_UDP_CONNECT_REQUEST, client_id);
        net_udp_send(udp_socket, udp_packet, server_address, &id_data, sizeof(id_data));
    }
#endif

    time_cap_fps(FPS_CAP);

    window_toggle_mouse();

    GLint basic_program_time = program_get_location(basic_program, "time");
    GLint basic_program_material_diffuse = program_get_location(basic_program, "material.diffuse");
    GLint basic_program_material_color = program_get_location(basic_program, "material.color");
    GLint basic_program_object_model = program_get_location(basic_program, "object.model");
    GLint basic_program_camera_projection = program_get_location(basic_program, "camera.projection");
    GLint basic_program_camera_view = program_get_location(basic_program, "camera.view");
    GLint basic_program_camera_position = program_get_location(basic_program, "camera.position");

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

    program_bind(basic_program);
    program_set_int(basic_program_material_diffuse, 0);
    program_unbind();

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
                case SDLK_F1:
                {
                    current_program = basic_program;
                }
                break;
                case SDLK_F2:
                {
                    current_program = phong_program;
                }
                break;
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
                    else
                    {
#if NETWORK
                        struct chat_data chat_data = chat_data_create(DATA_CHAT_REQUEST, client_id, "Hello, World!");
                        net_tcp_send(tcp_socket, &chat_data, sizeof(chat_data));
#endif
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
            case SDL_MOUSEBUTTONDOWN:
            {
#if NETWORK
                struct mouse_data mouse_data = mouse_data_create(DATA_MOUSEDOWN_REQUEST, client_id, event.button.x, event.button.y);
                net_udp_send(udp_socket, udp_packet, server_address, &mouse_data, sizeof(mouse_data));
#endif
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

#if NETWORK
        while (SDLNet_CheckSockets(socket_set, 0) > 0)
        {
            if (SDLNet_SocketReady(tcp_socket))
            {
                if (net_tcp_recv(tcp_socket, tcp_packet) == 1)
                {
                    struct data *data = (struct data *)tcp_packet->data;

                    switch (data->type)
                    {
                    case DATA_CONNECT_BROADCAST:
                    {
                        struct id_data *id_data = (struct id_data *)data;

                        printf("Client with ID %d has joined\n", id_data->id);
                    }
                    break;
                    case DATA_DISCONNECT_BROADCAST:
                    {
                        struct id_data *id_data = (struct id_data *)data;

                        printf("Client with ID %d has disconnected\n", id_data->id);
                    }
                    break;
                    case DATA_CHAT_BROADCAST:
                    {
                        struct chat_data *chat_data = (struct chat_data *)data;

                        printf("Client %d: %s\n", chat_data->id, chat_data->message);
                    }
                    break;
                    default:
                    {
                        printf("TCP: Unknown packet type\n");
                    }
                    break;
                    }
                }
            }

            if (SDLNet_SocketReady(udp_socket))
            {
                if (net_udp_recv(udp_socket, udp_packet) == 1)
                {
                    struct data *data = (struct data *)udp_packet->data;

                    switch (data->type)
                    {
                    default:
                    {
                        printf("UDP: Unknown packet type\n");
                    }
                    break;
                    }
                }
            }
        }
#endif

        glm_vec_copy(camera->position, spot_light->position);
        glm_vec_copy(camera->front, spot_light->direction);

        mat4 camera_projection;
        camera_calc_projection(camera, camera_projection);

        mat4 camera_view;
        camera_calc_view(camera, camera_view);

        program_bind(basic_program);
        program_set_int(basic_program_time, time_current());
        program_set_mat4(basic_program_camera_projection, camera_projection);
        program_set_mat4(basic_program_camera_view, camera_view);
        program_set_vec3(basic_program_camera_position, camera->position);
        program_unbind();

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

        object_update(object);

        window_clear();

        mat4 model = GLM_MAT4_IDENTITY_INIT;
        object_calc_model(object, model);

        program_bind(basic_program);
        program_set_mat4(basic_program_object_model, model);
        program_set_vec3(basic_program_material_color, object->material->color);
        program_unbind();

        program_bind(phong_program);
        program_set_mat4(phong_program_object_model, model);
        program_set_vec3(phong_program_material_color, object->material->color);
        program_set_float(phong_program_material_shininess, object->material->shininess);
        program_set_float(phong_program_material_glow, object->material->glow);
        program_unbind();

        program_bind(current_program);
        object_draw(object);
        program_unbind();

        window_render();

        time_frame_end();
    }

#if NETWORK
    {
        struct data data = data_create(DATA_DISCONNECT_REQUEST);
        net_tcp_send(tcp_socket, &data, sizeof(data));
    }

    SDLNet_UDP_DelSocket(socket_set, udp_socket);
    SDLNet_TCP_DelSocket(socket_set, tcp_socket);
    SDLNet_FreeSocketSet(socket_set);
    net_udp_free_packet(udp_packet);
    SDLNet_UDP_Close(udp_socket);
    net_tcp_free_packet(tcp_packet);
    SDLNet_TCP_Close(tcp_socket);
#endif

    object_destroy(object);
    material_destroy(box_material);
    texture_destroy(matrix_texture);
    texture_destroy(box_specular_texture);
    texture_destroy(box_diffuse_texture);
    mesh_destroy(monkey_mesh);
    mesh_destroy(cube_mesh);
    mesh_destroy(quad_mesh);
    program_destroy(phong_program);
    program_destroy(basic_program);
    audio_quit();
    window_quit();
    engine_quit();

    return 0;
}