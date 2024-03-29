
#include "window/window.hh"

#include "spdlog/spdlog.h"

namespace Fallment{

Window::Window(std::string title, unsigned w, unsigned h):_width{w},_height{h}{
}

Window::~Window(){

}

bool Window::onInit(){
    
    if (!glfwInit())
        return -1;

    
    _window = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
    if (!_window)
    {
        
        return false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(_window);

    glfwSetWindowSizeCallback(_window, fnptr<void(GLFWwindow*,int,int)>(
        [this](GLFWwindow* window, int width, int height){this->window_size_callback(window, width,height);}
    ));
    glfwSetCursorPosCallback(_window, fnptr<void(GLFWwindow*,double,double)>(
        [this](GLFWwindow* window, double xpos, double ypos){this->mouse_callback(window, xpos,ypos);}
    ));

    glfwSetScrollCallback(_window, fnptr<void(GLFWwindow*,double,double)>(
        [this](GLFWwindow* window, double xoffset, double yoffset){this->scroll_callback(window, xoffset,yoffset);}
    ));

    _dispatcher = std::make_unique<EventDispatcher>();

    return true;
}

bool Window::recreate(std::string title, unsigned w, unsigned h){
    _width = w;
    _height = h;
    //spdlog::debug(" window recreate");
    return true;

}

void Window::onFrame(){
    //spdlog::debug(" Window::onFrame");
    if(_framebuffer.get()){
        //std::unique_lock<std::mutex>(_framebuffer->m_color_mutex);
        glDrawPixels(_width, _height,GL_RGBA, GL_UNSIGNED_BYTE, _framebuffer->getColorBuffer());
        //spdlog::debug("framebuffer{} {} {}", int(_framebuffer->getColorBuffer()),_width,_height);
        glfwSwapBuffers(_window);
    }else{
        spdlog::error("Window surface doesnt have any famebuffer data useful");
    }


}

bool Window::onUpdate(){
    if(glfwWindowShouldClose(_window))return false;
    processInput();
    glfwPollEvents();

    return true;
    
}

void Window::onDestory(){
    glfwTerminate();
}

void Window::setFramebuffer(std::shared_ptr<Framebuffer> fb){
    _framebuffer = fb;
}

void Window::processInput()
{

    if(!_window)return;

    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(_window, true);
}

void Window::window_size_callback(GLFWwindow* window, int width, int height){
    
    if(_dispatcher.get()){
        //spdlog::info("window_size_callback");
        WindowSizeEvent e(width,height,EventType::WindowSize, false);
        _dispatcher->dispatch(e);
    }

    
}
void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos){
    
    if(_dispatcher.get()){
        MousePosEvent e(xpos,ypos,EventType::MousePos, false);
        _dispatcher->dispatch(e);
    }
    //spdlog::info("mouse_callback");

}
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    //spdlog::info("scroll_callback");
    if(_dispatcher.get()){
        MouseScrollEvent e(xoffset,yoffset,EventType::MouseScroll,false);
        _dispatcher->dispatch(e);
    }
    
}

EventDispatcher* Window::getEventDispatcher(){
    return _dispatcher.get();
}

}