#pragma once
#include "renderer/vulkan/Base.hpp"

/*
 * Vulkan validation layers
 */

namespace vk
{
#ifdef VALIDATION_LAYERS_ON
    // requested validation layers
    static const int validationLayerCount = 1;
    static const char *validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
#endif

    void createValidationLayers(const VkInstance &instance, bool useEXTDebugUtils);
    void destroyValidationLayers(const VkInstance &instance);
    bool validationLayersAvailable(const char **requested, size_t count);
}
