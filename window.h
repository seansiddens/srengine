#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "platform.h"

namespace sren {

typedef void (*OsMessagesCallback)(void *os_event, void *user_data);

class Window {
  public:
    bool init(u32 width, u32 height);
    void teardown();

    void handle_os_messages();

    u32 width = 0;
    u32 height = 0;

  private:
    SDL_Window *window_handle;

    std::vector<OsMessagesCallback> os_messages_callbacks;
    std::vector<void *> os_messages_callbacks_data;

    bool requested_exit = false;
    bool resized = false;
    bool minimized = false;
    u32 width = 0;
    u32 height = 0;
    f32 display_refresh = 1.0f / 60.0f;
};

} // namespace sren