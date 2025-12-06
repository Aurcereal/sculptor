#include "buffer_helper.h"
#include "webgpu_utils.h"

BufferHolder createBuffer(Device &device, size_t size, WGPUBufferUsageFlags usage, bool mappedAtCreation) {
    BufferDescriptor bufferDesc;
    bufferDesc.size = size;
    bufferDesc.usage = usage;
    bufferDesc.mappedAtCreation = mappedAtCreation;
    return {device.createBuffer(bufferDesc), size};
}

void BufferHelper::Initialize(Device &device) {
    BufferHelper& helper = GetInstance();

    helper.copyBackBuffer = createBuffer(device, 4*256, 
        BufferUsage::CopyDst | BufferUsage::MapRead, false);
}

const void* BufferHelper::MapBufferToCPU(Device& device, Queue& queue, BufferHolder& src) {
    auto& ins = GetInstance();
    assert(!ins.mapped);
    assert(src.size <= ins.copyBackBuffer.size); 

    auto encoder = device.createCommandEncoder(Default);
    encoder.copyBufferToBuffer(src.buffer, 0, ins.copyBackBuffer.buffer, 0, src.size);
    CommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Buffer reading command in buffer_helper";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    queue.submit(1, &command);

    bool finished = false;
    const void *data;
    auto callbackHandle = ins.copyBackBuffer.buffer.mapAsync(MapMode::Read, 0, src.size, 
        [&](BufferMapAsyncStatus status) {
            if(status == BufferMapAsyncStatus::Success) {
                data = ins.copyBackBuffer.buffer.getConstMappedRange(0, src.size);
                ins.mapped = true;
            }
            finished = true;
    });

    while(!finished) {
        wgpuPollEvents(device, true);
    }

    return data;
}

void BufferHelper::Unmap() {
    auto& ins = GetInstance();
    ins.copyBackBuffer.buffer.unmap();
    ins.mapped = false;
}
