#include "Window.hpp"
#include <iostream>

namespace stlr {

    Window::X11Info::X11Info( Display* display, X11Window window ) : display( display ), window( window ) {}

    Window::Window( int width, int height, const char* title ) : window( nullptr ) {
        initialize_glfw();
        window = glfwCreateWindow( width, height, title, nullptr, nullptr );
    }

    void Window::close() {
        glfwDestroyWindow( window );
    }

#ifdef GLFW_EXPOSE_NATIVE_X11
    const Window::X11Info Window::get_x11_info() const {
        return Window::X11Info( glfwGetX11Display(), glfwGetX11Window( window ) );
    }
#endif

    void Window::initialize_glfw() {
        if( glfw_initialized == GLFW_TRUE )
            return;

        glfwInit();
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
        glfwSetErrorCallback( error_callback );
        glfw_initialized = GLFW_TRUE;
    }

    void Window::error_callback( int code, const char* description ) {
        std::cerr << "Error (" << code << "): " << description << "\n";
    }
}
