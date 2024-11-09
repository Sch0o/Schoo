#pragma once

#include "vulkan/vulkan.hpp"
#include<iostream>
#include <optional>
#include"schoo/window.hpp"
#include "schoo/swapchain.hpp"

#define GLFW_INCLUDE_VULKAN

#include"GLFW/glfw3.h"
#include "renderProcess.hpp"
#include "renderer.hpp"
#include "command.hpp"


namespace schoo {
    class Context final {
    public:
        static void Init(schoo::Window &window);

        static void Quit();

        static Context &GetInstance();

        ~Context();

        struct QueueFamilyIndices final {
            std::optional<uint32_t> graphicQueue;
            std::optional<uint32_t> presentQueue;

            bool isCompeleted() {
                return graphicQueue.has_value() && presentQueue.has_value();
            }
        };

        vk::Instance instance;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        QueueFamilyIndices queueFamilyIndices;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::SurfaceKHR surface;
        std::unique_ptr<Swapchain> swapchain;
        std::unique_ptr<RenderProcess> renderProcess;
        std::unique_ptr<Renderer> renderer;
        std::unique_ptr<Command> commandManager;


        void DestroyRenderProcess();

        void DestroyRenderer();

        void InitSwapchain(uint32_t w, uint32_t h);

        void QuitSwapchain();

        void InitRenderer();

        void InitCommandManager();

        void DestroyCommandManager();

        void InitRenderProcess();

    private:
        Context(schoo::Window &window);

        static std::unique_ptr<Context> instance_;


        void createInstance(schoo::Window &window);

        void choosePhysicalDevice();

        void createDevice();

        void queryQueueFamilyIndices();

        void getQueues();

        void createSurface(schoo::Window &window);



    };
}