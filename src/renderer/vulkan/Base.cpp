#include "renderer/vulkan/Base.hpp"
#include "renderer/vulkan/Validation.hpp"
#include "Utils.hpp"
#include <SDL_vulkan.h>
#include <vector>

#define vkEnumerateInstanceVersion(instance, instanceVersion) callVkF2(vkEnumerateInstanceVersion, instance, instanceVersion)

namespace vk
{
    static bool instanceExtensionSupported(const char *extension)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        LOG_MESSAGE_ASSERT(extensionCount > 0, "No instance extensions available?");

        std::vector<VkExtensionProperties> instanceExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensions.data());

        for (auto &e : instanceExtensions)
        {
            if (!strcmp(e.extensionName, extension))
                return true;
        }

        return false;
    }

    VkResult createInstance(SDL_Window *window, VkInstance *instance, const char *title)
    {
        uint32_t instanceVersion = VK_API_VERSION_1_0;

        // vkEnumerateInstanceVersion does not exist on (some?) Android devices
        if (vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion"))
            vkEnumerateInstanceVersion(NULL, &instanceVersion);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = title;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "custom";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = instanceVersion;

        unsigned int extCount = 0;
        // get count of required extensions
        SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);

        std::vector<const char*> enabledExtensions(extCount);
        // get names of required extensions
        SDL_Vulkan_GetInstanceExtensions(window, &extCount, enabledExtensions.data());

#ifdef VALIDATION_LAYERS_ON
        // add validation layer extension to the list
        bool hasDebugUtils = instanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        if (hasDebugUtils)
            enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        else
            enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();

#ifdef VALIDATION_LAYERS_ON
        if (!validationLayersAvailable(validationLayers, validationLayerCount))
        {
            LOG_MESSAGE_ASSERT(false, "Validation layers not available!");
            return VK_RESULT_MAX_ENUM;
        }

        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
#else
        createInfo.enabledLayerCount = 0;
#endif
        VkResult instanceCreated = vkCreateInstance(&createInfo, nullptr, instance);

#ifdef VALIDATION_LAYERS_ON
        VK_VERIFY(instanceCreated);
        vk::createValidationLayers(*instance, hasDebugUtils);
#endif
        return instanceCreated;
    }

    VkResult createDescriptorSet(const Device &device, Descriptor *descriptor)
    {
        VkDescriptorSetLayout layouts[] = { descriptor->setLayout };
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptor->pool;
        allocInfo.pSetLayouts = layouts;
        allocInfo.descriptorSetCount = 1;

        return vkAllocateDescriptorSets(device.logical, &allocInfo, &descriptor->set);
    }

    // create VMA allocator
    VkResult createAllocator(const Device &device, VmaAllocator *allocator)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = device.physical;
        allocatorInfo.device = device.logical;

        return vmaCreateAllocator(&allocatorInfo, allocator);
    }

    // destroy VMA allocator
    void destroyAllocator(VmaAllocator &allocator)
    {
        vmaDestroyAllocator(allocator);
    }

    VkFormat getBestDepthFormat(const Device &device)
    {
        VkFormat depthFormats[] = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };

        for (int i = 0; i < 5; ++i)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(device.physical, depthFormats[i], &formatProps);

            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return depthFormats[i];
        }

        LOG_MESSAGE_ASSERT(false, "No supported depth format found!");
        return VK_FORMAT_D16_UNORM;
    }

// deprecated Vulkan surface creation prior to SDL 2.0.6
/*
    VkResult createSurface(const void *window, const VkInstance &instance, VkSurfaceKHR *surface)
    {
        SDL_SysWMinfo wminfo;
        SDL_VERSION(&wminfo.version);
        if (!SDL_GetWindowWMInfo((SDL_Window *)window, &wminfo))
        {
            LOG_MESSAGE_ASSERT(false, "Could not get WM info from SDL!");
            return VK_RESULT_MAX_ENUM;
        }

        switch (wminfo.subsystem)
        {
        default:
            LOG_MESSAGE_ASSERT(false, "Unsupported window subsystem: " << wminfo.subsystem);
            return VK_RESULT_MAX_ENUM;
        case SDL_SYSWM_WINDOWS:
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hwnd = wminfo.info.win.window;
            surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

            return vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, surface);
        }
    }
*/
}
