#include "lexer.hpp"
#include "utils.hpp"
#include "error.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#if __cpp_exceptions
MakefileLexer::MakefileLexer(const std::string &fileName)
    : fileName(fileName)
    , lineCount(1)
{
    char *rp = realpath(fileName.c_str(), 0);
    if (!rp)
        throw Error("cannot resolve file " + fileName);
    this->fileName = std::string(rp);
    free (rp);


    fd = ::open(fileName.c_str(), O_RDONLY);
    if (!fd)
        throw Error("cannot open file " + fileName);

    struct stat st;
    if (fstat(fd, &st)) {
        throw Error("stat failed " + fileName);
    }
    len = st.st_size;

    map = (char*)mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (!map)
        throw Error("cannot map file ");


    p = map;
    end = p + len;
    lineBegin = p;
}
#endif//__cpp_exceptions

MakefileLexer::MakefileLexer(const std::string &fileName, const char *data)
    : fileName(fileName)
    , lineCount(1)
    , fd(0)
{

    map = (char *)data;
    len = strlen(data);

    p = map;
    end = p + len;
    lineBegin = p;
}

MakefileLexer::~MakefileLexer()
{
    if (fd) {
        munmap(map, len);
        close(fd);
    }
}

bool MakefileLexer::accept(Token::Type type, Token &t, const char *end) {
    t.type     = type;
    t.fileName = fileName.c_str();
    t.line     = lineCount;

    t.begin    = p;
    t.end      = end;
    t.column   = (int)(p - lineBegin) + 1;

    p = end;
    return true;
}


bool MakefileLexer::nextLiteral(Token &t)
{
    std::vector <const char *> erase;

    const char *p =  this->p;
    const char *d  = this->p;
    const char *dx = 0;

    bool emptyOk = false;
    while (p < end) {
        switch (*p) {
            case '\n':
                goto end_literal;
                break;
            case '\\':
                if (++p < end && *p == '\n') {
                    --p;
                    goto end_literal;
                }
                erase.push_back(p);
                ++p;
                break;
            case '#':
                goto end_literal;
                break;
            case '"':
            case '`': {
                char e = *p;
                d = p++;
                emptyOk = true;
                for (;;) {
                    if (p >= end)
                        return false;

                    if (*p == '\\') {
                        // BC: do not erase
                        //erase.push_back(p + 1 /* +1 because we skipped the initial " char */);
                        if (p < end) {
                            ++p;
                        }
                    } else if (*p == e) {
                        dx = ++p;
                        goto end_literal;
                        break;
                    }
                    ++p;
                }
                break;
            }
            case '$':
                if (++p <= end && *p == '$') {
                    dx = p++;
                    goto end_literal;
                }
                --p;
            case '?':
            case ':':
            case '=':
            case ',':
            case '}':
            case '{':
            case ')':
            case '(':
            case '\t':
            case ' ':
                goto end_literal;
                break;
            // this is here to remind myself that single quote
            // is considered alphanumeric in make
            case '\'':
            default:
                ++p;
                continue;
        }
    }
end_literal:
    if (p > this->p || emptyOk) {
        accept(Token::Literal, t, p);
        t.begin = d;
        if (dx)
            t.end = dx;

        if (erase.size()) {
            std::string e(t.begin, t.end - t.begin);
            int removed = 0;
            for (auto i = erase.begin(); i != erase.end(); i++) {
                size_t pos = *i - t.begin - 1 - removed++;
                e.erase(pos, 1);
            }
            proc.push_back(e);
            t.begin = proc.back().c_str();
            t.end   = proc.back().c_str() +  proc.back().size();
        }

        return true;
    } else {
        return false;
    }
    return false;
}


bool MakefileLexer::nextSimple(Token &t)
{
    const char *p = this->p;
    char c;

    switch (*p++) {
        case ':':
            if (p < end && *p == '=')
                return accept(Token::Operator, t, p + 1);
            return accept(Token::Colon, t, p);

        case '?':
            if (p < end && *p == '=')
                return accept(Token::Operator, t, p + 1);
            else
                return accept(Token::Literal, t, p);


            return false;

        case '+':
            if (p < end && *p == '=')
                return accept(Token::Operator, t, p + 1);
            return false;

        case '=':
            return accept(Token::Operator, t, p);

        case '|':
            return accept(Token::Pipe,  t, p);

        case ',':
            return accept(Token::Comma, t, p);



        case '(':
        case '{':
            return accept(Token::BraceOpen, t, p);

        case ')':
        case '}':
            return accept(Token::BraceClose, t, p);

        case '$':
            if (p >= end) return false;
            c = *p;
            if (c == '$') {
                accept(Token::Literal, t, p + 1);
                t.end--;
                return true;
            } else if (c == '{' || c == '(') {
                return accept(Token::Expression, t, p);
            } else if (
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    (c == '@') ||
                    (c == '?') ||
                    (c == '^') ||
                    (c == '>') ||
                    (c == '<') ||
                    (c == '*') ||
                    (c == '\'') ||
                    (c == '!'))
            {
                accept(Token::ShortExpression, t, p + 1);
                t.begin++;
                return true;

            } else {
                return false;
            }
        case '\t':
            while (p < end && *p == '\t') { ++p; }
            return accept(Token::Tab, t, p);

        case ' ':
            while (p < end && *p == ' ') { ++p; }
            return accept(Token::Space, t, p);

        case '#':
            if (true) {
                bool escape = false;
                int newLineCount = lineCount;
                const char *newLinBegin = lineBegin;
                while (p < end) {
                    if (escape) {
                        if (*p == '\n') {
                            ++newLineCount;
                            newLinBegin = p + 1;
                        }
                        escape = false;
                    } else {
                        if (*p == '\\') {
                            escape = true;
                        }
                        if (*p == '\n') {
                            break;
                        }
                    }
                    ++p;
                }
                accept(Token::Comment, t, p);
                
                lineCount = newLineCount;
                lineBegin = newLinBegin;
                
                return true;
            }

        // line continuation means space.
        case '\\':
            if (p < end && *p == '\n') {
                accept(Token::Space, t, p + 1);
                lineBegin = p + 1;
                ++lineCount;
                return true;
            }
            return false;
        case '\n':
            return acceptEol(t, p);

        default:
            return false;
    };
    return false;
}


bool MakefileLexer::acceptEol(Token &t, const char *end)
{
    accept(Token::Eol, t, end);
    lineBegin = end;
    ++lineCount;
    return true;
}

Token MakefileLexer::next()
{
    Token t;

    if (p >= end) {
        t.type = Token::Eof;
        return t;
    }

    if (nextSimple(t))          return t;
    if (nextLiteral(t))         return t;

    accept(Token::ParseError, t, p + 1);
    return t;
}

const char* /*std::string*/ Token::typeName() const
{
    switch (type) {
        case Token::Eof:
            return"Eof ";
        case Token::Eol:
            return"Eol ";
        case Token::Empty:
            return"Empty ";
        case Token::ParseError:
            return"ParseError ";
        case Token::Comment:
            return"Comment ";
        case Token::Literal:
            return"Literal ";
        case Token::Operator:
            return"Operator ";
        case Token::Expression:
            return"Expression ";
        case Token::Tab:
            return"Tab";
        case Token::Pipe:
            return"Pipe ";
        case Token::Colon:
            return"Colon ";
        case Token::Comma:
            return"Comma ";
        case Token::BraceOpen:
            return"BraceOpen ";
        case Token::BraceClose:
            return"BraceClose ";
        case Token::Space:
            return"Space ";
        case Token::ShortExpression:
            return"ShortExpression";
        default:
            return "???";
    }
}
#if 0
std::string Token::dump() const
{
    std::stringstream oss;

    if (fileName)
        oss << fileName;
    else
        oss << "??";

    oss << " :" << line     << ":" << column   << ":"
        << typeName() << std::endl
        << std::string(begin, end - begin) << std::endl
    ;

    return oss.str();
}
#endif//0
