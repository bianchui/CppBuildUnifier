#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"



class Strip : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        VString ret;

        for (auto i = args.begin(); i != args.end(); i++) {
            auto arg = context->eval(*i).split();
            for (auto s = arg.begin(); s != arg.end(); s++) {
                ret.push_back(trim(*s));
            }
        }

        return ret;
    }
};


REGISTER_FUNCTION(strip, Strip);
