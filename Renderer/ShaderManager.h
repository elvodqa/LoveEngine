//
// Created by YAHAY on 29/12/2024.
//

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H
namespace renderer {

struct EngineShader {

};

}
namespace renderer::shader_manager {
    EngineShader CreateShader();
    void SerializeShader(EngineShader shader);
    EngineShader DeserializeShader();
}
#endif //SHADERMANAGER_H
