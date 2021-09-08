#pragma once

#ifdef __linux__
    #define GLFW_EXPOSE_NATIVE_X11
#endif // __linux__
#ifdef _WIN64
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN64

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef GLFW_EXPOSE_NATIVE_X11
/// Aliasing the X11 platform's native window class
/// before declaring ours to avoid conflict with our
/// Window class.
using X11Window = ::Window;
#endif // GLFW_EXPOSE_NATIVE_X11


namespace stlr {
    class Window {
    public:
#ifdef GLFW_EXPOSE_NATIVE_X11
        struct X11Info {
            Display* display;
            X11Window window;

            X11Info( Display* display, X11Window window );
        };
#endif // GLFW_EXPOSE_NATIVE_X11
    private:
        static inline int glfw_initialized = GLFW_FALSE;
        int width;
        int height;
        const char* title;
        GLFWwindow* window;

    public:
        Window( int width = 500, int height = 500, const char* title = "Title" );

        constexpr int get_width() const {
            return width;
        }

        constexpr int get_height() const {
            return  height;
        }

        constexpr const char* get_title() const {
            return title;
        }

        void close();

#ifdef GLFW_EXPOSE_NATIVE_X11
        const X11Info get_x11_info() const;
#endif // GLFW_EXPOSE_NATIVE_X11

#ifdef GLFW_EXPOSE_NATIVE_WIN32
        const HWND get_hwnd() const;
#endif // GLFW_EXPOSE_NATIVE_WIN32

    private:
        static void initialize_glfw();
        static void error_callback( int code, const char* description );
    };
}
