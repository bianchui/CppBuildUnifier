#ifndef MAKEFILESEMANTISDC_H_WUAST
#define MAKEFILESEMANTISDC_H_WUAST

#include "parser.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <map>
#include <set>
#include <iostream>


class VString : public std::string
{
public:
    VString()
        : std::string()
    {}
    VString(const std::string &s)
        : std::string(s)
    {
    }

    void push_back(const std::string &s) {
        if (size())
            (*this) += " ";
        (*this) += s;
    }

    std::vector<std::string> split() const;
};

class Context;
class MakefileSemantics;
class Function
{
public:
    virtual ~Function() {}
    virtual VString operator()(
            Context *ctx,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            ) = 0;


};


class IncludeHandler
{
public:
    virtual ~IncludeHandler() {}
    virtual bool operator()(
            Context *ctx,
            IncludeNode::Ptr node,
            MakefileParser *parser
            ) = 0;
};

class Context
{
public:
    std::map<std::string, Function *> functions;
    std::map<std::string, Expression::Ptr> values;
    std::vector<IncludeHandler *> includeHandlers;
    static Context *global;

    VString eval(Expression::Ptr);
    VString evalLiteral(LiteralExpression::Ptr);
    VString evalReference(ReferenceExpression::Ptr);
    VString evalCall(CallExpression::Ptr);
    VString evalCluster(Expression::Ptr);



private:
    friend class MakefileSemantics;
    void error (const std::string &text, Node::Ptr);
    std::set<std::string> stateInsideDeref;
};

#define REGISTER_FUNCTION_STR(name, klass, str) struct ___construct##name { \
    ___construct##name () {Context::global->functions[str] = new klass;} \
} static ___init_construct##name;

#define REGISTER_FUNCTION(name, klass) REGISTER_FUNCTION_STR(name, klass, #name)



class MakeScriptFunction;
class MakefileSemantics
{
public:


    Context *context;

    MakefileSemantics(MakefileParser *parser);
    MakefileParser *parser;



    void d();

private:
    void error (const std::string &text, Node::Ptr);


    bool n(Node::Ptr);
    bool nAssign(Assignment::Ptr);
    bool nCall(CallExpression::Ptr);
    bool nInclude(IncludeNode::Ptr);
    bool nIfCondition(IfCondition::Ptr, bool &result);
    bool nFunction(FunctionNode::Ptr);




    friend class MakeScriptFunction;
};


#endif
