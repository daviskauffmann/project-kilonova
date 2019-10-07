#include "point_light.hpp"

#include <iostream>

namespace pk
{
point_light::point_light(
    glm::vec3 position,
    glm::vec3 color)
    : position(position),
      color(color)
{
    glGenTextures(1, &depth_cubemap_texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap_texture_id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_DEPTH_COMPONENT,
            512,
            512,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &depth_cubemap_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_cubemap_fbo_id);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        depth_cubemap_texture_id,
        0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: Couldn't complete depth cubemap framebuffer" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

point_light::~point_light()
{
    glDeleteTextures(1, &depth_cubemap_texture_id);
    glDeleteFramebuffers(1, &depth_cubemap_fbo_id);
}
} // namespace pk
