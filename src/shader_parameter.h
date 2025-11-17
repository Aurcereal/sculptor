#pragma once

#include <variant>
#include <iostream>
#include "texture_holder.h"
#include "webgpu_utils.h"

struct BufferHolder;

namespace ShaderParameter {
    enum class Type {
        TEXTURE,
        SAMPLER,
        UNIFORM,
        BUFFER
    };

    struct UTexture {
        const TextureHolder *textureHolder;
        bool is3D;
        bool shaderWriteEnabled; // Texture can be write enabled but used for reading for this particular parameter, set to false in that case
    };

    struct USampler {
        const TextureHolder *textureHolder;
    };

    struct UUniform {
        const BufferHolder *uniformBufferHolder;
    };

    struct UBuffer {
        const BufferHolder *bufferHolder;
        bool forWriting; // read-only OR write-only
    };

    // using ParameterData = std::variant<Texture, Uniform>;

    struct Parameter {
        Type type;
        union {
            UTexture texture;
            USampler sampler;
            UUniform uniform;
            UBuffer buffer;
        } parameterData;
    };

    // inline static Type GetType(const Parameter& param) {
    //     std::visit([](const auto &paramData) {
    //         using T = std::decay_t<decltype(paramData)>;
    //         if constexpr(std::is_same_v<T, Texture>) {
    //             return Type::TEXTURE;
    //         } else if constexpr(std::is_same_v<T, Uniform>) {
    //             return Type::UNIFORM;
    //         }
    //         std::cerr << "Parameter not a valid type!" << std::endl;
    //     }, param.parameterData);
    // }
};