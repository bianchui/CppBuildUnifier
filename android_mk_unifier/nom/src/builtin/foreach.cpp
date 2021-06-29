#include "../semantics.hpp"
#include "../error.hpp"
#include <glob.h>


class Foreach : public Function
{
public:
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {
        if (args.size() != 3)
            throw Error("expecting exactly 3 arguments to foreach");

        VString name   = context->eval(args.at(0));
        auto source = context->eval(args.at(1)).split();


        CallExpression::Ptr call;

        if (args.at(2)->type == Node::CallExpression ) {
            call = std::static_pointer_cast<CallExpression>(args.at(2));
        } else if (args.at(2)->type == Node::Expression) {
            Expression::Ptr texp = std::static_pointer_cast<Expression>(args.at(2));
            if (texp->expression.size() != 1) {
                throw Error("third argument to foreach must be a call expression..");
            }
            if (texp->expression.at(0)->type != Node::CallExpression) {
                throw Error("third argument to foreach must be a call expression...");
            }
            call = std::static_pointer_cast<CallExpression>(texp->expression.at(0));
        } else {
            throw Error("third argument to foreach must be a call expression...");
        }




        VString ret;

        for (auto s = source.begin(); s != source.end(); s++ ) {
            context->values[name] = Expression::Ptr(Expression::Ptr(new LiteralExpression(*s)));

            auto r = context->evalCall(call);
            ret.insert(ret.end(), r.begin(), r.end());
        }

        return ret;
    }
};

REGISTER_FUNCTION(foreach, Foreach);
