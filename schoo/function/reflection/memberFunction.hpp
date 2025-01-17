#pragma once

#include<iostream>
#include <string>

namespace schoo {
    class MemberFunction {
    public:
        const std::string &getName() const {
            return name;
        };

        template<class Class, typename... Args>
        explicit MemberFunction(void (Class::*func)(Args...)) {
            function=[]
        }

    private:
        std::string name;
    };
}