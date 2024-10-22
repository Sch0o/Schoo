#pragma once

#define GLFW_INCLUDE_VULKAN
#include"GLFW/glfw3.h"
#include<iostream>
#include<vulkan/vulkan.hpp>

namespace schoo {
    class Window final {
    public:
        Window() = delete;

        Window &operator=(const Window &) = delete;

        ~Window();

        static void Init(uint32_t width, uint32_t height, const char *title);

        static void Quit();

        static Window&GetInstance();

        GLFWwindow*glfwWindow;

        bool ShouldClose();

        void PollEvents();

        void SwapBuffer();

        std::vector<const char*>getRequiredExtensions();

        uint32_t width;
        uint32_t height;


    private:
        static std::unique_ptr<Window> instance_;

        Window(uint32_t width, uint32_t height, const char *title);
    };
}