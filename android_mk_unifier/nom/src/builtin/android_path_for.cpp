#include "../semantics.hpp"
#include "../error.hpp"
#include <libgen.h>


class IncludePathFor : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        return "i/dont/know/the/include/path/for/" + context->eval(args.at(0));
    }
};

REGISTER_FUNCTION_STR(include_path_for, IncludePathFor, "include-path-for");



class LocalIntermediatesDir: public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        return std::string("i/dont/know/the/local/intermediates/dir/");
    }
};

class IntermediatesDirFor : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext)
    {
        return "i/dont/know/the/intermediates/dir/for/" + context->eval(args.at(0));
    }
};

REGISTER_FUNCTION_STR(local_intermediates_dir, LocalIntermediatesDir, "local-intermediates-dir");
REGISTER_FUNCTION_STR(intermediates_dir_for, IntermediatesDirFor, "intermediates-dir-for");
