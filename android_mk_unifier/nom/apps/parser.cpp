#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../not/not.h"

NOT_MAIN;


#define E  Node::Ptr next;

#define EXPECT_ASSIGN(node, name) EXPECT_EQ(true, !!node); EXPECT_EQ(Node::Assignment, node->type); \
    Assignment::Ptr name  = std::static_pointer_cast<Assignment>(node);

#define EXPECT_CALL(node, name)  EXPECT_EQ(true, !!node); EXPECT_EQ(Node::CallExpression, node->type); \
    CallExpression::Ptr name  = std::static_pointer_cast<CallExpression>(node);

#define EXPECT_FUNCTION(node, name)  EXPECT_EQ(true, !!node); EXPECT_EQ(Node::Function, node->type); \
    FunctionNode::Ptr name  = std::static_pointer_cast<FunctionNode>(node);

#define EXPECT_GENERATOR(node, name)  EXPECT_EQ(true, !!node); EXPECT_EQ(Node::Generator, node->type); \
    Generator::Ptr name  = std::static_pointer_cast<Generator>(node);

#define EXPECT_INCLUDE(node, name)  EXPECT_EQ(true, !!node); EXPECT_EQ(Node::Include, node->type); \
    IncludeNode::Ptr name  = std::static_pointer_cast<IncludeNode>(node);

#define EXPECT_IF(node, name)  EXPECT_EQ(true, !!node);  EXPECT_EQ(Node::IfCondition , node->type); \
    IfCondition::Ptr name  = std::static_pointer_cast<IfCondition>(node);

#define EXPECT_LITERAL(node, str)  EXPECT_EQ(true, !!node);  EXPECT_EQ(Node::LiteralExpression, node->type); { \
    if (Node::LiteralExpression == node->type) { \
    LiteralExpression::Ptr name  = std::static_pointer_cast<LiteralExpression>(node); \
    EXPECT_EQ(str, name->literal); \
    }\
}
#define EXPECT_LITERAL_REFERENCE(node, str)    EXPECT_EQ(Node::ReferenceExpression, node->type); { \
    if (Node::ReferenceExpression == node->type) { \
        ReferenceExpression::Ptr referenceExpression  = std::static_pointer_cast<ReferenceExpression>(node); \
    if (Node::LiteralExpression == referenceExpression->reference->type) { \
        LiteralExpression::Ptr literal = std::static_pointer_cast<LiteralExpression>(referenceExpression->reference); \
        EXPECT_EQ(str, literal->literal); \
    } else if  (referenceExpression->reference->expression.size() &&  Node::LiteralExpression == referenceExpression->reference->expression.at(0)->type) { \
        LiteralExpression::Ptr literal = std::static_pointer_cast<LiteralExpression>(referenceExpression->reference->expression.at(0)); \
        EXPECT_EQ(str, literal->literal); \
    } else { \
    EXPECT_EQ(Node::LiteralExpression, referenceExpression->reference->type); \
    }}}


#define EXPECT_EOF    EXPECT_EQ(false, !!parser.next());


TEST(MakefileParser, Assign)
{
    MakefileParser parser("test", "HELLO += blurp\n");

    Node::Ptr next = parser.next();

    EXPECT_ASSIGN(next, assignment);

    EXPECT_EQ(assignment->op, "+=");

    EXPECT_LITERAL(assignment->lhs->expression.at(0), "HELLO");
    EXPECT_LITERAL(assignment->rhs->expression.at(0), "blurp");

    EXPECT_EOF;
}



TEST(MakefileParser, Call)
{
    MakefileParser parser("test", "a=$(call $(me),,$(maybe)  not)\n");

    Node::Ptr next = parser.next();

    EXPECT_ASSIGN(next, assignment);

    EXPECT_LITERAL(assignment->lhs->expression.at(0), "a");

    EXPECT_CALL(assignment->rhs->expression.at(0), call);
    EXPECT_LITERAL(call->functionName->expression.at(0), "call");
    EXPECT_EQ(call->args.size(), 3);
    EXPECT_LITERAL_REFERENCE(call->args.at(0)->expression.at(0), "me");
    EXPECT_LITERAL_REFERENCE(call->args.at(2)->expression.at(0), "maybe");
    EXPECT_LITERAL(call->args.at(2)->expression.at(1), "  ");
    EXPECT_LITERAL(call->args.at(2)->expression.at(2), "not");

    EXPECT_EOF;
}


TEST(MakefileParser, Conditional)
{
    MakefileParser parser("test",
            "ifeq (a,b c)\n"
            "\ta=b\n"
            "else"
            "\ta=\\\n"
            "\t  c\n"
            "endif\n"
            "ifdef mops\n"
            "\tv=a\n"
            "endif\n"
            "ifeq \"\" b\n"
            "\tb=d\n"
            "endif\n"
            );

    Node::Ptr next = parser.next();

    EXPECT_IF(next, iff);
    EXPECT_EQ(iff->args.size(), 2);
    EXPECT_LITERAL(iff->args.at(0)->expression.at(0), "a");
    EXPECT_EQ(iff->args.at(1)->expression.size(), 3);
    EXPECT_LITERAL(iff->args.at(1)->expression.at(0), "b");
    EXPECT_LITERAL(iff->args.at(1)->expression.at(1), " ");
    EXPECT_LITERAL(iff->args.at(1)->expression.at(2), "c");

    //those should probably be inside the if node instead or something?
    next = parser.next();
    EXPECT_ASSIGN(next, assign1);
}


TEST(MakefileParser, QuotedLiteral)
{
    MakefileParser parser("test",
            "a=\"^%$^&*())_)(AS_(\\@ @ $< >$\"b  other\n"
            );

    Node::Ptr next = parser.next();


    EXPECT_ASSIGN(next, assign);

    EXPECT_LITERAL(assign->rhs->expression.at(0), "^%$^&*())_)(AS_(@ @ $< >$");
    EXPECT_LITERAL(assign->rhs->expression.at(1), "b");
    EXPECT_LITERAL(assign->rhs->expression.at(2), "  ");
    EXPECT_LITERAL(assign->rhs->expression.at(3), "other");

    EXPECT_EOF;
}

TEST(MakefileParser, TabSucks)
{
    MakefileParser parser("test",
            "a=stuff\tmore\n"
            );

    Node::Ptr next = parser.next();


    EXPECT_ASSIGN(next, assign);

    EXPECT_LITERAL(assign->rhs->expression.at(0), "stuff");

    EXPECT_EOF;
}

TEST(MakefileParser, ContinueLine)
{
    MakefileParser parser("test",
            "A=A\\\n"
            "\tB\\\n"
            " C\n"
            );

    Node::Ptr next = parser.next();


    EXPECT_ASSIGN(next, assign);

    EXPECT_EQ(assign->rhs->expression.size(),7);
    EXPECT_LITERAL(assign->rhs->expression.at(0), "A");
    EXPECT_LITERAL(assign->rhs->expression.at(1), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(2), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(3), "B");

    EXPECT_EOF;
}

TEST(MakefileParser, ExpandPatsubst)
{
    MakefileParser parser("test","c += $(a:%=$(b)/-Wl,-framework,%)\n");

    Node::Ptr next = parser.next();
    EXPECT_ASSIGN(next, assign);

    EXPECT_CALL(assign->rhs->expression.at(0), call);
    EXPECT_LITERAL(call->functionName, "patsubst");
    EXPECT_EQ(call->args.size(), 3);
    EXPECT_EQ(call->args.at(0)->expression.size(), 1);
    EXPECT_LITERAL(call->args.at(0)->expression.at(0), "%");
    EXPECT_EQ(call->args.at(1)->expression.size(), 6);
    EXPECT_LITERAL_REFERENCE(call->args.at(1)->expression.at(0), "b");
    EXPECT_LITERAL(call->args.at(1)->expression.at(1), "/-Wl");
    EXPECT_LITERAL(call->args.at(1)->expression.at(2), ",");
    EXPECT_LITERAL(call->args.at(1)->expression.at(3), "-framework");
    EXPECT_LITERAL(call->args.at(1)->expression.at(4), ",");
    EXPECT_LITERAL(call->args.at(1)->expression.at(5), "%");

    EXPECT_LITERAL_REFERENCE(call->args.at(2), "a");

    EXPECT_EOF;
}


TEST(MakefileParser, RhsContext)
{
    MakefileParser parser("test","a=&a |b ,f literal (d ':bla' : $$ -Dpopel=mo++ , same arglist\n");
    Node::Ptr next = parser.next();
    EXPECT_ASSIGN(next, assign);

    EXPECT_LITERAL(assign->rhs->expression.at(0), "&a");
    EXPECT_LITERAL(assign->rhs->expression.at(1), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(2), "|");
    EXPECT_LITERAL(assign->rhs->expression.at(3), "b");
    EXPECT_LITERAL(assign->rhs->expression.at(4), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(5), ",");
    EXPECT_LITERAL(assign->rhs->expression.at(6), "f");
    EXPECT_LITERAL(assign->rhs->expression.at(7), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(8), "literal");
    EXPECT_LITERAL(assign->rhs->expression.at(9), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(10), "(");
    EXPECT_LITERAL(assign->rhs->expression.at(11), "d");
    EXPECT_LITERAL(assign->rhs->expression.at(12), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(13), "'");
    EXPECT_LITERAL(assign->rhs->expression.at(14), ":");
    EXPECT_LITERAL(assign->rhs->expression.at(15), "bla'");
    EXPECT_LITERAL(assign->rhs->expression.at(16), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(17), ":");
    EXPECT_LITERAL(assign->rhs->expression.at(18), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(19), "$");
    EXPECT_LITERAL(assign->rhs->expression.at(20), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(21), "-Dpopel");
    EXPECT_LITERAL(assign->rhs->expression.at(22), "=");
    EXPECT_LITERAL(assign->rhs->expression.at(23), "mo++");
    EXPECT_LITERAL(assign->rhs->expression.at(24), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(25), ",");
    EXPECT_LITERAL(assign->rhs->expression.at(26), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(27), "same");
    EXPECT_LITERAL(assign->rhs->expression.at(28), " ");
    EXPECT_LITERAL(assign->rhs->expression.at(29), "arglist");

    EXPECT_EOF;
}


TEST(MakefileParser, CallContext)
{
    MakefileParser parser("test","$(call &a |b literal (d ':bla'$$) : , second arglist  )\n");
    Node::Ptr next = parser.next();

    EXPECT_CALL(next, call);
    EXPECT_LITERAL(call->args.at(0)->expression.at(0), "&a");
    EXPECT_LITERAL(call->args.at(0)->expression.at(1), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(2), "|");
    EXPECT_LITERAL(call->args.at(0)->expression.at(3), "b");
    EXPECT_LITERAL(call->args.at(0)->expression.at(4), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(5), "literal");
    EXPECT_LITERAL(call->args.at(0)->expression.at(6), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(7), "(");
    EXPECT_LITERAL(call->args.at(0)->expression.at(8), "d");
    EXPECT_LITERAL(call->args.at(0)->expression.at(9), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(10), "'");
    EXPECT_LITERAL(call->args.at(0)->expression.at(11), ":");
    EXPECT_LITERAL(call->args.at(0)->expression.at(12), "bla'$");
    EXPECT_LITERAL(call->args.at(0)->expression.at(13), ")");
    EXPECT_LITERAL(call->args.at(0)->expression.at(14), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(15), ":");
    EXPECT_LITERAL(call->args.at(1)->expression.at(0), "second");
    EXPECT_LITERAL(call->args.at(1)->expression.at(1), " ");
    EXPECT_LITERAL(call->args.at(1)->expression.at(2), "arglist");

    EXPECT_EOF;
}

TEST(MakefileParser, SubsitutionContext)
{
    MakefileParser parser("test","a=$(c:&a|b,fliterald'bla'$$-Dpopelmo++,samearglis=replacement)\n");
    Node::Ptr next = parser.next();

    EXPECT_ASSIGN(next, assign);
    EXPECT_CALL(assign->rhs->expression.at(0), call);
    EXPECT_LITERAL(call->args.at(0)->expression.at(0), "&a|b");
    EXPECT_LITERAL(call->args.at(0)->expression.at(1), ",");
    EXPECT_LITERAL(call->args.at(0)->expression.at(2), "fliterald'bla'$");
    EXPECT_LITERAL(call->args.at(0)->expression.at(3), "-Dpopelmo++");
    EXPECT_LITERAL(call->args.at(0)->expression.at(4), ",");
    EXPECT_LITERAL(call->args.at(0)->expression.at(5), "samearglis");
    EXPECT_LITERAL(call->args.at(1)->expression.at(0), "replacement");
    EXPECT_LITERAL_REFERENCE(call->args.at(2), "c");

    EXPECT_EOF;
}

TEST(MakefileParser, DefineFunction)
{
    MakefileParser parser("test",
            "define blurp\n"
            "a:b\n"
            "\tgcc kot\n"
            "endef\n"
            );
    Node::Ptr next = parser.next();

    EXPECT_FUNCTION(next, f);
    EXPECT_EQ(true, f->valid);
    EXPECT_LITERAL(f->functionName, "blurp");
    EXPECT_EQ(f->code.size(), 1);
    EXPECT_GENERATOR(f->code.at(0), gen);
    EXPECT_LITERAL(gen->lhs->expression.at(0), "a");
    EXPECT_LITERAL(gen->rhs->expression.at(0), "b");
    EXPECT_LITERAL(gen->rhs->expression.at(0), "b");
    // TODO: gen->code not parsed yet


    EXPECT_EOF;
}

TEST(MakefileParser, DefineInvalidFunction)
{
    MakefileParser parser("test",
            "define wop\n"
            "\twop\n"
            "endef\n"
            );
    Node::Ptr next = parser.next();

    EXPECT_FUNCTION(next, f);
    EXPECT_EQ(false, f->valid);

    EXPECT_EOF;
}

TEST(MakefileParser, Include)
{
    MakefileParser parser("test",
            "-include $(invalid)file $(TEST)\n"
            "include $(TEST)\n"
            );
    Node::Ptr next = parser.next();

    EXPECT_INCLUDE(next, i1);
    EXPECT_EQ(true, i1->ignoreErrors);
    EXPECT_EQ(4, i1->expr->expression.size());

    next = parser.next();
    EXPECT_INCLUDE(next, i2);
    EXPECT_EQ(false, i2->ignoreErrors);
    EXPECT_EQ(1, i2->expr->expression.size());

    EXPECT_EOF;
}

TEST(MakefileParser, IncludeMissingEol)
{
    MakefileParser parser("test","include $(TEST)");
    Node::Ptr next = parser.next();

    EXPECT_INCLUDE(next, i2);
    EXPECT_EQ(false, i2->ignoreErrors);
    EXPECT_EQ(1, i2->expr->expression.size());

    EXPECT_EOF;
}

TEST(MakefileParser, NestedReference)
{
    MakefileParser parser("test","A.$(B).C:=$(A.$(M).B) $(D) literal\n");

    Node::Ptr next = parser.next();
    EXPECT_ASSIGN(next, assign);

    EXPECT_LITERAL(assign->lhs->expression.at(0), "A.");
    EXPECT_LITERAL_REFERENCE(assign->lhs->expression.at(1), "B");
    EXPECT_LITERAL(assign->lhs->expression.at(2), ".C");


    EXPECT_EQ(Node::ReferenceExpression, assign->rhs->expression.at(0)->type);
    ReferenceExpression::Ptr rref = std::static_pointer_cast<ReferenceExpression>(assign->rhs->expression.at(0));

    EXPECT_LITERAL(rref->reference->expression.at(0), "A.");
    EXPECT_LITERAL_REFERENCE(rref->reference->expression.at(1), "M");
    EXPECT_LITERAL(rref->reference->expression.at(2), ".B");


    EXPECT_LITERAL_REFERENCE(assign->rhs->expression.at(2), "D");
    EXPECT_LITERAL(assign->rhs->expression.at(4), "literal");

    EXPECT_EOF;
}


TEST(MakefileParser, Obscure)
{
    MakefileParser parser("test",
            "NOTHING := \n"
            "SPACE := $(NOTHING) $(NOTHING)\n"
            "SPACESHIP:= BIG$(SPACE)SHIP\n"
            );

    Node::Ptr next = parser.next();
    EXPECT_ASSIGN(next, assign1);

    EXPECT_LITERAL(assign1->lhs->expression.at(0), "NOTHING");
    EXPECT_EQ(assign1->rhs->expression.size(), 0);

    next = parser.next();
    EXPECT_ASSIGN(next, assign2);

    EXPECT_EQ                (assign2->rhs->expression.size(), 3);
    EXPECT_LITERAL_REFERENCE (assign2->rhs->expression.at(0), "NOTHING");
    EXPECT_LITERAL           (assign2->rhs->expression.at(1), " ");
    EXPECT_LITERAL_REFERENCE (assign2->rhs->expression.at(2), "NOTHING");

    next = parser.next();
    EXPECT_ASSIGN(next, assign3);

    EXPECT_EQ                (assign3->rhs->expression.size(), 3);
    EXPECT_LITERAL           (assign3->rhs->expression.at(0), "BIG");
    EXPECT_LITERAL_REFERENCE (assign3->rhs->expression.at(1), "SPACE");
    EXPECT_LITERAL           (assign3->rhs->expression.at(2), "SHIP");
    EXPECT_EOF;
}

TEST(MakefileParser, TopLevelExpressions)
{
    MakefileParser parser("test",
            "$(info blurp  ( ilike 2 party party ) )\n"
            "$(call __ndk_info,You might want to use $$NDK/build/tools/build-stlport.sh)\n"
            );


    Node::Ptr next = parser.next();
    EXPECT_CALL(next, call1);

    next = parser.next();
    EXPECT_CALL(next, call2);


    EXPECT_EOF;
}


TEST(MakefileParser, Generator)
{
    MakefileParser parser("test", "ihoor: has dependencies | order\n\tstuff\n\tcd bla; goto hell\n");


    Node::Ptr next = parser.next();

    EXPECT_GENERATOR(next, gen);
    EXPECT_LITERAL(gen->lhs->expression.at(0), "ihoor");
    EXPECT_LITERAL(gen->rhs->expression.at(0), "has");
    EXPECT_LITERAL(gen->rhs->expression.at(1), " ");
    EXPECT_LITERAL(gen->rhs->expression.at(2), "dependencies");
    EXPECT_LITERAL(gen->orderonly->expression.at(0), "order");

    //FIXME: code is still useless

    EXPECT_EOF;
}



TEST(MakefileParser, NotAShellCommand)
{
    MakefileParser parser("test", "ihoor:\nifeq (a , b)\n");

    Node::Ptr next = parser.next();

    EXPECT_GENERATOR(next, gen);
    EXPECT_LITERAL(gen->lhs->expression.at(0), "ihoor");
    EXPECT_EQ(gen->rhs->expression.size(), 0);

    next = parser.next();
    EXPECT_IF(next, iff);


    EXPECT_EOF;
}

TEST(MakefileParser, GeneratorComment)
{
    MakefileParser parser("test", "ihoor: has dependencies | order\n\t#stuff\n\tcd bla; goto hell\n");
    Node::Ptr next = parser.next();

    EXPECT_GENERATOR(next, gen);
    EXPECT_LITERAL(gen->lhs->expression.at(0), "ihoor");
    EXPECT_LITERAL(gen->rhs->expression.at(0), "has");
    EXPECT_LITERAL(gen->rhs->expression.at(1), " ");
    EXPECT_LITERAL(gen->rhs->expression.at(2), "dependencies");
    EXPECT_LITERAL(gen->orderonly->expression.at(0), "order");

    EXPECT_EOF;
}

TEST(MakefileParser, ShellCall)
{
    MakefileParser parser("test", "$(shell cd $(L) && ls -d */$(1)/{a,b} 2> /dev/null ?a)");

    Node::Ptr next = parser.next();

    EXPECT_CALL(next, call);
    EXPECT_LITERAL(call->functionName->expression.at(0), "shell");
    EXPECT_EQ(call->args.size(), 1);
    EXPECT_EQ(call->args.at(0)->expression.size(), 25);
    EXPECT_LITERAL(call->args.at(0)->expression.at(0), "cd");
    EXPECT_LITERAL(call->args.at(0)->expression.at(1), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(4), "&&");
    EXPECT_LITERAL(call->args.at(0)->expression.at(5), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(6), "ls");
    EXPECT_LITERAL(call->args.at(0)->expression.at(7), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(8), "-d");
    EXPECT_LITERAL(call->args.at(0)->expression.at(9), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(10), "*/");
    EXPECT_LITERAL_REFERENCE(call->args.at(0)->expression.at(11), "1");
    EXPECT_LITERAL(call->args.at(0)->expression.at(12), "/");
    EXPECT_LITERAL(call->args.at(0)->expression.at(13), "{");
    EXPECT_LITERAL(call->args.at(0)->expression.at(14), "a");
    EXPECT_LITERAL(call->args.at(0)->expression.at(15), ",");
    EXPECT_LITERAL(call->args.at(0)->expression.at(16), "b");
    EXPECT_LITERAL(call->args.at(0)->expression.at(17), "}");
    EXPECT_LITERAL(call->args.at(0)->expression.at(18), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(19), "2>");
    EXPECT_LITERAL(call->args.at(0)->expression.at(21), "/dev/null");
    EXPECT_LITERAL(call->args.at(0)->expression.at(22), " ");
    EXPECT_LITERAL(call->args.at(0)->expression.at(23), "?");
    EXPECT_LITERAL(call->args.at(0)->expression.at(24), "a");

    EXPECT_EOF;
}


TEST(MakefileParser, DefineMultiLineEvil)
{
    MakefileParser parser("test",
            "define test\n"
            "$(eval a=1)\\\n"
            "$(eval b=$(a))\n"
            "$(eval c=$(b))\n"
            "endef\n"
            );

    Node::Ptr next = parser.next();

    EXPECT_FUNCTION(next, f);
    EXPECT_EQ(true, f->valid);
    EXPECT_LITERAL(f->functionName, "test");
    EXPECT_EQ(f->code.size(), 3);
    EXPECT_CALL(f->code.at(0), e1);
    EXPECT_CALL(f->code.at(1), e2);
    EXPECT_CALL(f->code.at(2), e3);

    EXPECT_EOF;
}

