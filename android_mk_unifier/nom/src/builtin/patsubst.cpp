#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"


/*  Straight from the gnumake manual:
 *
 * '%' characters in patsubst function invocations can be quoted with preceding backslashes ('\').
 * Backslashes that would otherwise quote '%' characters can be quoted with more backslashes.
 * Backslashes that quote '%' characters or other backslashes are removed from the pattern before
 * it is compared file names or has a stem substituted into it. Backslashes that are not in danger
 * of quoting '%' characters go unmolested. For example, the pattern 'the\%weird\\%pattern\\'
 * has 'the%weird\' preceding the operative '%' character, and 'pattern\\' following it.
 * The final two backslashes are left alone because they cannot affect any '%' character.
 *
 * So, does it behave like this?
 *
 * %       --> {EXPANDED}       = 0 % 2 = 0
 * \%      --> %                = 1 % 2 = 1
 * \\%     --> \{EXPANDED}      = 2 % 2 = 0
 * \\\%    --> \\%              = 3 % 2 = 1
 * \\\\%   --> \\\{EXPANDED}    = 4 % 2 = 0
 * \\\\\%  --> \\\\%            = 5 % 2 = 1
 *
 * Or even like this?
 *
 * %       --> {EXPANDED}       = 0
 * \%      --> %                = 1
 * \\%     --> \{EXPANDED}      = 2
 * \\\%    --> \\{EXPANDED}     = 2
 * \\\\%   --> \\\{EXPANDED}    = 2
 * \\\\\%  --> \\\\{EXPANDED}   = 2
 *
 * i dunno ... this is just batshit stupid
 *
 */




void splitshit(const std::string &in, std::string &a, std::string &b)
{
    bool found = false;
    int escaped = 0;
    for (int i = 0; i < in.size(); i++) {
        if (in.at(i) == '\\') {
            ++escaped;
            b += in.at(i);
        } else if (in.at(i) == '%') {
            if (escaped == 1) {
                b += '%';
            } else if (found) {
                throw Error("in pattern contains multiple % thingies");
            } else {
                a = b;
                b.resize(0);
                found = true;
                if (escaped > 1) {
                    a.resize(a.size() - 1);
                }
            }
        } else {
            escaped = 0;
            b += in.at(i);
        }
    }
}




class BuiltinFnPatsubst : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        if (args.size() != 3)
            throw Error("expecting exactly 3 arguments to function patsubst");

        auto arg0 = context->eval(args.at(0));
        auto arg1 = context->eval(args.at(1));
        auto arg2 = context->eval(args.at(2)).split();


        std::string searchA, searchB, replacA, replacB;


        splitshit(arg0, searchA, searchB);
        splitshit(arg1, replacA, replacB);

        VString ret;

        for (auto i = arg2.begin(); i != arg2.end(); i++) {
            std::string &me = *i;

            size_t ea = me.find(searchA);
            if (ea == std::string::npos || ea > 0) {
                ret.push_back(me);
                continue;
            }

            ea += searchA.size();

            size_t eb = std::string::npos;
            if (searchB.size()) {
                eb = me.find(searchB, ea);
                if (eb == std::string::npos)
                    continue;
            }

            std::string matched = me.substr(ea, eb - ea);

            ret.push_back(replacA + matched + replacB);

        }

        return ret;
    }
};


REGISTER_FUNCTION(patsubst, BuiltinFnPatsubst);
