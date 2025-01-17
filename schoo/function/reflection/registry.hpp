#pragma once

#include <unordered_map>
#include <memory>
#include "string"
#include"typeDescriptor.hpp"

namespace schoo {
    class Registry {
    public:
        static Registry &Instance();

        TypeDescriptor *Find(const std::string &name) const;

        std::vector<TypeDescriptor *> Get() const;

        void Register(std::unique_ptr<TypeDescriptor> desc);

        void Clear() {
            decltype(descs) temp;
            descs.swap(temp);
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<TypeDescriptor>> descs;

    };
}