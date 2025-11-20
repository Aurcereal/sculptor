
#include "application.h"
#include <iostream>
#include <vector>
#include <assert.h>
#include "webgpu_utils.h"
#include "resource_manager.h"
#include "shader_parameter.h"

using namespace glm;
using namespace wgpu;
using namespace std;
namespace SP = ShaderParameter;

// Almost identical to inspect adapter; I think the device is like what we use/our interface and the adapter is reality
void inspectDevice(WGPUDevice device) {
    std::vector<WGPUFeatureName> features;
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    features.resize(featureCount);
    wgpuDeviceEnumerateFeatures(device, features.data());

    std::cout << "Device features:" << std::endl;
    std::cout << std::hex;
    for (auto f : features) {
        std::cout << " - 0x" << f << std::endl;
    }
    std::cout << std::dec;

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
    bool success = wgpuDeviceGetLimits(device, &limits);
#endif

    if (success) {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
        // [...] Extra device limits
    }
}

void inspectAdapter(WGPUAdapter adapter) {
    // Limits
#ifndef __EMSCRIPTEN__
    WGPUSupportedLimits supportedLimits = {};
    supportedLimits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN // Dawn has a WGPUStatus return
    bool success = wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;
#else
    bool success = wgpuAdapterGetLimits(adapter, &supportedLimits);
#endif

    if (success) {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << supportedLimits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << supportedLimits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << supportedLimits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << supportedLimits.limits.maxTextureArrayLayers << std::endl;
    }

#endif // NOT __EMSCRIPTEN__

    // Features
    std::vector<WGPUFeatureName> features;
    size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);
    features.resize(featureCount);
    wgpuAdapterEnumerateFeatures(adapter, features.data());

    std::cout << "Adapter features:" << std::endl;
    std::cout << std::hex; // Enable hex mode
    for (WGPUFeatureName &f : features) {
        std::cout << " - 0x" << f << std::endl;
    }
    std::cout << std::dec; // Return to decimal mode
}

void commandBufferTest(WGPUQueue &queue, WGPUDevice &device) {
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "Command encoder/creator";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // Create a command that does a nothing task
    wgpuCommandEncoderInsertDebugMarker(encoder, "task 1");
    wgpuCommandEncoderInsertDebugMarker(encoder, "task 2");

    // Generate Command Buffer
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    // With the command buffer, we can submit command queue
    std::cout << "Submitting command..." << std::endl;
    wgpuQueueSubmit(queue, 1, &cmd);
    wgpuCommandBufferRelease(cmd);
    std::cout << "Command submitted" << std::endl;

    for (int i = 0 ; i < 5 ; ++i) {
        std::cout << "Tick/Poll device..." << std::endl;
    #if defined(WEBGPU_BACKEND_DAWN)
        wgpuDeviceTick(device);
    #elif defined(WEBGPU_BACKEND_WGPU)
        wgpuDevicePoll(device, false, nullptr);
    #elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
        emscripten_sleep(100);
    #endif
    }
}

WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const* options) {
    // A simple structure holding the local information shared with the
    // onAdapterRequestEnded callback.
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    // Callback called by wgpuInstanceRequestAdapter when the request returns
    // This is a C++ lambda function, but could be any function defined in the
    // global scope. It must be non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function pointer, which is what
    // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
    // is to convey what we want to capture through the pUserData pointer,
    // provided as the last argument of wgpuInstanceRequestAdapter and received
    // by the callback as its last argument.
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        }
        else {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
        };

    // Call to the WebGPU request adapter procedure
    wgpuInstanceRequestAdapter(
        instance /* equivalent of navigator.gpu */,
        options,
        onAdapterRequestEnded,
        (void*)&userData
    );

    // We wait until userData.requestEnded gets true, will only need delay in Emscripten
#ifdef __EMSCRIPTEN
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif

    assert(userData.requestEnded);
    std::cout << "Adapter request ended" << std::endl;

    return userData.adapter;
}

/**
 * Utility function to get a WebGPU device, so that
 *     WGPUDevice device = requestDeviceSync(adapter, options);
 * is roughly equivalent to
 *     const device = await adapter.requestDevice(descriptor);
 * It is very similar to requestAdapter
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor) {
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        }
        else {
            std::cout << "Could not get WebGPU device: " << message << std::endl;
        }
        userData.requestEnded = true;
        };

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        onDeviceRequestEnded,
        (void*)&userData
    );

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.device;
}

bool Application::Initialize() {
    // Get instance
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

    // For immediate error callback
#ifdef WEBGPU_BACKEND_DAWN
    // Make sure the uncaptured error callback is called as soon as an error
    // occurs rather than at the next call to "wgpuDeviceTick".
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.next = nullptr;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;
    toggles.enabledToggleCount = 1;
    const char* toggleName = "enable_immediate_error_handling";
    toggles.enabledToggles = &toggleName;

    desc.nextInChain = &toggles.chain;
#endif // WEBGPU_BACKEND_DAWN

    // For some reason, Emscripten doesn't want a descriptor
#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    WGPUInstance instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstance instance = wgpuCreateInstance(&desc);
#endif
    if (instance == nullptr) {
        std::cout << "Could not initialize WebGPU!" << std::endl;
        return false;
    }
    std::cout << "WGPU instance: " << instance << std::endl;

    // Create GLFW Window
    if(!glfwInit()) {
        std::cerr << "Couldn't initialize GLFW!!!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW not to think about API since it wouldn't know WebGPU anyways
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disallow resizing since it causes crash for now
    window = glfwCreateWindow(640, 480, "Learn WebGPU!!", nullptr, nullptr);
    if(!window) {
        std::cerr << "Couldn't open GLFW window!!" << std::endl;
        glfwTerminate();
        return false;
    }

    // Create surface
    surface = glfwGetWGPUSurface(instance, window);

    // Get adapter
    std::cout << "Requesting Device Adapter to determine capabilities..." << std::endl;
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = surface;
    WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
    wgpuInstanceRelease(instance); // Don't need instance now that we have adapter
    // inspectAdapter(adapter);

    // Get device
    vector<WGPUFeatureName> requiredFeatures = {
        FeatureName::Float32Filterable
    };
    std::cout << "Requesting device..." << std::endl;
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = "My Device"; // anything works here, that's your call
    deviceDesc.requiredFeatureCount = requiredFeatures.size();
    deviceDesc.requiredFeatures = requiredFeatures.data();
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";
    // A function that is invoked whenever the device stops being available.
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
        std::cout << "Device lost: reason " << reason;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
        };
    RequiredLimits limits = GetRequiredLimits(adapter);
    deviceDesc.requiredLimits = nullptr; //&limits; // Require at least these capabilities
    device = requestDeviceSync(adapter, &deviceDesc);
    std::cout << "Got device: " << device << std::endl;
    auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
        // Putting breakpoints in this error callback is useful..
        // But in Dawn, the callback will be called when the device 'ticks', less informative
        // So we can use an instance toggle above to force it to immediately call this callback on error
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
        };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);
    // inspectDevice(device);

    // Queue and Command Buffer
    queue = wgpuDeviceGetQueue(device);
    auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void *) {
        std::cout << "Queued work finished with status: " << status << std::endl;
        };
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData */);

    // Configure surface
    WGPUSurfaceConfiguration config = {};
    config.nextInChain = nullptr;

    config.width = 640;
    config.height = 480;
    surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
    config.format = surfaceFormat; // RGBA and channel size chosen by adapter
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.usage = WGPUTextureUsage_RenderAttachment; // Has to know what it'll be used for
    config.device = device;
    config.presentMode = WGPUPresentMode_Fifo; // How do we do the swap chain?  We have a regular queue here instead of just 2 that we swap
    config.alphaMode = WGPUCompositeAlphaMode_Auto; // How are textures composited onto OS window, could be used for transparent windows

    wgpuSurfaceConfigure(surface, &config);

    wgpuAdapterRelease(adapter);

    // Creation
    InitializeBuffers();
    depthTextureHolder.Initialize(device);

    testTexture.Initialize(device, uvec3(256), TextureFormat::R32Float, true, true);
    testInputTexture.Initialize(device, uvec3(256), TextureFormat::R32Float, true, true);

    // Running
    vector<SP::Parameter> testShaderParams = {
      SP::Parameter(SP::UUniform{&uniformBuffer}),
      SP::Parameter(SP::UTexture{&testTexture, true, false}),
      SP::Parameter(SP::USampler{&testTexture})
    };
    testShader.Initialize(device, testShaderParams, surfaceFormat, depthTextureHolder, "/test_shader.wgsl");

    vector<SP::Parameter> computeShaderParams = {
      SP::Parameter(SP::UTexture{&testInputTexture, true, false}),
      SP::Parameter(SP::USampler{&testInputTexture}),
      SP::Parameter(SP::UTexture{&testTexture, true, true})
    };

    testComputeShader.Initialize(device, computeShaderParams, "/test_compute.wgsl");
    bool finished = false;
    testComputeShader.Dispatch(device, queue, uvec3(256), &finished);
    while(!finished) {
        wgpuPollEvents(device, true);
    }

    testComputeMeshGenerate();

    return true;
}

void Application::Terminate() {
    testTexture.Destroy();
    testInputTexture.Destroy();
    testComputeShader.Destroy();

    testShader.Destroy();

    vertexBuffer.buffer.release();
    indexBuffer.buffer.release();
    uniformBuffer.buffer.release();

    wgpuSurfaceUnconfigure(surface);
    wgpuSurfaceRelease(surface);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
}

void Application::MainLoop() {
    glfwPollEvents(); // Process input events

    // Update uniforms
    float t = static_cast<float>(glfwGetTime());
    queue.writeBuffer(uniformBuffer.buffer, offsetof(MyUniforms, time), &t, sizeof(float));
    mat4x4 viewMat = camera.GetViewMatrix();
    mat4x4 modelMat = glm::rotate(mat4(1.0f), t, vec3(0.0f, 1.0f, 0.0f));
    queue.writeBuffer(uniformBuffer.buffer, offsetof(MyUniforms, modelMatrix), &modelMat, sizeof(mat4x4));
    queue.writeBuffer(uniformBuffer.buffer, offsetof(MyUniforms, viewMatrix), &viewMat, sizeof(mat4x4));

    auto [surfaceTexture, targetView] = GetNextSurfaceViewData();
    if(!targetView) return;

    // Create command encoder to create draw call command
    CommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = "Command encoder";
    CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

    // Create render pass color attachment
    RenderPassDescriptor renderPassDesc = {};
    RenderPassColorAttachment renderPassColorAttachment = {}; // Only 1 color attach for now
    renderPassColorAttachment.view = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear; // What to do before executing render pass
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store; // What to do after executing render pass
    renderPassColorAttachment.clearValue = WGPUColor{0.1, 1.0, 0.5, 1.0 };
    #ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED; // No depth buffer
    #endif

    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment; // Array

    // Create render pass depth attachment
    RenderPassDepthStencilAttachment depthStencilAttachment;
    // The view of the depth texture
    depthStencilAttachment.view = depthTextureHolder.depthTextureView;

    // 1 is far
    depthStencilAttachment.depthClearValue = 1.0f;
    #ifdef WEBGPU_BACKEND_DAWN
    depthStencilAttachment.clearDepth = std::numeric_limits<float>::quiet_NaN();
    #endif
    // Operation settings comparable to the color attachment
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    // we could turn off writing to the depth buffer globally here
    depthStencilAttachment.depthReadOnly = false;

    // Stencil setup, mandatory but unused
    depthStencilAttachment.stencilClearValue = 0;
    #ifndef WEBGPU_BACKEND_DAWN
    depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
    #else
    depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
    #endif
    depthStencilAttachment.stencilReadOnly = true;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

    RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

    // What to draw here
    renderPass.setPipeline(testShader.pipeline);
    renderPass.setVertexBuffer(0, vertexBuffer.buffer, 0, vertexBuffer.size); // Could change geo here
    renderPass.setIndexBuffer(indexBuffer.buffer, IndexFormat::Uint32, 0, indexBuffer.size);
    renderPass.setBindGroup(0, testShader.bindGroup, 0, nullptr);
    renderPass.drawIndexed(indexCount, 1, 0, 0, 0);

    renderPass.end();
    renderPass.release();

    // Create draw command
    CommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Command buffer";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    // Queue draw command
    queue.submit(1, &command);
    command.release();

    // At end of frame
    targetView.release();
    #ifndef __EMSCRIPTEN__ // We use a different way with Emscripten
    surface.present();
    #endif
}

bool Application::IsRunning() {
    return !glfwWindowShouldClose(window);
}

std::pair<SurfaceTexture, TextureView> Application::GetNextSurfaceViewData() {
    // Get next surface texture
    // Surface texture contains actual texture as well as additional info (status)
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
    if(surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        return {surfaceTexture, nullptr};
    }

    // Create surface texture view
    // Texture view could represent a sub-part of the texture or it in a different format..
    // For now we're using the given boiler plate
    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    // Release texture
    #ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view which contains its own reference to texture
    // with wgpu-native, surface textures must be release after the call to wgpuSurfacePresent
    wgpuTextureRelease(surfaceTexture.texture);
    #endif // WEBGPU_BACKEND_WGPU

    return {surfaceTexture, targetView};
}

RequiredLimits Application::GetRequiredLimits(Adapter adapter) const {
    SupportedLimits supportedLimits;
    adapter.getLimits(&supportedLimits);

    RequiredLimits requiredLimits = Default;
    requiredLimits.limits.minStorageBufferOffsetAlignment = 32;
    requiredLimits.limits.maxVertexAttributes = 2; // Require a max of 1
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxBufferSize = 6 * 6 * sizeof(float);
    requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
    requiredLimits.limits.maxInterStageShaderComponents = 3;
    // We can only draw 2 triangles with these limits..

    return requiredLimits;
}

void Application::InitializeBuffers() {
    const uint tempTriCount = 64000;

    // indices must be uint16_t or uint32_t
    indexCount = tempTriCount*3;//static_cast<uint32_t>(indices.size());

    // Vertex Buffer
    vertexBuffer = createBuffer(device, sizeof(float)*6*3* tempTriCount,//positions.size() * sizeof(float), 
        BufferUsage::CopyDst | BufferUsage::Vertex | BufferUsage::Storage,
        false);

    // Index Buffer
    indexBuffer = createBuffer(device, sizeof(uint32_t)*3* tempTriCount,//indices.size()*sizeof(uint32_t),
        BufferUsage::CopyDst | BufferUsage::Index | BufferUsage::Storage,
        false);
    
    // Atomic Count Buffer
    vector<uint32_t> counts = {0,0};
    countBuffer = createBuffer(
        device, 2 * sizeof(uint32_t), 
        BufferUsage::CopyDst | BufferUsage::Storage, false);
    queue.writeBuffer(countBuffer.buffer, 0, counts.data(), countBuffer.size);

    // Uniform Buffer
    uniformBuffer = createBuffer(device, sizeof(MyUniforms), BufferUsage::CopyDst | BufferUsage::Uniform, false);
    MyUniforms uniforms;
    uniforms.time = 1.0f;
    uniforms.color = vec4(1.0,0.0,0.0,1.0);
    uniforms.modelMatrix = mat4(1.0f);
    uniforms.projectionMatrix = camera.GetProjectionMatrix();
    uniforms.viewMatrix = camera.GetViewMatrix();
    queue.writeBuffer(uniformBuffer.buffer, 0, &uniforms, sizeof(MyUniforms));

    // !!! Buffers must be multiple of 4 bytes
    // So make sure your data AND buffer is a good size
    // Could be arbitrary data just avoid trash data
}

void Application::testComputeMeshGenerate() {
    // Temp placement oc, needs textures to be generated
    ComputeShader meshGenerateShader;
    
    vector<SP::Parameter> params = {
        SP::Parameter(SP::UBuffer{&vertexBuffer, true}),
        SP::Parameter(SP::UBuffer{&indexBuffer, true}),
        SP::Parameter(SP::UBuffer{&countBuffer, true}),
        SP::Parameter(SP::UTexture{&testTexture, true, false}),
        SP::Parameter(SP::USampler{&testTexture})
    };
    meshGenerateShader.Initialize(device, params, "/mesh_generate.wgsl");
    meshGenerateShader.Dispatch(device, queue, uvec3(16), nullptr);
    meshGenerateShader.Destroy();
}