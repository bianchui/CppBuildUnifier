#include "parser.hpp"
#include "utils.hpp"
#include "error.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>


inline bool streq(const char *expect, const char *begin, const char *end)
{
    size_t a = strlen(expect);
    size_t b = end - begin;
    if (a != b)
        return false;
    return strncmp(expect, begin, a) == 0;
}

inline const char *nullToEmpty(const char *s)
{
    return s ? s : "";
}

MakefileParser::MakefileParser(const std::string &f)
    : tpos(0)
{
    insert(f);
}

MakefileParser::MakefileParser(const std::string &fileName, const char *data)
    : tpos(0)
{
    lexer.push(new MakefileLexer(fileName, data));
}


void MakefileParser::insert(const std::string &fileName)
{
    lexer.push(new MakefileLexer(fileName));
}

bool MakefileParser::error(const std::string &text, int error, int begin, int end)
{
    if (begin == -1)
        begin = error;
    if (end == -1)
        end = begin;


    if (begin > error)
        begin = error;

    restore(begin);
    Token tBegin = nextToken();

    restore(error);
    Token tError = nextToken();

    restore(end);
    Token tEnd = nextToken();


#if 0
    std::string source(tError.location.source);
    replace(source, "\t", " ");


    int errorCol = tError.location.column;
    int beginCol =  tBegin.location.column;
    if (beginCol > errorCol)
        beginCol = errorCol;

    std::stringstream oss;
    oss << tError.location.fileName << " :"
        << tError.location.line     << ":"
        << tError.location.column   << ":"
        << tError.typeName() << ": "
        << text << std::endl
        << source << std::endl
        << std::string(beginCol, ' ')
        << std::string(errorCol- beginCol, '~')
        << "^";
#endif
    std::stringstream oss;
    oss << nullToEmpty(tError.fileName) << " :"
        << tError.line     << ":"
        << tError.column   << ":"
        << tError.typeName() << ": "
        << text << std::endl
    ;

    throw Error(oss.str());

}

Node::Ptr MakefileParser::next()
{
    Node::Ptr node;
    int p = save();
    if (globalNode(node)) {
        if (!node) {
            error("internal error: node was zero", save());
        }
        if (node->begin.line < 1) {
            error("internal error: node has no location", save());
        }
        return node;
    }
    if (!eof()) {
        std::cerr << "trailing tokens." << std::endl;
        for (;;) {
            Token t = nextToken();
            std::cerr  << t.dump() << std::endl;
            if (t.type == Token::Eof)
                break;
        }
    }
    return Node::Ptr(0);
}


bool MakefileParser::shellCommand(ShellCommand::Ptr &cmd)
{
    int begin = save();
    Token n = nextToken();
    if (n.type != Token::Tab)
        return false;

    int p = save();
    if (nextToken().type == Token::Eol) {
        return shellCommand(cmd);
    }
    restore(p);

    Expression::Ptr expr;
    if (!expressionList(expr, ShellContext))
        error("invalid shell command", save(), begin);

    Token eol = nextToken();
    if (eol.type != Token::Eol && eol.type != Token::Eof)
        error("shell: expected end of line after shell command",  save() - 1, begin);

    ShellCommand *c = new ShellCommand;
    c->line  = expr;
    c->begin = n;
    c->end   = eol;

    cmd = ShellCommand::Ptr(c);
    return true;
}


bool MakefileParser::privateGeneratorAssignment(Node::Ptr &node)
{
    int begin = save();
    Expression::Ptr lhs;
    if (!expressionList(lhs))
        return false;

    maybeSpace();

    if (nextToken().type != Token::Colon)
        return false;

    maybeSpace();


    Node::Ptr as;
    if (!assignmentNode(as))
        return false;

    maybeSpace();

    Token eol = nextToken();
    if (eol.type != Token::Eol)
        error("expected end of line after private generator assignment",  save() - 1, begin);


    PrivateAssignment *n = new PrivateAssignment;
    n->begin = lhs->begin;
    n->end   = eol;
    n->lhs   = lhs;
    n->assignment = std::static_pointer_cast<Assignment>(as);

    node = Node::Ptr(n);
    return true;
}

bool MakefileParser::generatorNode(Node::Ptr &node)
{
    std::vector<GeneratorCode::Ptr> code;

    Expression::Ptr lhs;
    if (!expressionList(lhs))
        return false;

    maybeSpace();

    if (nextToken().type != Token::Colon)
        return false;

    maybeSpace();

    Expression::Ptr rhs;

    int begin = save();
    Token eol = nextToken();
    if (eol.type != Token::Eol && eol.type != Token::Pipe) {
        restore(begin);
        if (!expressionList(rhs, GeneratorContext)) {
            error("cannot parse RHS of generator expression", save() - 1, begin);
        }

        maybeSpace();
    } else {
        restore(begin);
    }
    if (!rhs)
        rhs = Expression::Ptr(new Expression);

    maybeSpace();

    Expression::Ptr orderonly;
    eol = nextToken();
    if (eol.type == Token::Pipe) {
        maybeSpace();

        if (!expressionList(orderonly, RHSContext)) {
            error("cannot parse order-only RHS of generator expression", save(), begin);
            return false;
        }

        maybeSpace();
        eol = nextToken();
    }

    if (eol.type != Token::Eol)
        error("expected end of line after generator",  save() - 1, begin);

    ShellCommand::Ptr sh;
    GeneratorCondition::Ptr gc;
    int p = save();
    for (;;) {
        p = save();
        if (shellCommand(sh)) {
            code.push_back(sh);
            continue;
        }
        restore(p);


        eol = nextToken();
        if (eol.type == Token::Eol) break;
        restore(p);

        if (trash()) continue;

        Node::Ptr cond;
        if (ifConditionNode(cond)) goto generatorCondition;
        restore(p);
        if (elseConditionNode(cond)) goto generatorCondition;
        restore(p);
        if (endifConditionNode(cond)) goto generatorCondition;
        restore(p);

        break;
generatorCondition:
        //FIXME conditionals may include non-shell commands
        // and shell alike, since they're kinda text preprocessind
        // what a fucked up language
        int p2 = save();
        if (!shellCommand(sh)) { restore(p); break; }
        restore(p2);


        gc = GeneratorCondition::Ptr(new GeneratorCondition);
        gc->condition = cond;



        code.push_back(gc);
        continue;
    }




    Generator *g  = new Generator;
    g->code       = code;
    g->lhs        = lhs;
    g->rhs        = rhs;
    g->orderonly  = orderonly;
    g->begin      = lhs->begin;
    if (code.size())
        g->end        = code.back()->end;
    else
        g->end        = eol;


    node = Node::Ptr(g);
    return true;
}

bool MakefileParser::ifConditionNode(Node::Ptr &node)
{
    Token fun = nextToken();
    if (fun.type != Token::Literal)
        return false;

    if (strncmp("if", fun.begin, 2) != 0)
        return false;

    if (!whiteSpace())
        return false;

    std::vector<Expression::Ptr> args;

    int p = save();
    Token t = nextToken();
    if (t.type == Token::BraceOpen) {

        if (!callArguments(args))
            error("expecting call arguments", p);

        t = nextToken();
        if (t.type != Token::BraceClose)
            error("expecting ) or , ", save());
    } else {
        restore(p);
        Expression::Ptr argl;
        if (!expressionList(argl))
            error("expecting expression", p);

        args.push_back(argl);
    }


    maybeSpace();
    if (nextToken().type != Token::Eol)
        error("expecting end of line after ifCondition", save() -1);


    IfCondition *i = new IfCondition;
    i->function = std::string(fun.begin, fun.end - fun.begin);
    i->args   = args;
    i->begin  = fun;
    i->end    = t;

    node = Node::Ptr(i);
    return true;
}

bool MakefileParser::elseConditionNode(Node::Ptr &node)
{
    Token t = nextToken();
    if (t.type != Token::Literal)
        return false;

    if (!streq("else", t.begin, t.end))
        return false;

    node = Node::Ptr(new ElseCondition);
    node->begin = t;
    node->end   = t;

    return true;
}

bool MakefileParser::endifConditionNode(Node::Ptr &node)
{
    Token t = nextToken();
    if (t.type != Token::Literal)
        return false;

    if (!streq("endif", t.begin, t.end))
        return false;

    maybeSpace();
    if (nextToken().type != Token::Eol)
        error("expecting end of line after endif", save() -1);

    node = Node::Ptr(new EndifCondition);
    node->begin = t;
    node->end   = t;

    return true;
}

bool MakefileParser::assignmentNode(Node::Ptr &node)
{
    bool exported = false;
    int p = save();
    Token t = nextToken();
    if (t.type == Token::Literal && streq("export", t.begin, t.end)) {
        exported = true;
        if (!whiteSpace())
            return false;
    }
    else
        restore(p);


    Expression::Ptr lhs;
    if (!expressionNode(lhs))
        return false;

    maybeSpace();
    Token op  = nextToken();
    if (op.type != Token::Operator)
        return false;

    maybeSpace();


    p = save();
    Expression::Ptr rhs;
    if (!expressionList(rhs, RHSContext)) {
        rhs = Expression::Ptr(new Expression);
        restore (p);
        Token t = nextToken();
        if (t.type != Token::Eol) {
            error("invalid expressionlist", save());
            return false;
        }
        // previous chain might want to see the EOL
        restore (p);
    }

    Assignment *i = new Assignment;
    i->exported = exported;
    i->lhs   = lhs;
    i->rhs   = rhs;
    i->op    = std::string(op.begin, op.end - op.begin);
    i->begin = t;
    i->end   = rhs ? rhs->end : op;
    node = Node::Ptr(i);
    return true;
}

bool MakefileParser::literalExpressionNode(Expression::Ptr &node)
{
    Token literal = nextToken();
    if (literal.type != Token::Literal) {
        return false;
    }

    LiteralExpression *n =  new LiteralExpression;
    n->literal = std::string(literal.begin, literal.end - literal.begin);
    n->begin = n->end = literal;
    node = Expression::Ptr(n);
    return true;
}

bool MakefileParser::referenceExpressionNode(Expression::Ptr &node)
{
    Token t = nextToken();
    if (t.type != Token::Expression) {
        if (t.type != Token::ShortExpression)
            return false;

        ReferenceExpression *n =  new ReferenceExpression;
        n->reference = Expression::Ptr(new LiteralExpression(
                    std::string(t.begin, t.end - t.begin)));
        node = Expression::Ptr(n);
        return true;
    }
    if (nextToken().type != Token::BraceOpen)
        return false;

    Expression::Ptr reference;
    if(!expressionNode(reference))
        return false;

    Token close =  nextToken();
    if (close.type != Token::BraceClose)
        return false;


    ReferenceExpression *n =  new ReferenceExpression;
    n->reference = reference;
    n->begin = t;
    n->end   = close;
    node = Expression::Ptr(n);
    return true;
}

bool MakefileParser::substitutionReferenceExpressionNode(Expression::Ptr &node)
{
    Token begin = nextToken();
    if (begin.type != Token::Expression)
        return false;
    if (nextToken().type != Token::BraceOpen)
        return false;

    Token literal = nextToken();
    if (literal.type != Token::Literal)
        return false;

    if (nextToken().type != Token::Colon)
        return false;

    Expression::Ptr pattern;
    if (!expressionNode(pattern, SubstitutionContext))
        return false;

    if (nextToken().type != Token::Operator)
        return false;

    Expression::Ptr replacement;
    if (!expressionNode(replacement, SubstitutionContext))
        return false;

    if (nextToken().type != Token::BraceClose)
        return false;


    ReferenceExpression *n =  new ReferenceExpression;
    n->reference = Expression::Ptr(new LiteralExpression(
                std::string(literal.begin, literal.end - literal.begin)
                ));

    CallExpression *c = new CallExpression;
    c->functionName = Expression::Ptr(new LiteralExpression("patsubst"));
    c->args = { pattern, replacement, Expression::Ptr(n) };

    node = Expression::Ptr(c);
    return true;
}

//   Expression = ReferenceExpression | CallExpression | LiteralExpression [ Expression ]


bool MakefileParser::expressionNode(Expression::Ptr &node, ExpressionContext ctx,
        int *unmatchedLiteralBraces, bool literalSpace)
{
    std::vector<Expression::Ptr> expr;
    for (;;) {
        Expression::Ptr node2;
        int p = save();
        if (referenceExpressionNode(node2)) {
            expr.push_back(node2);
            continue;
        }
        restore(p);
        if (substitutionReferenceExpressionNode(node2)) {
            expr.push_back(node2);
            continue;
        }
        restore(p);
        if (literalExpressionNode(node2)) {
            expr.push_back(node2);
            continue;
        }
        restore(p);
        if (callExpressionNode(node2)) {
            expr.push_back(node2);
            continue;
        }
        restore(p);
        if (nextToken().type == Token::Expression) {
            if (nextToken().type == Token::BraceOpen) {
                error("expression cannot be parsed", p, save());
            }
        }
        restore(p);
        if (literalSpace || ctx == ShellContext) {
            Token maybeSpace = nextToken();
            if (maybeSpace.type == Token::Space || maybeSpace.type == Token::Tab) {
                std::string ss(maybeSpace.begin, maybeSpace.end - maybeSpace.begin);

                // this should only be space if there's no other space
                replace(ss, "\\\n", " ");
                replace(ss, "\t", " ");
                auto sl = Expression::Ptr(new LiteralExpression(ss));
                expr.push_back(sl);
                continue;;
            }
            restore(p);
        }

        if (unmatchedLiteralBraces) {
            Token maybeOp = nextToken();
            if (maybeOp.type == Token::BraceOpen) {
                std::string br(maybeOp.begin, maybeOp.end - maybeOp.begin);
                expr.push_back(Expression::Ptr(new LiteralExpression(br)));
                expr.back()->begin = expr.back()->end = maybeOp;
                ++*(unmatchedLiteralBraces);
                continue;
            }
            if (*unmatchedLiteralBraces > 0 && maybeOp.type == Token::BraceClose) {
                std::string br(maybeOp.begin, maybeOp.end - maybeOp.begin);
                expr.push_back(Expression::Ptr(new LiteralExpression(br)));
                expr.back()->begin = expr.back()->end = maybeOp;
                --*(unmatchedLiteralBraces);
                continue;
            }
            restore(p);
        }


        if (ctx != LHSContext) {
            Token maybeOp = nextToken();

            if (ctx == RHSContext || ctx == CallContext || ctx == SubstitutionContext || ctx == ShellContext) {
                if (maybeOp.type == Token::Pipe) {
                    expr.push_back(Expression::Ptr(new LiteralExpression("|")));
                    expr.back()->begin = expr.back()->end = maybeOp;
                    continue;
                }
            }

            if (ctx == RHSContext || ctx == CallContext || ctx == ShellContext) {
                if (maybeOp.type == Token::Operator) {
                    expr.push_back(Expression::Ptr(new LiteralExpression(
                                    std::string(maybeOp.begin, maybeOp.end - maybeOp.begin))));
                    expr.back()->begin = expr.back()->end = maybeOp;
                    continue;
                }
            }
            if (ctx == RHSContext || ctx == CallContext || ctx == ShellContext || ctx == GeneratorContext) {
                if (maybeOp.type == Token::Colon) {
                    expr.push_back(Expression::Ptr(new LiteralExpression(":")));
                    expr.back()->begin = expr.back()->end = maybeOp;
                    continue;
                }
            }
            if (ctx == RHSContext || ctx == SubstitutionContext || ctx == ShellContext || ctx == GeneratorContext) {
                if (maybeOp.type == Token::Comma) {
                    expr.push_back(Expression::Ptr(new LiteralExpression(",")));
                    expr.back()->begin = expr.back()->end = maybeOp;
                    continue;
                }
            }
            restore(p);
        }
        restore(p);
        break;
    }
    if (expr.size()) {
        node = Expression::Ptr(new Expression);
        node->expression = expr;
        node->begin = expr.front()->begin;
        node->end   = expr.back()->end;
        return true;
    }
    return false;
}

//   ExpressionList = Expression [ Space ExpressionList ]


bool MakefileParser::expressionList(Expression::Ptr &expr, ExpressionContext ctx)
{
    int unmatchedLiteralBraces = 0;
    bool r = expressionNode(expr, ctx, &unmatchedLiteralBraces, true);

    if (!expr)
        expr = Expression::Ptr(new Expression);

    return r;
}

bool MakefileParser::callArguments(std::vector<Expression::Ptr> &args,  ExpressionContext expressionContext )
{
    for (;;) {
        Expression::Ptr arg;

        int p = save();
        maybeSpace();
        if (!expressionList(arg, expressionContext)) {
            restore(p);
            //is it an empty argument?
            if (nextToken().type == Token::Comma) {
                args.push_back(arg);
                continue;
            } else {
                restore(p);
                // last arg is empty
                int p2 = save();
                if (args.size() && nextToken().type == Token::BraceClose) {
                    args.push_back(arg);
                    restore(p2);
                }
                break;
            }
        }
        p = save();
        args.push_back(arg);
        if (nextToken().type != Token::Comma) {
            restore(p);
            break;
        }
        maybeSpace();
    }
    return args.size();
}

// $ ( Expression [ ExpressionList [ , ExpressionList ] ] )
// note that args are two dimensional. first divided by comma then by space

bool MakefileParser::callExpressionNode(Expression::Ptr &node)
{
    int begin = save();

    Token a = nextToken();
    if (a.type != Token::Expression)
        return false;
    if (nextToken().type != Token::BraceOpen)
        return false;

    Expression::Ptr arg0;
    if (!expressionNode(arg0))
        return false;

    if (!whiteSpace())
        return false;


    ExpressionContext context = CallContext;
    if (arg0->expression.size() && arg0->expression.at(0)->type == Node::LiteralExpression) {
        LiteralExpression::Ptr lArg0 = std::static_pointer_cast<LiteralExpression>(arg0->expression.at(0));
        if (lArg0->literal == "shell") {
            context = ShellContext;
        }
    }

    std::vector<Expression::Ptr> args;
    if (!callArguments(args, context))
        error("expected call arguments", save(), begin);

    Token b = nextToken();
    if (b.type != Token::BraceClose)
        error("expected ')'", save() - 1, begin);

    CallExpression *c = new CallExpression;
    c->functionName = arg0;
    c->args  = args;
    c->begin = a;
    c->end   = b;
    node = Expression::Ptr(c);
    return true;
}


bool MakefileParser::include(Node::Ptr &node)
{
    Token t = nextToken();
    if (t.type != Token::Literal)
        return false;

    bool ignoreErrors = false;
    if (!streq("include", t.begin, t.end)) {
        if (streq("-include", t.begin, t.end))
            ignoreErrors = true;
        else
            return false;
    }

    if (!whiteSpace())
        return false;

    int p = save();

    Expression::Ptr rhs;
    if (!expressionList(rhs)) {
        error("invalid arguments to include", save());
    }

    Token eol = nextToken();
    if (eol.type != Token::Eol && eol.type != Token::Eof) {
        error("expecting end of line", save());
    }

    IncludeNode *i = new IncludeNode(rhs);
    i->ignoreErrors = ignoreErrors;
    i->begin = t;
    i->end = rhs->end;
    node = Node::Ptr(i);

    return true;
}

bool MakefileParser::whiteSpace()
{
    bool had = false;
    for (;;) {
        int p = save();
        switch (nextToken().type) {
            case Token::Space:
            case Token::Tab:
            case Token::Comment:
                had = true;
                continue;
            default:
                restore(p);
                return had;
        }
    }
    return false;
}
void MakefileParser::maybeSpace()
{
    int p = save();
    if (!whiteSpace())
        restore(p);
}

bool MakefileParser::trash()
{
    switch (nextToken().type) {
        case Token::Empty:
        case Token::Comment:
        case Token::Space:
        case Token::Tab:
        case Token::Eol:
            return true;
        default:
            return false;
    };
}

bool MakefileParser::defineFunction(Node::Ptr &n)
{
    Token start = nextToken();
    if (start.type  != Token::Literal)
        return false;

    if (!streq("define", start.begin, start.end))
        return false;

    if (!whiteSpace()) {
        error("expected whitespace", save());
        return false;
    }

    Token li = nextToken();
    if (li.type  != Token::Literal) {
        error("expected function name", save());
        return false;
    }

    bool success = true;

    std::vector<Node::Ptr> code;
    Token t;
    for (;;) {
        int p = save();
        t = nextToken();
        if (t.type == Token::Eof)
            break;
        if (t.type == Token::Literal) {
            if (streq("endef", t.begin, t.end)) break;
        }
        restore(p);
        if (trash()) continue;
        restore(p);
        Node::Ptr x;
        if (globalNode(x)) {
            code.push_back(x);
            continue;
        }
        nextToken();
        success = false;
    }

    FunctionNode *f = new FunctionNode;
    f->functionName = Expression::Ptr(new LiteralExpression(
                std::string(li.begin, li.end - li.begin)));
    f->code  = code;
    f->valid = success;
    f->begin = start;
    f->end   = t;
    n = Node::Ptr(f);
    return true;
}

bool MakefileParser::globalNode(Node::Ptr &node)
{
    for (;;) {
        if (eof())
            return false;

        int p = save();

        if (trash()) continue;
        restore(p);

        if (defineFunction(node)) return true;
        restore(p);

        if (include(node)) return true;
        restore(p);

        if (assignmentNode(node)) return true;
        restore(p);

        if (ifConditionNode(node)) return true;
        restore(p);

        if (elseConditionNode(node)) return true;
        restore(p);

        if (endifConditionNode(node)) return true;
        restore(p);

        if (privateGeneratorAssignment(node)) return true;
        restore(p);

        if (generatorNode(node)) return true;
        restore(p);

        Expression::Ptr exprn;
        if (callExpressionNode(exprn)) {
            node = exprn;
            return true;
        }
#if 0
        // unfortunately this is valid, and people use it inside define :(
        {

            // if an assignment with a complex LHS doesn't get parsed correctly,
            // we end up eating the LHS here, which is very confusing during debugging
            // so lets make sure we're followed by EOL
            int pp = save();
            whiteSpace();
            if (nextToken().type != Token::Eol) {
                error("excess tokens after call expression.", save());
            }
            restore(pp);
            node = exprn;
            return true;
        }
#endif
        restore(p);

        return false;
    }
}


// token buffering



bool MakefileParser::eof()
{
    return (tpos >= tokens.size() && lexer.size() == 0);
}

Token MakefileParser::nextToken()
{
    if (eof()) {
        Token t;
        t.type = Token::Eof;
        return t;
    }

    while (tpos + 1 > tokens.size()) {
        Token t = lexer.top()->next();

        if (t.type == Token::Eof) {
            lexer.pop();

            if (eof())
                return nextToken();

            continue;
        }

        tokens.push_back(t);
    }

    if (tokens.at(tpos).type == Token::Comment) {
        ++tpos;
        return nextToken();
    }

    return tokens.at(tpos++);
}

int MakefileParser::save()
{
    return tpos;
}

void MakefileParser::restore(int i)
{
    tpos = i;
}


std::string Node::dump() const
{
    std::string m;
    switch (type)
    {
        case Expression:
            m = "Expression";
            break;
        case ReferenceExpression:
            m = "ReferenceExpression";
            break;
        case LiteralExpression:
            m = "\"" + ((::LiteralExpression*)(this))->literal + "\"" ;
            break;
        case CallExpression:
            m = "CallExpression";
            break;
        case IfCondition:
            m = "IfCondition";
            break;
        case ElseCondition:
            m = "ElseCondition";
            break;
        case EndifCondition:
            m = "EndifCondition";
            break;
        case ShellCommand:
            m = "ShellCommand";
            break;
        case Generator:
            m = "Generator";
            break;
        case Assignment:
            m = "Assignment";
            break;
        case Include:
            m = "Include";
            break;
        case Invalid:
        default:
            m = "Invalid";
            break;
    };
    return m;
}



