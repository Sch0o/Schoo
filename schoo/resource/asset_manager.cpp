#include"asset_manager.hpp"
#include"filesystem"
#include<iostream>


namespace schoo {

    std::unique_ptr<AssetManager>AssetManager::instance_ = nullptr;

    AssetManager &AssetManager::Instance() {
        //static AssetManager instance;
        return *instance_;
    }

    void AssetManager::loadObjFile(std::string filePath) {

    }

    void AssetManager::loadGLTFFile(std::string filePath) {
        tinygltf::Model glTFInput;
        tinygltf::TinyGLTF gltfContext;
        std::string error, warning;

        namespace fs=std::filesystem;
        fs::path path(filePath);

        bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, path.string());

        if(!warning.empty()){
            std::cout << "load glTF warning: "+warning << std::endl;
        }
        if(!error.empty()){
            std::cout << "load glTF error: "+ error << std::endl;
        }

        if (fileLoaded) {
            glTFModel.folderPath=path.parent_path().string();
            glTFModel.name=path.filename().string();
            std::cout<<"glTFModel folderPath: "+glTFModel.folderPath<<std::endl;
            std::cout<<"glTFModel name: "+glTFModel.name<<std::endl;
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