#include "semantics.hpp"
#include "utils.hpp"
#include "error.hpp"

#include <fstream>
#include <sstream>
#include <iostream>



std::vector<std::string> VString::split() const
{
    std::vector<std::string> elems;
    std::stringstream ss(*this);
    std::string item;
    while(std::getline(ss, item, ' ')) {
        item = trim(item);
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}

Context * Context::global = new Context;


MakefileSemantics::MakefileSemantics(MakefileParser *parser)
    : parser(parser)
    , context(Context::global)
{
}

void MakefileSemantics::d()
{
    Node::Ptr i;
    for (Node::Ptr i = parser->next(); i ; i = parser->next() ) {
        n(i);
    }
}


bool MakefileSemantics::n(Node::Ptr node)
{
    //std::cerr << "> " << node->begin.fileName << " :" << node->begin.line << ": "  << node->type << std::endl;
    switch (node->type)
    {
        case Node::Assignment:
            return nAssign(std::static_pointer_cast<Assignment>(node));

        case Node::CallExpression:
            return nCall(std::static_pointer_cast<CallExpression>(node));

        case Node::Include:
            return nInclude(std::static_pointer_cast<IncludeNode>(node));

        case Node::Function:
            return nFunction(std::static_pointer_cast<FunctionNode>(node));

        case Node::ElseCondition: {
           // if we go here, it means the acompaning if condition was true
           // so we eat everything else until endif
            int st = 1;
            for (Node::Ptr i = parser->next(); i ; i = parser->next() ) {
                if (i->type == Node::IfCondition)
                    ++st;
                if (i->type == Node::EndifCondition)
                    if (--st < 1)
                        return true;
            }
            error("premature end of file looking for endif", node);
        }
        case Node::EndifCondition: {
             // same here. the if was true, so everything until here was just inlined
             return true;
        }
        case Node::IfCondition: {
            bool result = true;
            nIfCondition(std::static_pointer_cast<IfCondition>(node), result);
            if (result)
                return true;

            int st = 1;
            for (Node::Ptr i = parser->next(); i ; i = parser->next() ) {
                if (i->type == Node::IfCondition) {
                    ++st;
                } else if (i->type == Node::ElseCondition && st == 1) {
                    return true;
                } if (i->type == Node::EndifCondition) {
                    if (--st < 1)
                        return true;
                }
            }
            error("premature end of file looking for endif", node);
        }

        case Node::Invalid:
        default:
            std::cerr << "[SEM] ? " << node->begin.line << ": "  << node->type << std::endl;
            break;
    }
    return true;
}

bool MakefileSemantics::nAssign(Assignment::Ptr node)
{
    VString lhs = context->eval(node->lhs);
    if (lhs.find(' ') != std::string::npos)
        throw Error("LHS of an assignment cannot contain space");

    Expression::Ptr rhse;
    if (node->op == "=") {
        context->values[lhs] = node->rhs;
        return true;
    } else if (node->op == "+=") {
        //TODO: behaviour here would depend on if the variable was first assigned with := or =
        rhse = context->values[lhs];
    } else if (node->op == ":=") {
    }


    if (rhse) {
        if (node->rhs && node->rhs->expression.size())
            rhse->expression.push_back(Expression::Ptr(new LiteralExpression(" ")));
    } else {
        rhse = Expression::Ptr(new Expression);
    }
    VString n = context->eval(node->rhs);
    rhse->expression.push_back(Expression::Ptr(new LiteralExpression(n)));
    context->values[lhs] = rhse;
    return true;
}

bool MakefileSemantics::nCall(CallExpression::Ptr c)
{
    context->eval(c);
    return true;
}




bool MakefileSemantics::nInclude(IncludeNode::Ptr c)
{

    for (auto i = context->includeHandlers.begin(); i != context->includeHandlers.end(); i++) {
        if ((**i)(context, c, parser))
            return true;
    }

    std::vector<std::string> ee = context->eval(c->expr).split();

    for (auto i = ee.begin(); i != ee.end(); i++) {
        try {
            std::cerr << "+ include " << *i << std::endl;
            parser->insert(*i);
        } catch (Error &e) {
            if (c->ignoreErrors)
                std::cerr << "- (include error ignored) " << e.text << std::endl;
            else
                throw e;
        }
    }
    return true;
}

bool MakefileSemantics::nIfCondition(IfCondition::Ptr cond,  bool &result)
{

    //args actually have to be evaluated here because empty string equals undefined

    if (cond->function == "ifdef") {
        result = false;
        if (cond->args.size()) {
            auto name = context->eval(cond->args[0]);
            if (!context->values.count(name)) {
                result = false;
            } else {
                result = !context->eval(context->values[name]).empty();
            }
        }
    } else if (cond->function == "ifndef") {
        result = true;
        if (cond->args.size()) {
            auto name = context->eval(cond->args[0]);
            if (!context->values.count(name)) {
                result = true;
            } else {
                result = !!context->eval(context->values[name]).empty();
            }
        }
    } else if (cond->function == "ifeq" ||  cond->function == "ifneq") {
        if (cond->args.size() < 2) {
            throw Error("expecting two arguments to ifeq");
        }

        VString lhs = context->eval(cond->args[0]);
        VString rhs = context->eval(cond->args[1]);

        result = (lhs == rhs);

        if (cond->function == "ifneq")
            result = !result;

    } else {
        error("unknown conditional" ,cond);
    }

    return true;

}

class MakeScriptFunction : public Function
{
    FunctionNode::Ptr node;
public:
    MakeScriptFunction(FunctionNode::Ptr node);
    virtual VString operator()(
            Context *ctx,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            );
};

bool MakefileSemantics::nFunction(FunctionNode::Ptr fn)
{
    if (!fn->valid)
        return true;

    std::cerr << "defining function " << context->eval(fn->functionName)[0] << std::endl;

    std::string fnName = context->eval(fn->functionName);
    context->functions[fnName] = new MakeScriptFunction(fn);
}

    MakeScriptFunction::MakeScriptFunction(FunctionNode::Ptr node)
: node(node)
{
}

VString MakeScriptFunction::operator()(
        Context *context,
        const std::string &functionName,
        std::vector <Expression::Ptr> args,
        Expression::Ptr callContext
        )

{
    VString ret;

    int nr = 1;
    for (auto i = args.begin(); i != args.end(); i++) {
        context->values[std::to_string(nr++)] = *i;
    }

    for (auto c = node->code.begin(); c != node->code.end(); c++) {
        if ((*c)->type != Node::CallExpression) {
            throw Error("something used as function that is not a call expression", node);
        }

        VString r = context->evalCall(std::static_pointer_cast<CallExpression>(*c));
        ret.insert(ret.end(), r.begin(), r.end());
    }

    return ret;
}


void MakefileSemantics::error (const std::string &text, Node::Ptr node)
{
    context->error(text,node);
}
void Context::error (const std::string &text, Node::Ptr node)
{
    throw Error (text, node);
}




VString Context::eval(Expression::Ptr expr)
{
    switch (expr->type) {
        case Node::LiteralExpression:
            return evalLiteral(std::static_pointer_cast<LiteralExpression>(expr));
        case Node::ReferenceExpression:
            return evalReference(std::static_pointer_cast<ReferenceExpression>(expr));
        case Node::CallExpression:
            return evalCall(std::static_pointer_cast<CallExpression>(expr));
        case Node::Expression:
            return evalCluster(expr);
        default:
            error ("unsupported expression to eval", expr);
    }
}

VString Context::evalLiteral(LiteralExpression::Ptr e)
{
    return e->literal;
}

VString Context::evalReference(ReferenceExpression::Ptr e)
{
    VString ref = eval(e->reference);

    if (stateInsideDeref.count(ref)) {
        error("variable " + ref + " references itself. did you mean to use a := instead of = somewhere?", e);
    }

    stateInsideDeref.insert(ref);

    VString ret;
    if (!values.count(ref)) {
        //weird: user defined functions may be called without "call"

        if (functions.count(ref)) {
            Function *f = functions[ref];
            try {
                ret = f->operator()(this, ref, std::vector<Expression::Ptr>(),e );
                goto evalReferenceExit;
            } catch (...) {}
        }


        // get from env
        if (getenv(ref.c_str())) {
            std::string fret = getenv(ref.c_str());
            std::cerr << "from env " << ref << "=" << fret << std::endl;
            ret = fret;
            goto evalReferenceExit;
        }

        stateInsideDeref.erase(ref);
        error("undefined reference \"" + ref + "\"", e);
    }

    ret = eval(values[ref]);
evalReferenceExit:
    stateInsideDeref.erase(ref);
    return ret;
}

VString Context::evalCall(CallExpression::Ptr call)
{

    std::vector<Expression::Ptr> args = call->args;

    VString functionName = eval(call->functionName);



    if (functionName == "call") {
        if (args.size () < 1)
            error("expecting functionname argument to call", call);

        functionName = eval(args.front());
        args.erase(args.begin());
    }

    if (functions.count(functionName)) {
        Function *f = functions[functionName];
        return f->operator()(this, functionName, args, call);
    }

    error("undefined function \"" + functionName + "\"", call);
}

VString Context::evalCluster(Expression::Ptr expr)
{
    VString ret;

    for (auto i = expr->expression.begin(); i != expr->expression.end(); i++) {
        VString lr = eval(*i);
        ret += lr;
    }
    return ret;
}

