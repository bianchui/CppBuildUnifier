#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/semantics.hpp"
#include "../not/not.h"

NOT_MAIN;

TEST(MakefileSemantics, Assign)
{
    MakefileParser parser("test", "HELLO = Hello\nHELLO += World");
    MakefileSemantics sem(&parser);

    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["HELLO"]), "Hello World");
}

TEST(MakefileSemantics, EmptyStringCrazyness)
{
    // this just tests if it behaves identical to gnumake, not if it makes any sense
    MakefileParser parser("test",
            "A =\n"
            "A +=\n"
            "B = \n"
            "C := $(A) $(A)\n"
            "D := $(C)\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["A"]), "");
    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "");
    EXPECT_EQ(sem.context->eval(sem.context->values["C"]), " ");
    EXPECT_EQ(sem.context->eval(sem.context->values["D"]), " ");
}

TEST(MakefileSemantics, AssignCrazyness)
{
    // this just tests if it behaves identical to gnumake, not if it makes any sense
    MakefileParser parser("test",
            "A =\n"
            "A +=\n"
            "B = \n"
            "B += $(A)\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["A"]), "");
    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), " ");
}

TEST(MakefileSemantics, Ifdef)
{
    MakefileParser parser("test",
            "S1=f\nS2=f\nS3=f\nS4=f\nS5=f\n"
            "ifdef S1\n"
            "S1=t\n"
            "endif\n"
            "ifndef S2\n"
            "S2=t\n"
            "endif\n"
            "ifdef X\n"
            "S3=t\n"
            "endif\n"
            "ifndef X\n"
            "S4=t\n"
            "endif\n"
            "Y=i\n"
            "ifndef $(Y)\n"
            "S5=t\n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["S1"]), "t");
    EXPECT_EQ(sem.context->eval(sem.context->values["S2"]), "f");
    EXPECT_EQ(sem.context->eval(sem.context->values["S3"]), "f");
    EXPECT_EQ(sem.context->eval(sem.context->values["S4"]), "t");
    EXPECT_EQ(sem.context->eval(sem.context->values["S5"]), "t");
}

TEST(MakefileSemantics, IfFail)
{
    MakefileParser parser("test",
            "S=f\n"
            "ifeq (,b)\n"
            "S=t\n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["S"]), "f");
}

TEST(MakefileSemantics, IfPass)
{
    MakefileParser parser("test",
            "b=\n"
            "S=f\n"
            "ifeq (,$b)\n"
            "S=t\n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["S"]), "t");
}

TEST(MakefileSemantics, SpaceNotEqualsEmpty)
{
    MakefileParser parser("test",
            "A=\n"
            "B=$(A) $(A)\n"
            "S=f\n"
            "ifeq (,$(B))\n"
            "S=t\n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), " ");
    EXPECT_EQ(sem.context->eval(sem.context->values["S"]), "f");
}

TEST(MakefileSemantics, IfMeme)
{
    MakefileParser parser("test",
            "TARGET_BUILD_APPS=\n"
            "ifeq ($(TARGET_BUILD_APPS),)\n"
            "LOCAL_PATH:= d\\\n"
            "\t d \n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
}

TEST(MakefileSemantics, Undefined)
{
    MakefileParser parser("test",
            "A=\n"
            "S=f\n"
            "ifdef A\n"
            "S=t\n"
            "endif\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();
    EXPECT_EQ(sem.context->eval(sem.context->values["S"]), "f");
}
