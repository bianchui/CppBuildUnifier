#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/semantics.hpp"
#include "../not/not.h"

NOT_MAIN;

TEST(MakefileBuiltinSemantics, Addfix)
{
    MakefileParser parser("test",
            "A = pizza pasta burger\n"
            "B = $(addprefix crappy,$(A))\n"
            "C = $(addsuffix food,$(B))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["C"]), "crappypizzafood crappypastafood crappyburgerfood");
}

TEST(MakefileBuiltinSemantics, Filter)
{
    MakefileParser parser("test",
            "A = pizza pasta cheeseburger crapburger\n"
            "B = $(filter %burger,$(A))\n"
            "C = $(filter-out crap%,$(B))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "cheeseburger crapburger");
    EXPECT_EQ(sem.context->eval(sem.context->values["C"]), "cheeseburger");
}

TEST(MakefileBuiltinSemantics, PatSubst)
{
    MakefileParser parser("test",
            "A = mooburgerpizza burgerpasta cheeseburger\n"
            "B = $(patsubst burger%, wurst%brezel, $(A))\n"
            "C = $(patsubst %,super-%,$(A))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "mooburgerpizza wurstpastabrezel cheeseburger");
    EXPECT_EQ(sem.context->eval(sem.context->values["C"]), "super-mooburgerpizza super-burgerpasta super-cheeseburger");
}

TEST(MakefileBuiltinSemantics, Words)
{
    MakefileParser parser("test",
            "A = mooburgerpizza burgerpasta cheeseburger  r\t23    \n"
            "B = $(words $(A))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "5");
}

TEST(MakefileBuiltinSemantics, Strip)
{
    MakefileParser parser("test",
            "A = saadsd     a b   nr\t23    \n"
            "B = $(strip $(A))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "saadsd a b nr 23");
}

TEST(MakefileBuiltinSemantics, Subst)
{
    MakefileParser parser("test",
            "A = a hobbit came to da haus\n"
            "B = $(subst a,b,$(A))\n"
            );
    MakefileSemantics sem(&parser);
    sem.d();

    EXPECT_EQ(sem.context->eval(sem.context->values["B"]), "b hobbit cbme to db hbus");
}
