//
//反射成员变量
//
#pragma once

#include <iostream>
#include <any>
#include <functional>

namespace schoo {
    class MemberVariable {
    public:
        MemberVariable() = default;

        template<class Class, typename Var>
        MemberVariable(Var Class::*var) {
            getter = [var](std::any obj) -> std::any {
                return std::any_cast<const Class *>(obj)->*var;
            };
            setter = [var](std::any obj, std::any val) {
                auto *self = std::any_cast<Class *>(obj);
                self->*var = std::any_cast<Var>(val);
            };
        };

        const std::string &GetName() const {
            return name;
        };

        template<typename Var, class Class>
        Var GetValue(const Class &obj) const {
            return std::any_cast<Var>(getter(&obj));
        }

        template<typename Var, class Class>
        void SetValue(Class &obj, Var value) {
            setter(&obj, value);
        }

    private:
        std::string name;

        std::function<std::any(std::any)> getter{nullptr};
        std::function<void(std::any, std::any)> setter{nullptr};
    };


}
