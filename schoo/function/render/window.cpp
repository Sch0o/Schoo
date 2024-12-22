#include <experimental/vector>
#include"schoo/function/render/window.hpp"
#include"schoo/engine.hpp"


namespace schoo {
    std::unique_ptr<Window> Window::instance_ = nullptr;
    bool Window::firstMouse = true;
    float Window::lastX = 0;
    float Window::lastY = 0;

    void Window::Init(uint32_t width, uint32_t height, const char *title) {
        instance_.reset(new Window(width, height, title));
    }
    void Window::Quit() {
        instance_.reset();
    }

    Window::Window(uint32_t w, uint32_t h, const char *title) {
        width = w;
        height = h;

        if (!glfwInit()) {
            throw std::runtime_error("Failed to create glfw window!");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        //存储指针
        glfwSetWindowUserPointer(glfwWindow, this);

        //处理窗口大小改变 TODO

        //callback
        glfwSetCursorPosCallback(glfwWindow, mouse_move_callback);
        glfwSetKeyCallback(glfwWindow, key_press_callback);

        glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(glfwWindow,600,400);
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
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }

    void Window::mouse_move_callback(GLFWwindow *window, double xPos, double yPos) {
        //std::cout<<"xPos: "<<xPos<<"yPos: "<<yPos<<std::endl;
        auto camera = SchooEngine::GetInstance().camera;
        float deltaX, deltaY;
        if (firstMouse) {
            firstMouse = false;
            deltaY = 0;
            deltaX = 0;
        } else {
            deltaX = xPos - lastX;
            deltaY = lastY - yPos;
        }
        lastX = xPos;
        lastY = yPos;

        camera->ProcessMouseMovement(deltaX, deltaY);
    }

    void Window::key_press_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
        auto camera = SchooEngine::GetInstance().camera;
        float deltaTime = SchooEngine::GetInstance().deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->MoveForward(deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->MoveForward(-deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->MoveRight(deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->MoveRight(-deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            camera->MoveUp(-deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera->MoveUp(deltaTime);
        }

    }
}