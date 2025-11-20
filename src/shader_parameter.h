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

        //inline USampler(const TextureHolder *t) : textureHolder(t) {}
    };

    struct UUniform {
        const BufferHolder *uniformBufferHolder;

        //inline UUniform(const BufferHolder *b) : uniformBufferHolder(b) {}
    };

    struct UBuffer {
        const BufferHolder *bufferHolder;
        bool forWriting; // read-only OR write-only

        //inline UBuffer(const BufferHolder *b, bool fw) : bufferHolder(b), forWriting(fw) {}
    };

    // using ParameterData = std::variant<Texture, Uniform>;

    struct Parameter {
        Type type;
        union {
            UTexture texture;
            USampler sampler;
            UUniform uniform;
            UBuffer buffer;
        };

        inline Parameter(UTexture tex) : type(Type::TEXTURE), texture(tex) {}
        inline Parameter(USampler sam) : type(Type::SAMPLER), sampler(sam) {}
        inline Parameter(UUniform uni) : type(Type::UNIFORM), uniform(uni) {}
        inline Parameter(UBuffer buf) : type(Type::BUFFER), buffer(buf) {}
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