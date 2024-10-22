#include <experimental/vector>
#include"schoo/window.hpp"

namespace schoo {
    std::unique_ptr<Window> Window::instance_= nullptr;

    void Window::Init(uint32_t width, uint32_t height, const char *title) {
        instance_.reset(new Window(width, height, title));
    }

    void Window::Quit() {
        instance_.reset();
    }

    Window::Window(uint32_t w, uint32_t h, const char *title) {
        width=w;
        height=h;

        if (!glfwInit()) {
            throw std::runtime_error("Failed to create glfw window!");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        //存储指针
        glfwSetWindowUserPointer(glfwWindow, this);

        //处理窗口大小改变 TODO
    }
    Window::~Window() {
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
    }

    Window &Window::GetInstance() {
        return *instance_;
    }

    bool Window::ShouldClose() {
        return glfwWindowShouldClose(glfwWindow);
    }

    void Window::SwapBuffer() {
        glfwSwapBuffers(glfwWindow);
    }

    void Window::PollEvents() {
        glfwPollEvents();
    }


    std::vector<const char *> Window::getRequiredExtensions() {
        uint32_t glfwExtensionCount=0;
        const char**glfwExtensions;
        glfwExtensions= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*>extensions(glfwExtensions,glfwExtensions+glfwExtensionCount);

        return extensions;
    }
}