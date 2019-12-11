#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <SDL/SDL.h>

namespace pk
{
class display
{
public:
    display(const char *title, int width, int height);
    ~display();

    void set_title(const char *title);
    void set_window_size(int width, int height);
    void toggle_fullscreen();

    void swap();

private:
    SDL_Window *window;
    SDL_GLContext context;
};
} // namespace pk

#endif
