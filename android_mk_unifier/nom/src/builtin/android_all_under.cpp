#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"
#include <libgen.h>





const char * filtermap[] =
{
    "java"    , "*.java",
    "c"       , "*.c",
    "Iaidl"   , "I*.aidl",
    "logtags" , "*.logtags",
    "proto"   , "*.proto",
    "renderscript" , "*.rs",
    "html"    , "*.html",
    0
};


class AllUnder : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        VString ret;

        Function *shell_f = context->functions["shell"];
        Function *mydir_f = context->functions["my-dir"];




        std::vector<std::string> fname = split(functionName, '-');
        if (fname.size() != 4) {
            throw  Error("weird functionname to AllUnder " + functionName);
        }


        if (fname[1] == "subdir") {
            std::string where = mydir_f->operator()(context, "my-dir",
                    std::vector <Expression::Ptr> (), callContext);

            std::vector <Expression::Ptr> aarg({Expression::Ptr(new LiteralExpression(where))});

            std::string what = fname[2];

            return operator()(context, "all-" + what + "-files-under",
                     std::vector <Expression::Ptr> (aarg), callContext);
        } else if (fname[3] == "under") {
            std::string what = fname[1];
            std::string filter;
            for (const char **i = filtermap; *i; i++) {
                if (*i == what) {
                    filter = *(++i);
                    break;
                }
            }
            if (filter.empty())
                throw Error("cant map filter " + what);



            auto local_path = context->eval(context->values["LOCAL_PATH"]);
            auto where = context->eval(args[0]);

            std::string shell = "cd '" + local_path + "' ; find '" + where + "' -name \"" + filter + "\" -and -not -name \".*\"";

            std::vector <Expression::Ptr> aarg({Expression::Ptr(new LiteralExpression(shell))});


            return shell_f->operator()(context, "shell",
                    std::vector <Expression::Ptr> ({aarg}), callContext);


        }
    }
};

REGISTER_FUNCTION_STR(all_subdir_java_files, AllUnder, "all-subdir-java-files");
REGISTER_FUNCTION_STR(all_java_files_under,  AllUnder, "all-java-files-under");

REGISTER_FUNCTION_STR(all_renderscript_files_under, AllUnder, "all-renderscript-files-under");
REGISTER_FUNCTION_STR(all_subdir_renderscript_files, AllUnder, "all-subdir-renderscript-files");

REGISTER_FUNCTION_STR(all_subdir_c_files, AllUnder, "all-subdir-c-files");
REGISTER_FUNCTION_STR(all_c_files_under,  AllUnder, "all-c-files-under");

REGISTER_FUNCTION_STR(all_subdir_html_files, AllUnder, "all-subdir-html-files");
REGISTER_FUNCTION_STR(all_html_files_under,  AllUnder, "all-html-files-under");

REGISTER_FUNCTION_STR(all_subdir_logtags_files, AllUnder, "all-subdir-logtags-files");
REGISTER_FUNCTION_STR(all_logtags_files_under,  AllUnder, "all-logtags-files-under");

REGISTER_FUNCTION_STR(all_subdir_proto_files, AllUnder, "all-subdir-proto-files");
REGISTER_FUNCTION_STR(all_proto_files_under,  AllUnder, "all-proto-files-under");

REGISTER_FUNCTION_STR(all_subdir_Iaidl_files, AllUnder, "all-subdir-Iaidl-files");
REGISTER_FUNCTION_STR(all_Iaidl_files_under,  AllUnder, "all-Iaidl-files-under");
