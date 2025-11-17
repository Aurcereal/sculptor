#pragma once
#include <webgpu/webgpu.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>

using namespace wgpu;
using namespace glm;

void wgpuPollEvents(Device device, bool yieldToWebBrowser);

uint32_t ceilToNextMultiple(uint32_t value, uint32_t step);

struct BufferHolder {
    //inline BufferHolder() {}
    Buffer buffer = nullptr;
    size_t size;
};

BufferHolder createBuffer(Device &device, size_t size, WGPUBufferUsageFlags usage, bool mappedAtCreation);