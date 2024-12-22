#pragma once
#include"gltfModel.hpp"

namespace schoo{
    class AssetManager{
    public:
        GLTFModel glTFModel;

        void loadGLTFFile(std::string filename);
        static void Init();
        static void Quit();
        static AssetManager &GetInstance();
    private:
        AssetManager()=default;
        static std::unique_ptr<AssetManager>instance_;

    };

}
