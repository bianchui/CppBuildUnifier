#include "../semantics.hpp"
#include <glob.h>
#include <iostream>


class Wildcard : public Function
{
public:
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

            for (auto j = arg.begin(); j != arg.end(); j++) {

                glob_t globbuf;
                glob((*j).c_str(), GLOB_NOSORT /*| GLOB_NOESCAPE*/, NULL, &globbuf);

                for (int i = 0; i < globbuf.gl_pathc; i++) {
                    ret.push_back(globbuf.gl_pathv[i]);
                }
                globfree(&globbuf);

            }
        }
        return ret;
    }
};

REGISTER_FUNCTION(wildcard, Wildcard);
