#include "compute_shader.h"
#include "resource_manager.h"

void ComputeShader::InitBindGroupLayout(Device &device) {
    // Temp, for now hardcoded but could later pasa a ref of struct
    // List detailing each binding/bind group
    vector<BindGroupLayoutEntry> bindings(3, Default);

    // Read texture
    bindings[0].binding = 0;
    bindings[0].visibility = ShaderStage::Compute;
    bindings[0].texture.sampleType = TextureSampleType::Float;
    bindings[0].texture.viewDimension = TextureViewDimension::_3D;

    // Read texture sampler
    bindings[1].binding = 1;
    bindings[1].visibility = ShaderStage::Compute;
    bindings[1].sampler.type = SamplerBindingType::Filtering;

    // Write texture
    bindings[2].binding = 2;
    bindings[2].visibility = ShaderStage::Compute;
    bindings[2].storageTexture.access = StorageTextureAccess::WriteOnly;
    bindings[2].storageTexture.format = TextureFormat::RGBA8Unorm;
    bindings[2].texture.viewDimension = TextureViewDimension::_3D;

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t) bindings.size();
    bindGroupLayoutDesc.entries = bindings.data();
    bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
}
void ComputeShader::InitBindGroups(Device &device, TextureHolder &inputTexture, TextureHolder &outputTexture) {
    vector<BindGroupEntry> entries(3, Default);

    entries[0].binding = 0;
    entries[0].textureView = inputTexture.textureView;

    entries[1].binding = 1;
    entries[1].sampler = inputTexture.sampler;
    
    entries[2].binding = 2;
    entries[2].textureView = outputTexture.textureView;

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = entries.size();
    bindGroupDesc.entries = entries.data();
    bindGroup = device.createBindGroup(bindGroupDesc);
}

void ComputeShader::Initialize(Device &device, TextureHolder &inputTexture, TextureHolder &outputTexture) {
    #ifdef RESOURCE_DIR
    ShaderModule computeShaderModule = ResourceManager::LoadShaderModule(RESOURCE_DIR "/test_compute.wgsl", device);
    #else
    ShaderModule computeShaderModule = nullptr;
    std::cerr << "RESOURCE_DIR Undefined!" << std::endl;
    #endif

    InitBindGroupLayout(device);
    InitBindGroups(device, inputTexture, outputTexture);

    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &bindGroupLayout;
    pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);

    ComputePipelineDescriptor computePipelineDesc = Default;
    computePipelineDesc.compute.entryPoint = "computeStuff";
    computePipelineDesc.compute.module = computeShaderModule;
    computePipelineDesc.layout = pipelineLayout;
    computePipeline = device.createComputePipeline(computePipelineDesc);
}

void ComputeShader::Dispatch(Queue &queue, CommandEncoder &encoder, uvec3 jobSize) {
    ComputePassDescriptor computePassDesc;
    computePassDesc.timestampWrites = nullptr;
    ComputePassEncoder computePass = encoder.beginComputePass(computePassDesc);

    computePass.setPipeline(computePipeline);
    computePass.setBindGroup(0, bindGroup, 0, nullptr);

    const uvec3 workGroupSize = uvec3(8); // TODO: can it read shader or cmake or smth
    uvec3 workGroupCount = (jobSize + workGroupSize - uvec3(1)) / workGroupSize;

    computePass.dispatchWorkgroups(workGroupCount.x, workGroupCount.y, workGroupCount.z);

    computePass.end();

    CommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Compute shader command";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    // Queue draw command
    queue.submit(1, &command);
    command.release();

    #ifndef WEBGPU_BACKEND_WGPU
    wgpuComputePassEncoderRelease(computePass);
    #endif
}

void ComputeShader::Destroy() {
    bindGroupLayout.release();
    pipelineLayout.release();
}