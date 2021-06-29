#include "../semantics.hpp"
#include "../error.hpp"
#include <libgen.h>


class MyDir : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        std::string x = callContext->begin.fileName;
        std::string n = dirname((char *)x.c_str());

        return n;
    }
};

REGISTER_FUNCTION_STR(my_dir, MyDir, "my-dir");
