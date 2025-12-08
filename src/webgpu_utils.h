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

void waitForQueueCompletion(Device&, Queue&);