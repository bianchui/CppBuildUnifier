#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"

#include <string>


class Words : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        int n = 0;

        for (auto i = args.begin(); i != args.end(); i++) {
            auto  arg = context->eval(*i).split();
            for (auto s = arg.begin(); s != arg.end(); s++) {
                n++;
            }
        }

        return std::to_string(n);
    }
};


REGISTER_FUNCTION(words, Words);
