#include "../semantics.hpp"
#include "../utils.hpp"
#include "../error.hpp"

class BuiltinFnShell : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        std::string gargs;

        for (auto i = args.begin(); i != args.end(); i++) {
            if (i != args.begin())
                gargs += ","; //TODO not sure if this is handled by the parser context?
            gargs += context->eval(*i);
        }

        FILE *proc = popen(gargs.c_str(), "r");

        std::string ret;

        char path[1024];
        while (fgets(path, 1024, proc) != NULL)
            ret += path;

        ret = rtrim(ret);

        int status = pclose(proc);

//        std::cerr << "(shell " + gargs + " ) returned " + ret << std::endl;


        //i dunno how this should work
        replace(ret, "\n", " ");
        return ret;
    }
};

REGISTER_FUNCTION(shell, BuiltinFnShell);
