#include "texture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>

pk::texture::texture(const std::string &filename, bool srgb)
{
    SDL_Surface *surface = IMG_Load(filename.c_str());
    if (!surface)
    {
        spdlog::error("Failed to load texture: {}", IMG_GetError());
        return;
    }

    GLenum internal_format;
    GLenum format;
    if (surface->format->BytesPerPixel == 1)
    {
        internal_format = format = GL_RED;
    }
    else if (surface->format->BytesPerPixel == 3)
    {
        internal_format = srgb ? GL_SRGB : GL_RGB;
        format = GL_RGB;
    }
    else if (surface->format->BytesPerPixel == 4)
    {
        internal_format = srgb ? GL_SRGB_ALPHA : GL_RGBA;
        format = GL_RGBA;
    }

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internal_format,
            surface->w,
            surface->h,
            0,
            format,
            GL_UNSIGNED_BYTE,
            surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surface);
}

pk::texture::~texture()
{
    glDeleteTextures(1, &texture_id);
}

void pk::texture::bind(unsigned int index) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}
