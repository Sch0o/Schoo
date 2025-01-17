#include"typeDescriptor.hpp"
namespace schoo{
    const std::string &TypeDescriptor::GetName() const {
        return name;
    }

    const std::vector<MemberVariable> &TypeDescriptor::GetMeberVariables() const {
        return vars;
    }

    const std::vector<MemberFunction> &TypeDescriptor::GetMemberFunctions() const {
        return funcs;
    }

    MemberVariable TypeDescriptor::GetMemberVariable(const std::string&name) const {
        for(auto&var:vars){
            if(var.GetName()==name){
                return var;
            }
        }
        return MemberVariable{};
    }

    MemberFunction TypeDescriptor::GetMemberFunction(const std::string&name) const {
        for(auto&func:funcs){
            if(func.getName()==name){
                return func;
            }
        }
        return MemberFunction{};
    }
}

