#ifndef ENGINE_SOUND_H
#define ENGINE_SOUND_H

struct sound
{
    ALuint buffer_id;
};

struct sound *sound_create(const char *filename);
void sound_destroy(struct sound *sound);

#endif