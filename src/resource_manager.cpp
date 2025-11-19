#include "resource_manager.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace wgpu;
using namespace std;

ShaderModule ResourceManager::LoadShaderModule(const std::filesystem::path &path, Device device) {
    std::ifstream file = std::ifstream(path);
    if(!file.is_open()) {
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    string shaderSource(size, ' ');
    file.seekg(0, std::ios::beg);
    file.read(shaderSource.data(), size);

    ShaderModuleWGSLDescriptor shaderCodeDesc = {};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str(); // !
    // std::cout << shaderCodeDesc.code << std::endl;

    ShaderModuleDescriptor shaderDesc = {};
    #ifdef WEBGPU_BACKEND_WGPU
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
    #endif
    shaderDesc.nextInChain = &shaderCodeDesc.chain;

    return device.createShaderModule(shaderDesc);
}