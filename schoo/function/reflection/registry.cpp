#include"registry.hpp"

namespace schoo {
    Registry &Registry::Instance() {
        static Registry instance;
        return instance;
    }

    void Registry::Register(std::unique_ptr<TypeDescriptor> desc) {
        auto name = desc->GetName();
        descs[name] = std::move(desc);
    }


    TypeDescriptor *Registry::Find(const std::string &name) const {
        auto it = descs.find(name);
        if (it != descs.end()) {
            return it->second.get();
        } else {
            throw std::runtime_error("can`t find name in descs");
        }
    }

    std::vector<TypeDescriptor *> Registry::Get() const {
        std::vector<TypeDescriptor *> tempDescs;
        for (auto &desc: descs) {
            tempDescs.push_back(desc.second.get());
        }
        return tempDescs;
    }
}