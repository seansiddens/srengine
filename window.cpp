#include "window.h"

#include "log.h"

namespace sren {

static f32 sdl_get_monitor_refresh() {
    SDL_DisplayMode current;
    if (SDL_GetCurrentDisplayMode(0, &current) != 0) {
        LOG_ERR("Failed to fetch monitor refresh rate: %s", SDL_GetError());
    }
    return 1.0f / current.refresh_rate;
}

bool Window::init(u32 width_, u32 height_, const char *window_title) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        LOG_ERR("SDL_Init error: %s", SDL_GetError());
        return false;
    }
    width = width_;
    height = height_;

    // TODO: Initialize w/ user passed window config.
    auto window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    window_handle = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, width, height, window_flags);

    display_refresh = sdl_get_monitor_refresh();
    return true;
}

void Window::teardown() {
    SDL_DestroyWindow(window_handle);
    SDL_Quit();
}

void Window::handle_os_messages() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        // TODO: Implement Imgui
        // ImGui_ImplSDL2_ProcessEvent( &event );

        switch (event.type) {
        case SDL_QUIT: {
            requested_exit = true;
            goto propagate_event;
            break;
        }

        // Handle subevent
        case SDL_WINDOWEVENT: {
            switch (event.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
            case SDL_WINDOWEVENT_RESIZED: {
                {
                    u32 new_width = (u32)(event.window.data1);
                    u32 new_height = (u32)(event.window.data2);

                    // Update only if needed.
                    if (new_width != width || new_height != height) {
                        resized = true;
                        width = new_width;
                        height = new_height;

                        LOG_DBG("Resizing to %u, %u", width, height);
                    }
                }

                break;
            }

            case SDL_WINDOWEVENT_FOCUS_GAINED: {
                LOG_DBG("Focus Gained");
                break;
            }
            case SDL_WINDOWEVENT_FOCUS_LOST: {
                LOG_DBG("Focus Lost");
                break;
            }
            case SDL_WINDOWEVENT_MAXIMIZED: {
                LOG_DBG("Maximized");
                minimized = false;
                break;
            }
            case SDL_WINDOWEVENT_MINIMIZED: {
                LOG_DBG("Minimized");
                minimized = true;
                break;
            }
            case SDL_WINDOWEVENT_RESTORED: {
                LOG_DBG("Restored");
                minimized = false;
                break;
            }
            case SDL_WINDOWEVENT_TAKE_FOCUS: {
                LOG_DBG("Take Focus");
                break;
            }
            case SDL_WINDOWEVENT_EXPOSED: {
                LOG_DBG("Exposed");
                break;
            }

            case SDL_WINDOWEVENT_CLOSE: {
                requested_exit = true;
                LOG_DBG("Window close event received.");
                break;
            }
            default: {
                display_refresh = sdl_get_monitor_refresh();
                break;
            }
            }
            goto propagate_event;
            break;
        }
        }
    // Maverick:
    propagate_event:
        // Callbacks
        for (u32 i = 0; i < os_messages_callbacks.size(); ++i) {
            OsMessagesCallback callback = os_messages_callbacks[i];
            callback(&event, os_messages_callbacks_data[i]);
        }
    }
}

} // namespace sren