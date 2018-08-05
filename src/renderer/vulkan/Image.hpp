#pragma once

#include "renderer/vulkan/Base.hpp"
#include "renderer/vulkan/Device.hpp"

/*
 *  Vulkan textures and image views
 */

namespace vk
{
    struct Texture
    {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView   imageView  = VK_NULL_HANDLE;
        VkSampler sampler   = VK_NULL_HANDLE;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
        VkFormat  format    = VK_FORMAT_R8G8B8A8_UNORM;
        VkFilter  minFilter = VK_FILTER_LINEAR;
        VkFilter  magFilter = VK_FILTER_LINEAR;
        // mipmap settings
        uint32_t mipLevels = 1;
        float mipLodBias = 0.f;
        float mipMinLod  = 0.f;
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkFilter mipmapFilter = VK_FILTER_LINEAR;
    };


    void createTextureImage(const Device &device, const VkCommandPool &commandPool, const VkCommandPool &commandPool2, Texture *dstTex, const unsigned char *data, uint32_t width, uint32_t height);
    void createTexture(const Device &device, const VkCommandPool &commandPool, const VkCommandPool &commandPool2, Texture *dstTex, const unsigned char *data, uint32_t width, uint32_t height);
    void releaseTexture(const Device &device, Texture &texture);
    VkResult createImageView(const Device &device, const VkImage &image, VkImageAspectFlags aspectFlags, VkImageView *imageView, VkFormat format, uint32_t mipLevels);
    VkResult createTextureSampler(const Device &device, Texture *texture);
    Texture  createColorBuffer(const Device &device, const SwapChain &swapChain, const VkCommandPool &commandPool, VkSampleCountFlagBits sampleCount);
    Texture  createDepthBuffer(const Device &device, const SwapChain &swapChain, const VkCommandPool &commandPool, VkSampleCountFlagBits sampleCount);
}
