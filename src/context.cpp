#include"schoo/context.hpp"


namespace schoo {
    std::unique_ptr<Context>Context::instance_ = nullptr;

    void Context::Init(schoo::Window &window) {
        instance_.reset(new Context(window));
    }

    void Context::Quit() {
        instance_.reset();
    }

    Context &Context::GetInstance() {
        return *instance_;
    }

    Context::Context(schoo::Window &window) {
        //创建实例
        createInstance(window);
        //创建物理设备
        choosePhysicalDevice();
        //创建surface
        createSurface(window);
        //查询用于渲染的队列族
        queryQueueFamilyIndices();
        //创建逻辑设备
        createDevice();
        //得到渲染队列
        getQueues();
        //渲染管线
        InitRenderProcess();
    }

    Context::~Context() {
        device.destroy();
        instance.destroySurfaceKHR(surface);
        instance.destroy();
    }

    void Context::createInstance(schoo::Window &window) {
        vk::InstanceCreateInfo createInfo;

        std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

        vk::ApplicationInfo appInfo;
        appInfo.setApiVersion(VK_API_VERSION_1_3);

        auto extensions = window.getRequiredExtensions();

        createInfo.setPApplicationInfo(&appInfo)
                .setPEnabledLayerNames(layers)
                .setPEnabledExtensionNames(extensions);

        instance = vk::createInstance(createInfo);
    }

    void Context::choosePhysicalDevice() {

        auto physicalDevices = instance.enumeratePhysicalDevices();
        physicalDevice = physicalDevices[0];

        //std::cout << physicalDevice.getProperties().deviceName << std::endl;
    }

    void Context::createDevice() {
        std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        vk::DeviceCreateInfo deviceCreateInfo;
        vk::DeviceQueueCreateInfo queueCreateInfo;

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float queuePriorities = 1.0f;
        queueCreateInfo.setPQueuePriorities(&queuePriorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(queueFamilyIndices.graphicQueue.value());
        queueCreateInfos.push_back(queueCreateInfo);
        if (queueFamilyIndices.graphicQueue.value() != queueFamilyIndices.presentQueue.value()) {
            vk::DeviceQueueCreateInfo presentQueueCreateInfo;
            presentQueueCreateInfo.setPQueuePriorities(&queuePriorities)
                    .setQueueCount(1).setQueueFamilyIndex(queueFamilyIndices.presentQueue.value());
            queueCreateInfos.push_back(presentQueueCreateInfo);
        }

        deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
        deviceCreateInfo.setPEnabledExtensionNames(extensions);
        device = physicalDevice.createDevice(deviceCreateInfo);
    }

    void Context::queryQueueFamilyIndices() {
        auto properties = physicalDevice.getQueueFamilyProperties();
        //找到所需的队列族
        for (uint32_t i = 0; i < properties.size(); i++) {
            if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                queueFamilyIndices.graphicQueue = i;
            }
            if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
                queueFamilyIndices.presentQueue = i;
            }
            if (queueFamilyIndices.isCompeleted()) {
                break;
            }
        }
    }

    void Context::getQueues() {
        graphicsQueue = device.getQueue(queueFamilyIndices.graphicQueue.value(), 0);
        presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
    }

    void Context::createSurface(Window &window) {
        if (glfwCreateWindowSurface(instance, window.glfwWindow, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface)) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }

    }

    void Context::InitSwapchain(uint32_t w, uint32_t h) {
        swapchain.reset(new Swapchain(w, h));
    }

    void Context::QuitSwapchain() {
        swapchain.reset();
    }

    void Context::DestroyRenderProcess() {
        renderProcess.reset();
    }

    void Context::InitRenderer() {
        renderer.reset(new Renderer());
    }

    void Context::DestroyRenderer() {
        renderer.reset();
    }

    void Context::InitCommandManager() {
        commandManager.reset(new Command());
    }

    void Context::DestroyCommandManager() {
        commandManager.reset();
    }

    void Context::InitRenderProcess() {
        renderProcess.reset(new RenderProcess());
    }
}