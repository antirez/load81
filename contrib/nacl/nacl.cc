#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/input_event.h>
#include <nacl-mounts/memory/MemMount.h>

#include <SDL_video.h>
#include <SDL.h>
#include <SDL_nacl.h>

extern "C" {
    extern int load81_main(int argc, char *argv[]);
    extern int mount(const char *type, const char *dir, int flags, void *data);
    extern char _binary_example_lua_start[];
    extern int _binary_example_lua_size;
}

/*
 * Copied from earth sample in naclports.
 */
class Load81Instance : public pp::Instance {
public:
    explicit Load81Instance(PP_Instance instance)
        : pp::Instance(instance),
          sdl_main_thread_(0),
          width_(0),
          height_(0) {
        printf("PluginInstance\n");
    }

    ~Load81Instance() {
        if (sdl_main_thread_) {
            pthread_join(sdl_main_thread_, NULL);
        }
    }

    virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
        printf("did change view, new %dx%d, old %dx%d\n",
                position.size().width(), position.size().height(),
                width_, height_);

        if (position.size().width() == width_ &&
            position.size().height() == height_)
            return;  // Size didn't change, no need to update anything.

        if (sdl_thread_started_ == false) {
            width_ = position.size().width();
            height_ = position.size().height();

            SDL_NACL_SetInstance(pp_instance(), width_, height_);
            // It seems this call to SDL_Init is required. Calling from
            // sdl_main() isn't good enough.
            // Perhaps it must be called from the main thread?
            int lval = SDL_Init(SDL_INIT_VIDEO);
            assert(lval >= 0);
            if (0 == pthread_create(&sdl_main_thread_, NULL, sdl_thread, this)) {
                sdl_thread_started_ = true;
            }
        }
    }

    bool HandleInputEvent(const pp::InputEvent& event) {
        SDL_NACL_PushEvent(event);
        return true;
    }

    bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
        if (RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD|PP_INPUTEVENT_CLASS_WHEEL)) {
            throw std::runtime_error("failed to request filtering input events");
        }

        if (RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE)) {
            throw std::runtime_error("failed to request input events");
        }

        /* 
         * The example Lua program is embedded in an object file by the NaCl build
         * script.
         * TODO: support loading local files.
         */
        int fd = open("example.lua", O_CREAT | O_WRONLY);
        if (fd < 0) {
            perror("open");
            return false;
        }

        if (write(fd, _binary_example_lua_start, (int)&_binary_example_lua_size) < 0) {
            perror("write");
            return false;
        }

        if (close(fd) < 0) {
            perror("close");
            return false;
        }

        return true;
    }

private:
    bool sdl_thread_started_;
    pthread_t sdl_main_thread_;
    int width_;
    int height_;

    static void* sdl_thread(void* param) {
        char argv0[] = "load81";
        char argv1[] = "example.lua";
        char argv2[] = "--fps";
        char *argv[] = { argv0, argv1, argv2, NULL };
        load81_main(3, argv);
        return NULL;
    }
};

class Load81Module : public pp::Module {
public:
    Load81Module() : pp::Module() {}

    virtual ~Load81Module() {
    }

    virtual bool Init() {
        return true;
    }

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new Load81Instance(instance);
    }
};

namespace pp {
    Module* CreateModule() {
        return new Load81Module();
    }
}
