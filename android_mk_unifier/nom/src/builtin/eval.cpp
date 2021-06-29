#include "../semantics.hpp"
#include <iostream>

class BuiltinEval : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        std::cerr << "[SEM] eval doesnt do anything yet" << std::endl;

        return VString();
    }
};

REGISTER_FUNCTION(eval,  BuiltinEval);

