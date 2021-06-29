#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"


class BuiltinAbsPath : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {

        auto arg0 = context->eval(args.at(0)).split();

        VString ret;

        for (auto i = arg0.begin(); i != arg0.end(); i++) {
            std::string &me = *i;

            char *rr = realpath(me.c_str(), NULL);
            if (rr)
                ret.push_back(rr);
            free(rr);

        }

        return ret;
    }
};


//TODO: realpath is wrong for abspath
REGISTER_FUNCTION(abspath, BuiltinAbsPath);
REGISTER_FUNCTION(realpath, BuiltinAbsPath);
