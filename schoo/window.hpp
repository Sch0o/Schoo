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

        static bool firstMouse;
        static float lastX;
        static float lastY;



    private:
        static std::unique_ptr<Window> instance_;

        Window(uint32_t width, uint32_t height, const char *title);

        static void mouse_move_callback(GLFWwindow *window, double xPos, double yPos);

        static void key_press_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

    };
}