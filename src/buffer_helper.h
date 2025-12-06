#pragma once
#include <webgpu/webgpu.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>

using namespace wgpu;

struct BufferHolder {
    Buffer buffer = nullptr;
    size_t size; 
};

class BufferHelper {
public:

    static void Initialize(Device&);
    static const void* MapBufferToCPU(Device& device, Queue& queue, BufferHolder& src);
    static void Unmap();

    static BufferHelper& GetInstance() {
        static BufferHelper ins;
        return ins;
    }
    BufferHolder copyBackBuffer;

private:
    inline BufferHelper() {}

    bool mapped = false;
};

BufferHolder createBuffer(Device &device, size_t size, WGPUBufferUsageFlags usage, bool mappedAtCreation);

const void* mapBufferToCPU(BufferHolder&);