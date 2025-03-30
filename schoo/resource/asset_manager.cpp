#include"asset_manager.hpp"
#include<iostream>


namespace schoo {

    std::unique_ptr<AssetManager>AssetManager::instance_ = nullptr;

    AssetManager &AssetManager::Instance() {
        //static AssetManager instance;
        return *instance_;
    }

    void AssetManager::loadObjFile(std::string filename) {

    }

    void AssetManager::loadGLTFFile(std::string filename) {
        tinygltf::Model glTFInput;
        tinygltf::TinyGLTF gltfContext;
        std::string error, warning;

        bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

        if(!warning.empty()){
            std::cout << "load glTF warning: "+warning << std::endl;
        }
        if(!error.empty()){
            std::cout << "load glTF error: "+ error << std::endl;
        }

        if (fileLoaded) {
            glTFModel.Init(glTFInput);
        } else {
            throw std::runtime_error("open gltf failed");
        }

    }

    void AssetManager::Init() {
        instance_.reset(new AssetManager);
    }

    void AssetManager::Quit() {
        instance_.reset();
    }

    void AssetManager::AddAsset(std::string name, std::string path) {

    }
}