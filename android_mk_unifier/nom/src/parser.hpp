#ifndef MAKEFILEPARSER_H_WUAST
#define MAKEFILEPARSER_H_WUAST

#include "lexer.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <map>
#include <queue>

struct Node
{
    typedef std::shared_ptr<Node> Ptr;

    enum Type {
        Invalid              = 0,
        Expression           = 1,
        Assignment           = 2,
        ReferenceExpression  = 3,
        LiteralExpression    = 4,
        CallExpression       = 5,
        IfCondition          = 6,
        ElseCondition        = 7,
        EndifCondition       = 8,
        Generator            = 9,
        PrivateAssignment    = 10,
        GeneratorCondition   = 11,
        ShellCommand         = 12,
        Include              = 13,
        Function             = 14
    };
    Type type;

    Token begin;
    Token end;

    std::string dump() const;

    Node(Type t)
        : type (t)
    {
    }
};

struct Expression : Node
{
    typedef std::shared_ptr<Expression> Ptr;

    Expression(Node::Type t = Node::Expression)
        : Node(t)
    {}

    std::vector<Expression::Ptr> expression;
};

struct Assignment : Node
{
    typedef std::shared_ptr<Assignment> Ptr;

    Assignment()
        : Node(Node::Assignment)
    {}

    bool exported;
    std::string op;
    Expression::Ptr lhs;
    Expression::Ptr rhs;
};

struct ReferenceExpression : Expression
{
    typedef std::shared_ptr<ReferenceExpression> Ptr;

    ReferenceExpression ()
        : Expression(Node::ReferenceExpression)
    {}

    ReferenceExpression (Expression::Ptr ref)
        : Expression(Node::ReferenceExpression)
        , reference(ref)
    {}

    Expression::Ptr reference;
};

struct LiteralExpression : Expression
{
    typedef std::shared_ptr<LiteralExpression> Ptr;

    LiteralExpression (const std::string &literal = std::string())
        : Expression(Node::LiteralExpression)
        , literal(literal)
    {}

    std::string literal;
};

struct CallExpression : Expression
{
    typedef std::shared_ptr<CallExpression> Ptr;

    CallExpression ()
        : Expression(Node::CallExpression)
    {}

    Expression::Ptr functionName;
    std::vector<Expression::Ptr>  args;
};

struct Condition : Node
{
    Condition(Node::Type t)
        : Node(t)
    {}
};

struct IfCondition : Condition
{
    typedef std::shared_ptr<IfCondition> Ptr;

    IfCondition()
        : Condition(Node::IfCondition)
    {}

    std::string function;
    std::vector<Expression::Ptr> args;

};

struct ElseCondition : Condition
{
    ElseCondition()
        : Condition(Node::ElseCondition)
    {}
};

struct EndifCondition : Condition
{
    EndifCondition()
        : Condition(Node::EndifCondition)
    {}
};

struct GeneratorCode: Node
{
    typedef std::shared_ptr<GeneratorCode> Ptr;

    GeneratorCode(Node::Type t)
        : Node(t)
    {}
};

struct PrivateAssignment : GeneratorCode
{
    typedef std::shared_ptr<PrivateAssignment> Ptr;

    PrivateAssignment()
        : GeneratorCode(Node::PrivateAssignment)
    {}
    Expression::Ptr  lhs;
    Assignment::Ptr assignment;
};

struct GeneratorCondition : GeneratorCode
{
    typedef std::shared_ptr<GeneratorCondition> Ptr;

    GeneratorCondition()
        : GeneratorCode(Node::GeneratorCondition)
    {}
    Condition::Ptr condition;
};

struct ShellCommand : GeneratorCode
{
    typedef std::shared_ptr<ShellCommand> Ptr;

    ShellCommand()
        : GeneratorCode(Node::ShellCommand)
    {}
    Expression::Ptr line;
    std::vector<Condition::Ptr> conditions;
};

struct Generator : Node
{
    typedef std::shared_ptr<Generator> Ptr;

    Generator()
        : Node(Node::Generator)
    {}
    Expression::Ptr lhs;
    Expression::Ptr rhs;
    Expression::Ptr orderonly;
    std::vector<GeneratorCode::Ptr> code;
};

struct FunctionNode : Node
{
    typedef std::shared_ptr<FunctionNode> Ptr;

    FunctionNode ()
        : Node(Node::Function)
    {}
    bool valid;
    Expression::Ptr functionName;
    std::vector<Node::Ptr> code;
};

struct IncludeNode : Node
{
    typedef std::shared_ptr<IncludeNode> Ptr;

    IncludeNode(Expression::Ptr expr)
        : Node(Node::Include)
        , expr(expr)
        , ignoreErrors(false)
    {}
    Expression::Ptr expr;
    bool ignoreErrors;
};

class MakefileParser
{
public:
    MakefileParser(const std::string &main);
    MakefileParser(const std::string &fileName, const char *data);


    void insert(const std::string &fileName);

    Node::Ptr next();

private:
    enum ExpressionContext
    {
        LHSContext,
        RHSContext,
        CallContext,
        SubstitutionContext,
        GeneratorContext,
        ShellContext
    };


    bool privateGeneratorAssignment(Node::Ptr &);
    bool shellCommand(ShellCommand::Ptr &);
    bool generatorNode(Node::Ptr &);

    bool ifConditionNode(Node::Ptr &);
    bool elseConditionNode(Node::Ptr &);
    bool endifConditionNode(Node::Ptr &);

    bool literalExpressionNode(Expression::Ptr &);
    bool referenceExpressionNode(Expression::Ptr &);
    bool substitutionReferenceExpressionNode(Expression::Ptr &);
    bool callExpressionNode(Expression::Ptr &);
    bool callArguments(std::vector<Expression::Ptr> &, ExpressionContext expressionContext = CallContext);

    bool expressionNode(Expression::Ptr &, ExpressionContext ctx = LHSContext,
            int *unmatchedLiteralBraces = 0, bool literalSpace = false);
    bool expressionList(Expression::Ptr &, ExpressionContext ctx = LHSContext);

    bool assignmentNode(Node::Ptr &);
    bool include(Node::Ptr &);

    bool whiteSpace();
    void maybeSpace();
    bool trash();
    bool defineFunction(Node::Ptr &);


    bool globalNode(Node::Ptr &);

    std::stack<MakefileLexer *> lexer;
    std::vector<Token> tokens;
    int tpos;

    bool  eof();
    Token nextToken();
    int   save();
    void  restore(int);


    bool error(const std::string &text, int error, int begin = -1, int end = -1);


};


#endif
