#include "texture_holder.h"

TextureHolder::TextureHolder() {}

void TextureHolder::Initialize(Device &device, glm::uvec3 size, bool is3D, bool shaderWrite) {
    // size 256, 256, 1, dimension was 2d, format was rgba8unorm, try that and then go to like r32float
    textureDesc.dimension = is3D ? TextureDimension::_3D : TextureDimension::_2D;
    textureDesc.size = {size.x, size.y, size.y};
    this->size = size;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.format = TextureFormat::RGBA8Unorm; // 8 bits each col
    textureDesc.usage =
        TextureUsage::TextureBinding |  // Can sample in shader
        TextureUsage::CopyDst |          // Can go CPU -> GPU
        (shaderWrite ? TextureUsage::StorageBinding : 0); // Can write to tex in shader
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;

    texture = device.createTexture(textureDesc);

    TextureViewDescriptor textureViewDesc;

    textureViewDesc.aspect = TextureAspect::All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = is3D ? TextureViewDimension::_3D : TextureViewDimension::_2D;
    textureViewDesc.format = textureDesc.format;
    textureView = texture.createView(textureViewDesc); // COULD have multiple views of 1 tex (diff mip levels)

    SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = AddressMode::ClampToEdge;
    samplerDesc.addressModeV = AddressMode::ClampToEdge;
    samplerDesc.addressModeW = AddressMode::ClampToEdge;
    samplerDesc.magFilter = FilterMode::Linear;
    samplerDesc.minFilter = FilterMode::Linear;
    samplerDesc.mipmapFilter = MipmapFilterMode::Linear;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 1.0f;
    samplerDesc.compare = CompareFunction::Undefined;
    samplerDesc.maxAnisotropy = 1;
    sampler = device.createSampler(samplerDesc);
}

void TextureHolder::WriteToTexture(Queue &queue, const std::vector<uint8_t> &pixels) {
    // What part of texture we upload to
    ImageCopyTexture destination;
    destination.texture = texture;
    destination.mipLevel = 0; // Write to the 1st mip level
    destination.origin = {0,0,0}; // Offset we write to
    destination.aspect = TextureAspect::All; // irrelevant for color textures

    // How is the C++ pixel memory laid out?
    TextureDataLayout source;
    source.offset = 0; // Offset of what we read from
    source.bytesPerRow = 4 * textureDesc.size.width; // Stride (may be rounded up)
    source.rowsPerImage = textureDesc.size.height; // (Per Depth Slice) Important for uploading multiple images at once

    queue.writeTexture(destination, pixels.data(), pixels.size(), source, textureDesc.size);
}

void TextureHolder::Destroy() {
    sampler.release();
    texture.destroy();
    texture.release();
}