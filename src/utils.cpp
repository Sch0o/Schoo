#include "schoo/utils.hpp"
#include "filesystem"

namespace schoo{
    std::string ReadWholeFile(const std::string&filename){
        std::ifstream file(filename,std::ios::binary|std::ios::ate);

        if(!file.is_open()){
            std::cout<<"file open failed"<<std::endl;
            return std::string {};
        }

        auto size=file.tellg();
        std::string context;
        context.resize(size);
        file.seekg(0);
        file.read(context.data(),context.size());

        return context;
    }

    bool CheckPath(const std::string path){
        if(std::filesystem::exists(path)){
            return true;
        }else{
            std::cout<<"this path :"<<path<<"is not exist"<<std::endl;
            return false;
        }
    }
}