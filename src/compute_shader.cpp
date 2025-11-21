#include "compute_shader.h"
#include "resource_manager.h"

void ComputeShader::InitBindGroupLayout(Device &device, const vector<ShaderParameter::Parameter> &shaderParams) {
    vector<BindGroupLayoutEntry> bindings(shaderParams.size(), Default);

    for(int i=0; i<bindings.size(); i++) {
        bindings[i].binding = i;
        bindings[i].visibility = ShaderStage::Compute; // UNIQUE TO COMPUTE
        switch(shaderParams[i].type) {
            case ShaderParameter::Type::TEXTURE:
                ShaderParameter::UTexture texParam = shaderParams[i].texture;
                if(texParam.shaderWriteEnabled) {
                    bindings[i].storageTexture.access = StorageTextureAccess::WriteOnly;
                    bindings[i].storageTexture.format = texParam.textureHolder->format;
                    bindings[i].storageTexture.viewDimension = texParam.is3D ? TextureViewDimension::_3D : TextureViewDimension::_2D;
                } else {
                    bindings[i].texture.sampleType = TextureSampleType::Float;
                    bindings[i].texture.viewDimension = texParam.is3D ? TextureViewDimension::_3D : TextureViewDimension::_2D;
                    bindings[i].texture.multisampled = false;
                }
                break;
            case ShaderParameter::Type::SAMPLER:
                bindings[i].sampler.type = SamplerBindingType::Filtering;
                break;
            case ShaderParameter::Type::UNIFORM:
                ShaderParameter::UUniform uniformParam = shaderParams[i].uniform;
                bindings[i].buffer.type = BufferBindingType::Uniform;
                bindings[i].buffer.minBindingSize = uniformParam.uniformBufferHolder->size;
                break;
            case ShaderParameter::Type::BUFFER:
                ShaderParameter::UBuffer bufferParam = shaderParams[i].buffer;
                bindings[i].buffer.type = 
                    bufferParam.forWriting ? BufferBindingType::Storage : BufferBindingType::ReadOnlyStorage;
                break;
        }
    }

    BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = (uint32_t) bindings.size();
    bindGroupLayoutDesc.entries = bindings.data();
    bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
}
void ComputeShader::InitBindGroups(Device &device, const vector<ShaderParameter::Parameter> &shaderParams) {//, TextureHolder &inputTexture, TextureHolder &outputTexture) {
    vector<BindGroupEntry> entries(shaderParams.size(), Default);

    // can be generalized for regular shader too
    for(int i=0; i<entries.size(); i++) {
        entries[i].binding = i;
        switch(shaderParams[i].type) {
            case ShaderParameter::Type::TEXTURE:
                ShaderParameter::UTexture texParam = shaderParams[i].texture;
                entries[i].textureView = texParam.textureHolder->textureView;
                break;
            case ShaderParameter::Type::SAMPLER:
                ShaderParameter::USampler samplerParam = shaderParams[i].sampler;
                entries[i].sampler = samplerParam.textureHolder->sampler;
                break;
            case ShaderParameter::Type::UNIFORM:
                ShaderParameter::UUniform uniformParam = shaderParams[i].uniform;
                entries[i].buffer = uniformParam.uniformBufferHolder->buffer;
                entries[i].offset = 0;
                entries[i].size = uniformParam.uniformBufferHolder->size;
                break;
            case ShaderParameter::Type::BUFFER:
                ShaderParameter::UBuffer bufferParam = shaderParams[i].buffer;
                entries[i].buffer = bufferParam.bufferHolder->buffer;
                entries[i].offset = 0;
                entries[i].size = bufferParam.bufferHolder->size;
                break;
        }
    }

    BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = entries.size();
    bindGroupDesc.entries = entries.data();
    bindGroup = device.createBindGroup(bindGroupDesc);
}

void ComputeShader::Initialize(Device &device, const vector<ShaderParameter::Parameter> &shaderParams, std::string path) {//TextureHolder &inputTexture, TextureHolder &outputTexture) {
    #ifdef RESOURCE_DIR
    ShaderModule computeShaderModule = ResourceManager::LoadShaderModule(RESOURCE_DIR + path, device);
    #else
    ShaderModule computeShaderModule = nullptr;
    std::cerr << "RESOURCE_DIR Undefined!" << std::endl;
    #endif

    InitBindGroupLayout(device, shaderParams);
    InitBindGroups(device, shaderParams);

    PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &bindGroupLayout;
    pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);

    ComputePipelineDescriptor computePipelineDesc = Default;
    computePipelineDesc.compute.entryPoint = "main";
    computePipelineDesc.compute.module = computeShaderModule;
    computePipelineDesc.layout = pipelineLayout;
    computePipeline = device.createComputePipeline(computePipelineDesc);
}

void ComputeShader::Dispatch(Device &device, Queue &queue, uvec3 jobSize, bool *finished) {
    auto encoder = device.createCommandEncoder(Default);

    ComputePassDescriptor computePassDesc;
    computePassDesc.timestampWrites = nullptr;
    ComputePassEncoder computePass = encoder.beginComputePass(computePassDesc);

    computePass.setPipeline(computePipeline);
    computePass.setBindGroup(0, bindGroup, 0, nullptr);

    const uvec3 workGroupSize = uvec3(4); // TODO: can it read shader or cmake or smth
    uvec3 workGroupCount = (jobSize + workGroupSize - uvec3(1)) / workGroupSize;

    computePass.dispatchWorkgroups(workGroupCount.x, workGroupCount.y, workGroupCount.z);

    computePass.end();

    CommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Compute shader command";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    // Queue draw command
    queue.submit(1, &command);

    if(finished != nullptr) {
        *finished = false;
        auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus , void *userData) {
            bool *isFinished = (bool*) userData;
            *isFinished = true;
            };
        wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, finished);
    }

    command.release();

    #ifndef WEBGPU_BACKEND_WGPU
    wgpuComputePassEncoderRelease(computePass);
    #endif
}

void ComputeShader::DispatchSync(Device &device, Queue &queue, uvec3 jobSize) {
    bool finished = false;
    Dispatch(device, queue, jobSize, &finished);
    while(!finished) {
        wgpuPollEvents(device, true);
    }
}

void ComputeShader::Destroy() {
    bindGroupLayout.release();
    pipelineLayout.release();
}