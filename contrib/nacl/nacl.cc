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
#include <ppapi/cpp/var.h>
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
    }

    bool HandleInputEvent(const pp::InputEvent& event) {
        SDL_NACL_PushEvent(event);
        return true;
    }

    void HandleMessage(const pp::Var& message) {
        fprintf(stderr, "HandleMessage\n");
        std::string data = message.AsString();
        int fd = open("program.lua", O_CREAT | O_WRONLY);
        if (fd < 0) {
            perror("open");
            return;
        }

        if (write(fd, data.c_str(), data.length()) < 0) {
            perror("write");
            return;
        }

        if (close(fd) < 0) {
            perror("close");
            return;
        }

        SDL_NACL_SetInstance(pp_instance(), 800, 600);
        int lval = SDL_Init(SDL_INIT_VIDEO);
        assert(lval >= 0);
        pthread_create(&sdl_main_thread_, NULL, sdl_thread, this);
    }

    bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
        if (RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD|PP_INPUTEVENT_CLASS_WHEEL)) {
            throw std::runtime_error("failed to request filtering input events");
        }

        if (RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE)) {
            throw std::runtime_error("failed to request input events");
        }

        return true;
    }

private:
    pthread_t sdl_main_thread_;
    int width_;
    int height_;

    static void* sdl_thread(void* param) {
        char argv0[] = "load81";
        char argv1[] = "program.lua";
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
