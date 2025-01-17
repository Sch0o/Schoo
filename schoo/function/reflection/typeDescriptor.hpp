//
//描述反射出的类
//
#include"iostream"
#include"memberVariable.hpp"
#include"memberFunction.hpp"
#include"vector"

namespace schoo {
    class TypeDescriptor {
    public:
        const std::vector<MemberVariable> &GetMeberVariables() const;

        const std::string &GetName() const;

        const std::vector<MemberFunction> &GetMemberFunctions() const;

        MemberVariable GetMemberVariable(const std::string&name) const;

        MemberFunction GetMemberFunction(const std::string&name) const;


    private:
        std::string name;
        std::vector<MemberVariable> vars;
        std::vector<MemberFunction> funcs;
    };
}
