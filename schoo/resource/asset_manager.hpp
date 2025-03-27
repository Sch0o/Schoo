#pragma once
#include"gltfModel.hpp"

namespace schoo{
    class AssetManager{
    public:
        GLTFModel glTFModel;

        struct Asset{
            std::string name;
            std::string path;
        };

        void AddAsset(std::string name,std::string path);

        void loadGLTFFile(std::string filename);
        void loadObjFile(std::string filename);

        static void Init();
        static void Quit();
        static AssetManager &Instance();
    private:

        AssetManager()=default;
        static std::unique_ptr<AssetManager>instance_;
    };


}
