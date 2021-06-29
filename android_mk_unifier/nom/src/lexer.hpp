#ifndef LEXER_H_sad
#define LEXER_H_sad

#include <string>
#include <stack>
#include <vector>

class Token
{
public:
    Token()
        : line(0)
        , column(0)
        , begin(0)
        , end(0)
        , fileName(0)
    {
    }
    enum Type
    {
        Eof        = 0,
        Eol        = 1,
        Empty      = 2,
        ParseError = 3,
        Comment    = 4,

        ShortExpression = 5,

        Literal    = 6,
        Operator   = 7,

        Expression = 8,
        BraceOpen  = 9,
        BraceClose = 10,
        Comma      = 11,

        Colon      = 12,
        Pipe       = 13,

        Space      = 14,
        Tab        = 15
    };
    Type type;

    const char* /*std::string*/ typeName() const;
    std::string dump() const;

    int line;
    int column;

    const char *begin;
    const char *end;

    const char *fileName;

};

class MakefileLexer
{
public:
    MakefileLexer(const std::string &fileName);
    MakefileLexer(const std::string &fileName, const char *data);
    MakefileLexer();
    ~MakefileLexer();

    Token next();
private:
    char *map;
    size_t len;
    int fd;

    const char *p;
    const char *end;
    const char *lineBegin;
    int lineCount;

    std::string fileName;
    bool accept(Token::Type type, Token &t, const char *end);
    bool acceptEol(Token &t, const char *end);

    bool nextSimple(Token &t);
    bool nextLiteral(Token &t);

    std::vector<std::string> proc;
};


#endif
