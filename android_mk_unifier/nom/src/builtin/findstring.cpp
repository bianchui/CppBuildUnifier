#include "../semantics.hpp"
#include "../error.hpp"
#include <glob.h>


class FindString : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        if (args.size() < 2)
            throw Error("findstring expects two comma arguments");

        auto needle = context->eval(args.at(0)).split();

        auto e = context->eval(args.at(1)).split();
        for (auto j = e.begin(); j != e.end(); j++) {
            for (auto n = needle.begin(); n != needle.end(); n++) {

                if (j->find(*n) != std::string::npos) {
                    return *n;
                }
            }

        }

        return VString();
    }
};

REGISTER_FUNCTION(findstring,FindString);
