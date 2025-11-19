#include "shader.h"
#include "resource_manager.h"

using namespace wgpu;

void Shader::Initialize(Device& device, const vector<ShaderParameter::Parameter>& shaderParams, TextureFormat surfaceFormat, DepthTexture &depthTexture, std::string path) {
    RenderPipelineDescriptor pipelineDesc;

    #ifdef RESOURCE_DIR
    ShaderModule shaderModule = ResourceManager::LoadShaderModule(RESOURCE_DIR + path, device);
    #else
    ShaderModule shaderModule = nullptr;
    std::cerr << "Resource Directory Undefined by CMake!" << std::endl;
    #endif
    if(shaderModule == nullptr) {
        std::cerr << "Couldn't load shader!" << std::endl;
        exit(1);
    }

    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList; // Interpret as triangles
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = FrontFace::CCW; // What counts as front face?
    pipelineDesc.primitive.cullMode = CullMode::None; // Well ^ don't matter for now cuz we're not culling

    FragmentState fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    
    BlendState blendState;
    // [...] Configure Color Blending which takes form 'col = SourceFactor * src (VariableOperation) DstFactor * dst
    // Src is what we're drawing and Dst is what already exists so Src is on top of Dst
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;
    // [...] Configure Alpha Blending, we'll just make it always 1 for now
    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;

    ColorTargetState colorTarget;
    colorTarget.format = surfaceFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All; // Could write to a subset of color channels

    // Only 1 color attachment so only 1 target color
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDesc.fragment = &fragmentState; // Nullable since fragment state is optional
    pipelineDesc.depthStencil = &depthTexture.depthStencilState; // Configure ZBuffer test

    // You can have multiple fragments per pixel and avg the result into a pixel
    pipelineDesc.multisample.count = 1; // But we won't do multisampling
    pipelineDesc.multisample.mask = ~0u; // All bits on
    pipelineDesc.multisample.alphaToCoverageEnabled = false; // Irrelevant for now

    InitBindGroupLayout(device, shaderParams);
    PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*) &bindGroupLayout;
    pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);

    pipelineDesc.layout = pipelineLayout;

    InitBindGroups(device, shaderParams);

    vector<VertexAttribute> vertexAttribs;
    VertexBufferLayout vertexBufferLayout = InitVertexLayout(vertexAttribs);
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;
    
    pipeline = device.createRenderPipeline(pipelineDesc);

    shaderModule.release();
}

VertexBufferLayout Shader::InitVertexLayout(vector<VertexAttribute> &vertexAttribs) {
    // [...] Describe Vertex Layout
    VertexBufferLayout vertexBufferLayout;

    vertexAttribs.resize(2);

    vertexAttribs[0].shaderLocation = 0;
    vertexAttribs[0].format = VertexFormat::Float32x3;
    vertexAttribs[0].offset = 0;

    vertexAttribs[1].shaderLocation = 1;
    vertexAttribs[1].format = VertexFormat::Float32x3;
    vertexAttribs[1].offset = 3 * sizeof(float);
    
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = vertexAttribs.data();

    vertexBufferLayout.arrayStride = 6 * sizeof(float);
    vertexBufferLayout.stepMode = VertexStepMode::Vertex; // Each new val is a new vertex

    return vertexBufferLayout;
}

 void Shader::InitBindGroupLayout(Device &device, const vector<ShaderParameter::Parameter>& shaderParams) {

    vector<BindGroupLayoutEntry> bindings(shaderParams.size(), Default);

    for(int i=0; i<bindings.size(); i++) {
        bindings[i].binding = i;
        bindings[i].visibility = ShaderStage::Vertex | ShaderStage::Fragment; // TODO: make you able to choose what you want them to be visible in
        switch(shaderParams[i].type) {
            case ShaderParameter::Type::TEXTURE:
                ShaderParameter::UTexture texParam = shaderParams[i].texture;
                if(texParam.shaderWriteEnabled) {
                    bindings[i].storageTexture.access = StorageTextureAccess::WriteOnly;
                    bindings[i].storageTexture.format = TextureFormat::RGBA8Unorm; // TODO: make parameter
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

    BindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    bindGroupLayoutDesc.entryCount = bindings.size();
    bindGroupLayoutDesc.entries = bindings.data();
    bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
 }

  void Shader::InitBindGroups(Device &device, const vector<ShaderParameter::Parameter>& shaderParams) {
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

    BindGroupDescriptor bindGroupDesc = {};
    bindGroupDesc.layout = bindGroupLayout;
    bindGroupDesc.entryCount = static_cast<uint32_t>(entries.size()); // Same as layout
    bindGroupDesc.entries = entries.data();
    bindGroup = device.createBindGroup(bindGroupDesc);
 }

 void Shader::Destroy() {
    bindGroup.release();

    pipelineLayout.release();
    bindGroupLayout.release();

    pipeline.release();
 }