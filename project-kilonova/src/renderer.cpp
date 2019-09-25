#include "renderer.hpp"

#include <iostream>

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define LIGHTING_COLOR 1
#define LIGHTING_TEXTURE 2
#define LIGHTING_FORWARD 3
#define LIGHTING_DEFERRED 4
#define LIGHTING_MODE LIGHTING_FORWARD

namespace pk
{
renderer::renderer(int render_width, int render_height, float render_scale, int shadow_width, int shadow_height)
{
    this->render_width = render_width;
    this->render_height = render_height;
    this->render_scale = render_scale;
    this->shadow_width = shadow_width;
    this->shadow_height = shadow_height;
    this->reflection_width = render_width;
    this->reflection_height = render_height;
    this->refraction_width = render_width;
    this->refraction_height = render_height;

    // init GLEW
    {
        GLenum glewError = glewInit();

        if (glewError != GLEW_OK)
        {
            std::cout << "Error: Couldn't initialize GLEW\n"
                      << glewGetErrorString(glewError) << std::endl;
        }
    }

    std::cout << "GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Vendor " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // setup opengl
    glViewport(0, 0, render_width, render_height);
    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);

    // create programs
    this->depth_program = new pk::program(
        "assets/shaders/depth.vert",
        "",
        "assets/shaders/depth.frag");
    this->depth_cube_program = new pk::program(
        "assets/shaders/depth_cube.vert",
        "assets/shaders/depth_cube.geom",
        "assets/shaders/depth_cube.frag");
    this->color_program = new pk::program(
        "assets/shaders/color.vert",
        "",
        "assets/shaders/color.frag");
    this->texture_program = new pk::program(
        "assets/shaders/texture.vert",
        "",
        "assets/shaders/texture.frag");
    this->forward_program = new pk::program(
        "assets/shaders/forward.vert",
        "",
        "assets/shaders/forward.frag");
    this->geometry_program = new pk::program(
        "assets/shaders/geometry.vert",
        "",
        "assets/shaders/geometry.frag");
    this->skybox_program = new pk::program(
        "assets/shaders/skybox.vert",
        "",
        "assets/shaders/skybox.frag");
    this->water_program = new pk::program(
        "assets/shaders/water.vert",
        "",
        "assets/shaders/water.frag");
    this->sprite_program = new pk::program(
        "assets/shaders/sprite.vert",
        "",
        "assets/shaders/sprite.frag");
    this->screen_program = new pk::program(
        "assets/shaders/screen.vert",
        "",
        "assets/shaders/screen.frag");

    // setup shader samplers
    this->texture_program->bind();
    this->texture_program->set_int("color_map", 0);
    this->texture_program->unbind();

    this->forward_program->bind();
    this->forward_program->set_int("material.albedo_map", 0);
    this->forward_program->set_int("material.normal_map", 1);
    this->forward_program->set_int("material.metallic_map", 2);
    this->forward_program->set_int("material.roughness_map", 3);
    this->forward_program->set_int("material.ao_map", 4);
    this->forward_program->set_int("light.depth_map", 5);
    this->forward_program->set_int("light.depth_cubemap", 6);
    this->forward_program->unbind();

    this->geometry_program->bind();
    this->geometry_program->set_int("material.diffuse_map", 0);
    this->geometry_program->set_int("material.specular_map", 1);
    this->geometry_program->set_int("material.normal_map", 2);
    this->geometry_program->set_int("material.emission_map", 3);
    this->geometry_program->unbind();

    this->skybox_program->bind();
    this->skybox_program->set_int("skybox.color_map", 0);
    this->skybox_program->unbind();

    this->water_program->bind();
    this->water_program->set_int("water.reflection_map", 0);
    this->water_program->set_int("water.refraction_map", 1);
    this->water_program->set_int("water.dudv_map", 2);
    this->water_program->set_int("water.normal_map", 3);
    this->water_program->unbind();

    this->sprite_program->bind();
    this->sprite_program->set_int("sprite.color_map", 0);
    this->sprite_program->unbind();

    this->screen_program->bind();
    this->screen_program->set_int("screen.color_map", 0);
    this->screen_program->unbind();

    // setup screen fbo
    glGenFramebuffers(1, &this->screen_fbo_id);

    glGenTextures(1, &this->screen_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->screen_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        render_width,
        render_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &this->screen_rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, this->screen_rbo_id);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        render_width,
        render_height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->screen_fbo_id);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        this->screen_texture_id,
        0);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        this->screen_rbo_id);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: Couldn't complete screen framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup geometry fbo
    glGenFramebuffers(1, &this->geometry_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, this->geometry_fbo_id);

    glGenTextures(1, &this->geometry_position_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->geometry_position_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB16F,
        (GLsizei)(render_width * render_scale),
        (GLsizei)(render_height * render_scale),
        0,
        GL_RGB,
        GL_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        this->geometry_position_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &this->geometry_normal_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->geometry_normal_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB16F,
        (GLsizei)(render_width * render_scale),
        (GLsizei)(render_height * render_scale),
        0,
        GL_RGB,
        GL_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT1,
        GL_TEXTURE_2D,
        this->geometry_normal_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &this->geometry_albedo_specular_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->geometry_albedo_specular_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        (GLsizei)(render_width * render_scale),
        (GLsizei)(render_height * render_scale),
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT2,
        GL_TEXTURE_2D,
        this->geometry_albedo_specular_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum geometry_fbo_attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(sizeof(geometry_fbo_attachments) / sizeof(GLenum), geometry_fbo_attachments);

    glGenRenderbuffers(1, &this->geometry_rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, this->geometry_rbo_id);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT,
        (GLsizei)(render_width * render_scale),
        (GLsizei)(render_height * render_scale));

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        this->geometry_rbo_id);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: Couldn't complete geometry framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup water reflection fbo
    glGenFramebuffers(1, &this->water_reflection_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, this->water_reflection_fbo_id);

    glGenTextures(1, &this->water_reflection_color_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->water_reflection_color_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        this->reflection_width,
        this->reflection_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        this->water_reflection_color_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum water_reflection_fbo_attachments[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(sizeof(water_reflection_fbo_attachments) / sizeof(GLenum), water_reflection_fbo_attachments);

    glGenTextures(1, &this->water_reflection_depth_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->water_reflection_depth_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT32,
        this->reflection_width,
        this->reflection_height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        this->water_reflection_depth_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &this->water_reflection_depth_rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, this->water_reflection_depth_rbo_id);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT,
        this->reflection_width,
        this->reflection_height);

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        this->water_reflection_depth_rbo_id);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: Couldn't complete water reflection framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup water refraction fbo
    glGenFramebuffers(1, &this->water_refraction_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, this->water_refraction_fbo_id);

    glGenTextures(1, &this->water_refraction_color_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->water_refraction_color_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        this->refraction_width,
        this->refraction_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        this->water_refraction_color_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum water_refraction_fbo_attachments[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(sizeof(water_refraction_fbo_attachments) / sizeof(GLenum), water_refraction_fbo_attachments);

    glGenTextures(1, &this->water_refraction_depth_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->water_refraction_depth_texture_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT32,
        this->refraction_width,
        this->refraction_height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        this->water_refraction_depth_texture_id,
        0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &this->water_refraction_depth_rbo_id);
    glBindRenderbuffer(GL_RENDERBUFFER, this->water_refraction_depth_rbo_id);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT,
        this->refraction_width,
        this->refraction_height);

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        this->water_refraction_depth_rbo_id);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: Couldn't complete water refraction framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create water mesh
    std::vector<float> water_vertices =
        {-1.0f, -1.0f,
         -1.0f, +1.0f,
         +1.0f, -1.0f,
         +1.0f, -1.0f,
         -1.0f, +1.0f,
         +1.0f, +1.0f};
    this->water_vertices_size = (GLsizei)(water_vertices.size() * sizeof(float));

    glGenVertexArrays(1, &this->water_vao_id);
    glGenBuffers(1, &this->water_vbo_id);

    glBindVertexArray(this->water_vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, this->water_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, this->water_vertices_size, water_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // create skybox mesh
    std::vector<float> skybox_vertices =
        {-1.0f, +1.0f, -1.0f,
         -1.0f, -1.0f, -1.0f,
         +1.0f, -1.0f, -1.0f,
         +1.0f, -1.0f, -1.0f,
         +1.0f, +1.0f, -1.0f,
         -1.0f, +1.0f, -1.0f,

         -1.0f, -1.0f, +1.0f,
         -1.0f, -1.0f, -1.0f,
         -1.0f, +1.0f, -1.0f,
         -1.0f, +1.0f, -1.0f,
         -1.0f, +1.0f, +1.0f,
         -1.0f, -1.0f, +1.0f,

         +1.0f, -1.0f, -1.0f,
         +1.0f, -1.0f, +1.0f,
         +1.0f, +1.0f, +1.0f,
         +1.0f, +1.0f, +1.0f,
         +1.0f, +1.0f, -1.0f,
         +1.0f, -1.0f, -1.0f,

         -1.0f, -1.0f, +1.0f,
         -1.0f, +1.0f, +1.0f,
         +1.0f, +1.0f, +1.0f,
         +1.0f, +1.0f, +1.0f,
         +1.0f, -1.0f, +1.0f,
         -1.0f, -1.0f, +1.0f,

         -1.0f, +1.0f, -1.0f,
         +1.0f, +1.0f, -1.0f,
         +1.0f, +1.0f, +1.0f,
         +1.0f, +1.0f, +1.0f,
         -1.0f, +1.0f, +1.0f,
         -1.0f, +1.0f, -1.0f,

         -1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f, +1.0f,
         +1.0f, -1.0f, -1.0f,
         +1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f, +1.0f,
         +1.0f, -1.0f, +1.0f};
    this->skybox_vertices_size = (GLsizei)(skybox_vertices.size() * sizeof(float));

    glGenVertexArrays(1, &this->skybox_vao_id);
    glGenBuffers(1, &this->skybox_vbo_id);

    glBindVertexArray(this->skybox_vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, this->skybox_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, this->skybox_vertices_size, skybox_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // create sprite mesh
    std::vector<float> sprite_vertices =
        {+0.0f, +1.0f, +0.0f, +1.0f,
         +1.0f, +0.0f, +1.0f, +0.0f,
         +0.0f, +0.0f, +0.0f, +0.0f,

         +0.0f, +1.0f, +0.0f, +1.0f,
         +1.0f, +1.0f, +1.0f, +1.0f,
         +1.0f, +0.0f, +1.0f, +0.0f};
    this->sprite_vertices_size = (GLsizei)(sprite_vertices.size() * sizeof(float));

    glGenVertexArrays(1, &this->sprite_vao_id);
    glGenBuffers(1, &this->sprite_vbo_id);

    glBindVertexArray(this->sprite_vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, this->sprite_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, this->sprite_vertices_size, sprite_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // create screen mesh
    std::vector<float> screen_vertices =
        {-1.0f, -1.0f, +0.0f, +0.0f,
         -1.0f, +1.0f, +0.0f, +1.0f,
         +1.0f, -1.0f, +1.0f, +0.0f,
         +1.0f, -1.0f, +1.0f, +0.0f,
         -1.0f, +1.0f, +0.0f, +1.0f,
         +1.0f, +1.0f, +1.0f, +1.0f};
    this->screen_vertices_size = (GLsizei)(screen_vertices.size() * sizeof(float));

    glGenVertexArrays(1, &this->screen_vao_id);
    glGenBuffers(1, &this->screen_vbo_id);

    glBindVertexArray(this->screen_vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, this->screen_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, this->screen_vertices_size, screen_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // create water dudv texture
    this->water_dudv_texture = new pk::texture("assets/images/water_dudv.png");

    if (!this->water_dudv_texture)
    {
        std::cout << "Error: Couldn't load water dudv texture" << std::endl;
    }

    // create water normal texture
    this->water_normal_texture = new pk::texture("assets/images/water_normal.png");

    if (!this->water_normal_texture)
    {
        std::cout << "Error: Couldn't load water normal texture" << std::endl;
    }
}

renderer::~renderer()
{
}

void renderer::add_object(pk::object *object)
{
    this->objects.push_back(object);
}

void renderer::add_directional_light(pk::directional_light *directional_light)
{
    this->directional_lights.push_back(directional_light);
}

void renderer::add_point_light(pk::point_light *point_light)
{
    this->point_lights.push_back(point_light);
}

void renderer::add_spot_light(pk::spot_light *spot_light)
{
    this->spot_lights.push_back(spot_light);
}

void renderer::set_skybox(pk::cubemap *skybox)
{
    this->skybox = skybox;
}

void renderer::add_water(pk::water *water)
{
    this->waters.push_back(water);
}

void renderer::add_sprite(pk::sprite *sprite)
{
    this->sprites.push_back(sprite);
}

void renderer::draw(pk::camera *camera, float aspect, unsigned int elapsed_time, float delta_time)
{
    if (!camera)
    {
        std::cout << "Error: Camera cannot be null" << std::endl;

        return;
    }

    // render scene to the screen framebuffer
    this->render_scene(this->screen_fbo_id, camera, aspect, elapsed_time, glm::vec4(0.0f));

    // render water
    if (this->waters.size() > 0)
    {
        render_waters(this->screen_fbo_id, camera, aspect, elapsed_time);
    }

    // render sprites
    if (this->sprites.size() > 0)
    {
        render_sprites(this->screen_fbo_id, aspect);
    }

    // render the screen framebuffer to the default framebuffer and apply post-processing
    render_screen(0);

    // clear renderables
    this->objects.clear();
    this->directional_lights.clear();
    this->point_lights.clear();
    this->spot_lights.clear();
    this->waters.clear();
    this->sprites.clear();
}

void renderer::render_scene(GLuint fbo_id, pk::camera *camera, float aspect, unsigned int elapsed_time, glm::vec4 clipping_plane)
{
    // clear the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glViewport(0, 0, this->render_width, this->render_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // calculate camera projection matrix
    glm::mat4 camera_projection = camera->calc_projection(aspect);

    // calculate camera view matrix
    glm::mat4 camera_view = camera->calc_view();

    for (auto &directional_light : this->directional_lights)
    {
        // calculate sun projection matrix
        directional_light->projection = directional_light->calc_projection();

        // calculate sun view matrix
        directional_light->view = directional_light->calc_view();

        // render sun shadows to depthmap
        glBindFramebuffer(GL_FRAMEBUFFER, directional_light->depthmap_fbo_id);

        glViewport(0, 0, this->shadow_width, this->shadow_height);
        glClear(GL_DEPTH_BUFFER_BIT);

        this->depth_program->bind();

        this->depth_program->set_mat4("projection", directional_light->projection);
        this->depth_program->set_mat4("view", directional_light->view);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->depth_program->set_mat4("object.model", object_model);

            object->mesh->draw();
        }

        this->depth_program->unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    for (auto &point_light : this->point_lights)
    {
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)4096 / (float)4096, 1.0f, 25.0f);

        std::vector<glm::mat4> shadow_matrices;
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadow_matrices.push_back(projection * glm::lookAt(point_light->position, point_light->position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        glBindFramebuffer(GL_FRAMEBUFFER, point_light->depth_cubemap_fbo_id);

        glViewport(0, 0, this->shadow_width, this->shadow_height);
        glClear(GL_DEPTH_BUFFER_BIT);

        this->depth_cube_program->bind();

        for (unsigned int i = 0; i < 6; i++)
        {
            this->depth_cube_program->set_mat4("shadow_matrices[" + std::to_string(i) + "]", shadow_matrices[i]);
        }

        this->depth_cube_program->set_vec3("light.position", point_light->position);

        this->depth_cube_program->set_float("far_plane", 25.0f);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->depth_cube_program->set_mat4("object.model", object_model);

            object->mesh->draw();
        }

        this->depth_cube_program->unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    if (this->objects.size() > 0)
    {
#if LIGHTING_MODE == LIGHTING_COLOR
        // draw objects to the specified framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glViewport(0, 0, this->render_width, this->render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CLIP_DISTANCE0);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->color_program->bind();

            this->color_program->set_mat4("camera.projection", camera_projection);
            this->color_program->set_mat4("camera.view", camera_view);

            this->color_program->set_mat4("object.model", object_model);

            this->color_program->set_vec4("clipping_plane", clipping_plane);

            this->color_program->set_vec3("color", object->material->color);

            object->mesh->draw();

            this->color_program->unbind();
        }

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#elif LIGHTING_MODE == LIGHTING_TEXTURE
        // draw objects to the specified framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glViewport(0, 0, this->render_width, this->render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CLIP_DISTANCE0);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->texture_program->bind();

            this->texture_program->set_mat4("camera.projection", camera_projection);
            this->texture_program->set_mat4("camera.view", camera_view);

            this->texture_program->set_mat4("object.model", object_model);

            this->texture_program->set_vec4("clipping_plane", clipping_plane);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, object->material->albedo_map ? object->material->albedo_map->texture_id : 0);

            object->mesh->draw();

            this->texture_program->unbind();
        }

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#elif LIGHTING_MODE == LIGHTING_FORWARD
        // draw objects to the specified framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glViewport(0, 0, this->render_width, this->render_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CLIP_DISTANCE0);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->color_program->bind();

            this->color_program->set_mat4("camera.projection", camera_projection);
            this->color_program->set_mat4("camera.view", camera_view);

            this->color_program->set_mat4("object.model", object_model);

            this->color_program->set_vec4("clipping_plane", clipping_plane);

            this->color_program->set_vec3("color", glm::vec3(0.0f, 0.0f, 0.0f));

            object->mesh->draw();

            this->color_program->unbind();

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_EQUAL);

            if (this->directional_lights.size() > 0)
            {
                this->forward_program->bind();

                this->forward_program->set_mat4("camera.projection", camera_projection);
                this->forward_program->set_mat4("camera.view", camera_view);
                this->forward_program->set_vec3("camera.position", camera->position);

                this->forward_program->set_mat4("object.model", object_model);

                this->forward_program->set_vec4("clipping_plane", clipping_plane);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, object->material->albedo_map ? object->material->albedo_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, object->material->normal_map ? object->material->normal_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, object->material->metallic_map ? object->material->metallic_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, object->material->roughness_map ? object->material->roughness_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, object->material->ao_map ? object->material->ao_map->texture_id : 0);

                this->forward_program->set_int("light.type", 0);

                for (auto &directional_light : this->directional_lights)
                {
                    this->forward_program->set_vec3("light.direction", directional_light->direction);
                    this->forward_program->set_vec3("light.color", directional_light->color);
                    this->forward_program->set_mat4("light.projection", directional_light->projection);
                    this->forward_program->set_mat4("light.view", directional_light->view);

                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, directional_light->depthmap_texture_id);

                    object->mesh->draw();
                }

                this->forward_program->unbind();
            }

            if (this->point_lights.size() > 0)
            {
                this->forward_program->bind();

                this->forward_program->set_mat4("camera.projection", camera_projection);
                this->forward_program->set_mat4("camera.view", camera_view);
                this->forward_program->set_vec3("camera.position", camera->position);

                this->forward_program->set_mat4("object.model", object_model);

                this->forward_program->set_vec4("clipping_plane", clipping_plane);

                this->forward_program->set_float("far_plane", 25.0f);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, object->material->albedo_map ? object->material->albedo_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, object->material->normal_map ? object->material->normal_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, object->material->metallic_map ? object->material->metallic_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, object->material->roughness_map ? object->material->roughness_map->texture_id : 0);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, object->material->ao_map ? object->material->ao_map->texture_id : 0);

                this->forward_program->set_int("light.type", 1);

                for (auto &point_light : this->point_lights)
                {
                    this->forward_program->set_vec3("light.position", point_light->position);
                    this->forward_program->set_vec3("light.color", point_light->color);

                    glActiveTexture(GL_TEXTURE6);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, point_light->depth_cubemap_texture_id);

                    object->mesh->draw();
                }

                this->forward_program->unbind();
            }

            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#elif LIGHTING_MODE == LIGHTING_DEFERRED
        // draw objects to the gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, this->geometry_fbo_id);

        glViewport(0, 0, (GLsizei)(this->render_width * this->render_scale), (GLsizei)(this->render_height * this->render_scale));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CLIP_DISTANCE0);

        this->geometry_program->bind();

        this->geometry_program->set_mat4("camera.projection", camera_projection);
        this->geometry_program->set_mat4("camera.view", camera_view);
        this->geometry_program->set_vec3("camera.position", camera->position);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->geometry_program->set_mat4("object.model", object_model);

            this->geometry_program->set_vec4("clipping_plane", clipping_plane);

            this->geometry_program->set_vec3("material.color", object->material->color);
            this->geometry_program->set_float("material.shininess", object->material->shininess);
            this->geometry_program->set_float("material.glow", object->material->glow);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, object->material->diffuse_map ? object->material->diffuse_map->texture_id : 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, object->material->specular_map ? object->material->specular_map->texture_id : 0);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, object->material->normal_map ? object->material->normal_map->texture_id : 0);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, object->material->emission_map ? object->material->emission_map->texture_id : 0);

            object->mesh->draw();
        }

        this->geometry_program->unbind();

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // draw the gbuffer to the specified framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        glViewport(0, 0, this->render_width, this->render_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        for (auto &object : this->objects)
        {
            glm::mat4 object_model = object->calc_model();

            this->color_program->bind();

            this->color_program->set_mat4("camera.projection", camera_projection);
            this->color_program->set_mat4("camera.view", camera_view);

            this->color_program->set_mat4("object.model", object_model);

            this->color_program->set_vec4("clipping_plane", clipping_plane);

            this->color_program->set_vec3("color", glm::vec3(0.0f, 0.0f, 0.0f));

            object->mesh->draw();
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_EQUAL);

        // TODO: deferred pbr lighting

        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glEnable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // copy depth information to specified framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, this->geometry_fbo_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
        glBlitFramebuffer(0, 0, this->render_width, this->render_height, 0, 0, this->render_width, this->render_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    }

    // draw other things that need to be drawn after the deferred rendering pass
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glViewport(0, 0, this->render_width, this->render_height);

    // render skybox
    if (this->skybox)
    {
        glDepthFunc(GL_LEQUAL);

        glm::mat4 camera_view_no_translate = camera_view;
        // camera_view_no_translate = glm::rotate(camera_view_no_translate, glm::radians((float)elapsed_time) * 0.001f, glm::vec3(0.0f, 1.0f, 0.0f));
        camera_view_no_translate[3][0] = 0.0f;
        camera_view_no_translate[3][1] = 0.0f;
        camera_view_no_translate[3][2] = 0.0f;

        this->skybox_program->bind();

        this->skybox_program->set_mat4("camera.projection", camera_projection);
        this->skybox_program->set_mat4("camera.view", camera_view_no_translate);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox->texture_id);

        glBindVertexArray(this->skybox_vao_id);
        glDrawArrays(GL_TRIANGLES, 0, this->skybox_vertices_size);
        glBindVertexArray(0);

        this->skybox_program->unbind();

        glDepthFunc(GL_LESS);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer::render_waters(GLuint fbo_id, pk::camera *camera, float aspect, unsigned int elapsed_time)
{
    // calculate camera projection matrix
    glm::mat4 camera_projection = camera->calc_projection(aspect);

    // calculate camera view matrix
    glm::mat4 camera_view = camera->calc_view();

    // send camera transformations to water program
    this->water_program->bind();

    this->water_program->set_mat4("camera.projection", camera_projection);
    this->water_program->set_mat4("camera.view", camera_view);
    this->water_program->set_vec3("camera.position", camera->position);

    this->water_program->set_unsigned_int("elapsed_time", elapsed_time);

    this->water_program->unbind();

    for (auto &water : this->waters)
    {
        // render water reflection
        glm::vec4 reflection_clipping_plane = {0.0f, 1.0f, 0.0f, -water->position[1]};
        bool reflect = false;

        if (camera->position[1] > water->position[1]) // don't draw reflections if under the water
        {
            reflect = true;

            float old_camera_y = camera->position[1];
            float old_camera_pitch = camera->pitch;

            camera->position[1] -= 2 * (camera->position[1] - water->position[1]);
            camera->pitch = -camera->pitch;

            render_scene(this->water_reflection_fbo_id, camera, aspect, elapsed_time, reflection_clipping_plane);

            camera->position[1] = old_camera_y;
            camera->pitch = old_camera_pitch;
        }

        // render water refraction
        glm::vec4 refraction_clipping_plane = {0.0f, -1.0f, 0.0f, water->position[1]};

        if (camera->position[1] < water->position[1]) // flip refraction clipping plane if under the water
        {
            refraction_clipping_plane[1] = 1.0f;
            refraction_clipping_plane[3] = -water->position[1];
        }

        render_scene(this->water_refraction_fbo_id, camera, aspect, elapsed_time, refraction_clipping_plane);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

        this->water_program->bind();

        glm::mat4 water_model(1.0f);
        water_model = glm::translate(water_model, water->position);
        water_model = glm::scale(water_model, {water->scale.x, 1.0f, water->scale.y});

        this->water_program->set_mat4("water.model", water_model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflect ? this->water_reflection_color_texture_id : 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->water_refraction_color_texture_id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, this->water_dudv_texture ? this->water_dudv_texture->texture_id : 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, this->water_normal_texture ? this->water_normal_texture->texture_id : 0);

        glBindVertexArray(this->water_vao_id);
        glDrawArrays(GL_TRIANGLES, 0, this->water_vertices_size);
        glBindVertexArray(0);

        this->water_program->unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void renderer::render_sprites(GLuint fbo_id, float aspect)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    this->sprite_program->bind();

    // calculate ortho projection
    glm::mat4 camera_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);

    this->sprite_program->set_mat4("camera.projection", camera_projection);

    for (auto &sprite : this->sprites)
    {
        glm::mat4 sprite_model = sprite->calc_model();

        this->sprite_program->set_mat4("sprite.model", sprite_model);
        this->sprite_program->set_vec3("sprite.color", sprite->color);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite->color_map->texture_id);

        glBindVertexArray(this->sprite_vao_id);
        glDrawArrays(GL_TRIANGLES, 0, this->sprite_vertices_size);
        glBindVertexArray(0);
    }

    this->sprite_program->unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer::render_screen(GLuint fbo_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glViewport(0, 0, this->render_width, this->render_height);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    this->screen_program->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->screen_texture_id);

    glBindVertexArray(this->screen_vao_id);
    glDrawArrays(GL_TRIANGLES, 0, this->screen_vertices_size);
    glBindVertexArray(0);

    this->screen_program->unbind();

    glEnable(GL_DEPTH_TEST);
}
} // namespace pk