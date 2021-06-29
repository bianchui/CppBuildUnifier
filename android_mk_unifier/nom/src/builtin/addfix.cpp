#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"



class Addfix : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        if (args.size() != 2)
            throw Error("expecting exactly 2 arguments to function addsuffix");

        VString arg0 = context->eval(args.at(0));
        auto arg1 = context->eval(args.at(1)).split();

        VString r;

        if (functionName == "addsuffix") {
            for (auto i = arg1.begin(); i != arg1.end(); i++) {
                if (i != arg1.begin())
                    r += " ";
                r += *i;
                r += arg0;
            }
        } else if (functionName == "addprefix") {
            for (auto i = arg1.begin(); i != arg1.end(); i++) {
                if (i != arg1.begin())
                    r += " ";
                r += arg0;
                r += *i;
            }
        }

        return r;
    }
};


REGISTER_FUNCTION(addsuffix, Addfix);
REGISTER_FUNCTION(addprefix, Addfix);
