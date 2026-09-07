#pragma once
#ifndef __XYVK_BUFFER
#define __XYVK_BUFFER
	#include "./xy2d_engine.hpp"

	namespace XY2D_NAMESPACE {
		enum class XY2D_BUFFERTYPE {
			VERTEX,
			UNIFORM,
			STAGING
		};
		
		class xy2d_buffer : public xy2d_disposable {
		public:
			xy2d_device& vkdevice;
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation memory = VK_NULL_HANDLE;
			VmaAllocationInfo description;
			VkDeviceSize size;
			XY2D_BUFFERTYPE bufferType;
			VkResult initialized = VK_SUCCESS;
			
			xy2d_buffer operator=(const xy2d_buffer&) = delete;
			xy2d_buffer(const xy2d_buffer&) = delete;
			~xy2d_buffer() { this->Dispose(); }
			
			xy2d_buffer(xy2d_device& vkdevice, XY2D_BUFFERTYPE bufferType, VkDeviceSize dataSize)
			: vkdevice(vkdevice), size(dataSize), bufferType(bufferType) {
				onDispose.hook(xy2d_callback<>([this]() {
					vmaDestroyBuffer(this->vkdevice.memoryAllocator, this->buffer, this->memory);
				}));
				
				initialized = Initialize();
			}
			
			VkResult CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags) {
				VkBufferCreateInfo bufCreateInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage };
				VmaAllocationCreateInfo allocCreateInfo { .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST, .flags = flags };
				
				return vmaCreateBuffer(vkdevice.memoryAllocator, &bufCreateInfo, &allocCreateInfo, &buffer, &memory, &description);
			}
			
			VkResult Initialize() {
				switch (bufferType) {
					case XY2D_BUFFERTYPE::VERTEX: return CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0x00000000);
					case XY2D_BUFFERTYPE::UNIFORM: return CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0x00000000);
					case XY2D_BUFFERTYPE::STAGING: return CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
				}
			}
		};
	}
#endif