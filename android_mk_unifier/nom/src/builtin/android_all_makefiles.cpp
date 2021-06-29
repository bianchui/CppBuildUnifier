#include "../semantics.hpp"
#include "../error.hpp"
#include <libgen.h>


class AllMakefiles : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        VString ret;
        std::string where;

        if (functionName == "first-makefiles-under") {
            where = context->eval(args[0]);

            Function *shell_f = context->functions["shell"];

            std::string shell = "build/tools/findleaves.py --prune=out --prune=.repo --prune=.git "
            "--mindepth=2 " + where + " Android.mk";

            std::vector <Expression::Ptr> aarg({Expression::Ptr(new LiteralExpression(shell))});
            return shell_f->operator()(context, "shell",
                    std::vector <Expression::Ptr> ({aarg}), callContext);
        }

        if (functionName == "all-subdir-makefiles") {
            std::string x = callContext->begin.fileName;
            where  = dirname((char *)x.c_str());
        }
        if (functionName == "all-makefiles-under") {
            std::string x = callContext->begin.fileName;
            where  = dirname((char *)x.c_str());

            if (args.size() < 1) {
                throw Error("need arg");
            }
            auto aa = context->eval(args[0]).split();
            if (aa.size() < 1) {
                throw Error("need arg");
            }

            //FIXME
            // fix what? i guess i should actually look at the definition.mk
            where += "/" + aa[0];
        }


        Function *wild = context->functions["wildcard"];
        std::string wi = where + "/*/Android.mk";
        std::vector <Expression::Ptr> nargs;
        nargs.push_back(Expression::Ptr(new LiteralExpression(wi)));
        ret = wild->operator()(context, "wildcard", nargs, callContext);


        return ret;
    }
};

REGISTER_FUNCTION_STR(all_subdir_makefiles, AllMakefiles , "all-subdir-makefiles");
REGISTER_FUNCTION_STR(all_makefiles_under,  AllMakefiles , "all-makefiles-under");
REGISTER_FUNCTION_STR(first_makefiles_under,  AllMakefiles , "first-makefiles-under");
