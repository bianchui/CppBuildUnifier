#ifndef WOPWOPERRORHPPBLACRAP
#define WOPWOPERRORHPPBLACRAP

#include <string>
#include <iostream>
#include <string.h>
#include "parser.hpp"


#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>


#define UNW_LOCAL_ONLY
#include <libunwind.h>



inline std::string unwind_backtrace () {
    char *buf  = (char*)malloc(1024);
    char *buf2 = (char*)malloc(1024);

    std::string r;

    unw_cursor_t cursor; unw_context_t uc;
    unw_word_t ip, sp;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    unw_step(&cursor);
    while (unw_step(&cursor) > 0) {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        buf[0] = 0;
        unw_get_proc_name(&cursor, buf, 1024, 0);
        size_t length = 1024;


        if (strcmp("__libc_start_main", buf) == 0)
            break;

        buf2[0] = 0;
        abi::__cxa_demangle(
                buf,
                buf2,
                &length, 0);

        r += "    -> ";
        r += buf2[0] ? buf2 : buf;
        r += " \n";
    }

    free(buf);
    free(buf2);
    return r;
}



class Error : public std::exception
{
public:
    std::string text;
    Node::Ptr node;

    virtual ~Error() throw() {}
    Error(const std::string &t, Node::Ptr node = Node::Ptr(0))
        : text(t)
        , node(node)
    {
        text += unwind_backtrace();
    }
    virtual const char* what() const noexcept {
        return text.c_str();
    }

    int code () const { return 8; }

    void pretty() const;
};


#endif
