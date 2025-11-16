#pragma once

#include <variant>
#include <iostream>
#include "texture_holder.h"

namespace ShaderParameter {
    enum class Type {
        TEXTURE,
        SAMPLER,
        UNIFORM
    };

    struct Texture {
        const TextureHolder *textureHolder;
        bool is3D;
        bool shaderWriteEnabled; // Texture can be write enabled but used for reading for this particular parameter, set to false in that case
    };

    struct Sampler {
        const TextureHolder *textureHolder;
    };

    struct Uniform {
        const Buffer *uniformBuffer;
        int size;
    };

    // using ParameterData = std::variant<Texture, Uniform>;

    struct Parameter {
        Type type;
        union {
            Texture texture;
            Sampler sampler;
            Uniform uniform;
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