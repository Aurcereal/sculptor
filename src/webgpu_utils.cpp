#include "webgpu_utils.h"

#pragma warning(disable:4100)
void wgpuPollEvents(Device device, bool yieldToWebBrowser) {
    #if defined(WEBGPU_BACKEND_DAWN)
        device.tick();
    #elif defined(WEBGPU_BACKEND_WGPU)
        wgpuDevicePoll(device, false, nullptr);
    #elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
        if (yieldToWebBrowser) {
            emscripten_sleep(100);
        }
    #endif
}

uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
    uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
    return step * divide_and_ceil;
}