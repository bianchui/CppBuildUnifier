#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"


class BuiltinFnSubst : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        if (args.size() != 3)
            throw Error("expecting exactly 3 arguments to function subst");

        auto arg0 = context->eval(args.at(0));
        auto arg1 = context->eval(args.at(1));
        auto arg2 = context->eval(args.at(2)).split();


        VString ret;

        for (auto i = arg2.begin(); i != arg2.end(); i++) {
            std::string &me = *i;

            replace(me, arg0, arg1);
            ret.push_back(me);

        }

        return ret;
    }
};


REGISTER_FUNCTION(subst, BuiltinFnSubst);
