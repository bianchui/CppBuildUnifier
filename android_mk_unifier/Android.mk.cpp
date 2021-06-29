// Copyright (c) 2018-2021 bianchui https://github.com/bianchui
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Android.mk.h"
#include <vector>
#include <sstream>
#include "nom/src/lexer.hpp"
#include "shared/SSources.h"

std::string AndroidMk_replace_LOCAL_SRC_FILES(const std::string& mk, const std::string& newSrc) {
    std::ostringstream os;
    
    MakefileLexer l("test", mk.c_str());
    
    const std::string LOCAL_SRC_FILES("LOCAL_SRC_FILES");
    
    std::string assignName;
    Token::Type expectToken = Token::Literal;
    bool isMatching = false;
    bool firstMatching = true;
    bool running = true;
    
    while (running) {
        Token token = l.next();
        LOG_T("Token: %s: %d %.*s\n", token.typeName(), (int)(token.end - token.begin), (int)(token.end - token.begin), token.begin);
        
        switch (token.type) {
            case Token::ParseError:
                return "";
                
            case Token::Eof:
                running = false;
                break;
                
            case Token::Empty:
            case Token::Comment:
            case Token::Space:
            case Token::Tab:
                break;
                
            case Token::Eol:
                expectToken = Token::Literal;
                if (isMatching) {
                    firstMatching = false;
                    isMatching = false;
                    assignName.clear();
                }
                break;
                
            case Token::Literal:
                if (expectToken == Token::Literal) {
                    assignName.assign(token.begin, token.end - token.begin);
                    //printf("Literal:%s\n", assignName.c_str());
                    if (!firstMatching) {
                        isMatching = (assignName == LOCAL_SRC_FILES);
                    }
                    expectToken = Token::Operator;
                } else {
                    expectToken = Token::Eol;
                }
                break;
        
            case Token::ShortExpression:
            case Token::Expression:
                expectToken = Token::Eol;
                break;
                
            case Token::Operator:
                if (expectToken == Token::Operator) {
                    isMatching = (assignName == LOCAL_SRC_FILES);
                    expectToken = Token::Eol;
                }
                break;
                
            case Token::BraceOpen:
            case Token::BraceClose:
            case Token::Comma:
            case Token::Colon:
            case Token::Pipe:
                expectToken = Token::Eol;
                break;
        }
        
        if (isMatching) {
            if (firstMatching) {
                firstMatching = false;
                os << ":= \\\n" << newSrc;
            }
        } else {
            os << std::string(token.begin, token.end - token.begin);
        }
        //printf("%s\n", token.dump().c_str());
    }
    //printf("%s\n", os.str().c_str());

    return os.str();
}
