#include "../semantics.hpp"
#include <iostream>

class BuiltinFnPrint : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        std::cout << functionName << ": ";
        for (auto i = args.begin(); i != args.end(); i++) {
            std::cout << context->eval(*i) << " ";
        }
        std::cout << std::endl;


        if (functionName == "error")
            exit (8);

        return VString();
    }
};

REGISTER_FUNCTION(info,    BuiltinFnPrint);
REGISTER_FUNCTION(warning, BuiltinFnPrint);
REGISTER_FUNCTION(error,   BuiltinFnPrint);

