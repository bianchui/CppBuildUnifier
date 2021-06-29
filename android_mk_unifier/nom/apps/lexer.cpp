#include "../src/lexer.hpp"

#include "../not/not.h"
NOT_MAIN;

#define EXPECT_TOKEN(tok) EXPECT_EQ(tok, lexer.next().type);
#define EXPECT_LITERAL(str) {  Token t = lexer.next();   EXPECT_EQ(Token::Literal, t.type); \
    EXPECT_EQ(str, std::string(t.begin, t.end - t.begin) ); }

TEST(MakefileLexer, Assign)
{
    MakefileLexer lexer("test", "HELLO += blurp\n");

    EXPECT_LITERAL("HELLO");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("blurp");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, ReferenceExpression)
{
    MakefileLexer lexer("test", "foo = $(bar)\n");

    EXPECT_LITERAL("foo");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("bar");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}
TEST(MakefileLexer, Dollars)
{
    MakefileLexer lexer("test", "a=$Ab$$)");

    EXPECT_LITERAL("a");
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::ShortExpression);
    EXPECT_LITERAL("b$");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, CallExpression)
{
    MakefileLexer lexer("test", "foo = $(call me , maybe)\n");

    EXPECT_LITERAL("foo");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("call");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("me");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Comma);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("maybe");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, Generator)
{
    MakefileLexer lexer("test", "ihoor: has dependencies\n\tstuff\n\tcd bla; goto hell\n");

    EXPECT_LITERAL("ihoor");
    EXPECT_TOKEN(Token::Colon);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("has");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("dependencies");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Tab);
    EXPECT_LITERAL("stuff");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Tab);
    EXPECT_LITERAL("cd");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("bla;");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("goto");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("hell");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, ConditionalGenerator)
{
    MakefileLexer lexer("test",
            "$(a): has dependencies\n"
            "ifeq (a,b )\n"
            "\tstuff $@\n"
            "endif\n"
    );

    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Colon);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Eol);

    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Comma);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Eol);

    EXPECT_TOKEN(Token::Tab);
    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::ShortExpression);
    EXPECT_TOKEN(Token::Eol);

    EXPECT_TOKEN(Token::Literal);
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}
TEST(MakefileLexer, EscapedLiteral)
{
    MakefileLexer lexer("test",
            "foo = \\#ab\\\"c \"df\\\"g\" \"\" \\\n"
            "  i j 'duh\\' \\$ \\\\duh'\n"
    );

    EXPECT_LITERAL("foo");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("#ab\"c");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("df\"g");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("i");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("j");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("'duh'");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("$");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("\\duh'");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, QuotedLiteral)
{
    MakefileLexer lexer("test",
            "a=d\"\\#define STAGE2_SIZE if `stat -c '%s' $<` > $@ |}{}#P{\" second\n"
    );

    EXPECT_LITERAL("a");
    EXPECT_TOKEN(Token::Operator);
    EXPECT_LITERAL("#define STAGE2_SIZE if `stat -c '%s' $<` > $@ |}{}#P{");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("second");
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, LineContinuation)
{
    MakefileLexer lexer("test",
            "HELLO \\\n"
            "= $(mega blurp $(and $(more,  \\\n"
            "  ))\\\n"
            ") \\\n"
            "more \n"
    );

    EXPECT_LITERAL("HELLO");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Operator);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("mega");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("blurp");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("and");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("more");
    EXPECT_TOKEN(Token::Comma);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("more");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Eol);
    EXPECT_TOKEN(Token::Eof);
}

TEST(MakefileLexer, ShellCall)
{
    MakefileLexer lexer("test", "$(shell cd $(L) && ls */$(1)/{a,b} 2> /dev/null ?a)");


    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("shell");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("cd");
    EXPECT_TOKEN(Token::Space);
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("L");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("&&");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("ls");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("*/");
    EXPECT_TOKEN(Token::Expression);
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("1");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_LITERAL("/");

    //TODO: not entirely sure if this should be a seperate type
    EXPECT_TOKEN(Token::BraceOpen);
    EXPECT_LITERAL("a");
    EXPECT_TOKEN(Token::Comma);
    EXPECT_LITERAL("b");
    EXPECT_TOKEN(Token::BraceClose);
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("2>");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("/dev/null");
    EXPECT_TOKEN(Token::Space);
    EXPECT_LITERAL("?");
    EXPECT_LITERAL("a");
    EXPECT_TOKEN(Token::BraceClose);

    EXPECT_TOKEN(Token::Eof);
}


