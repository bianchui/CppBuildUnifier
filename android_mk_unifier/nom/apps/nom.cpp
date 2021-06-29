#include <iostream>
#include <fstream>

#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/semantics.hpp"
#include "../src/utils.hpp"

#include <stdexcept>
#include "../src/error.hpp"
#include <string.h>



void getContextStringValue(Context *context, const std::string &name, std::string &value) {
    if (!context->values[name]) {
        value = std::string();
        return;
    }
    value = context->eval(context->values[name]);
}

void getContextStringValues(Context *context, const std::string &name, std::vector<std::string> &values) {
    if (!context->values[name]) {
        return;
    }
    values = context->eval(context->values[name]).split();
}

void getContextPathValues(Context *context, const std::string &name, std::vector<std::string> &values) {
    if (!context->values[name]) {
        return;
    }
    auto local_path = context->eval(context->values["LOCAL_PATH"]);

    values = context->eval(context->values[name]).split();


    for (int i = 0; i < values.size(); i++) {
        values[i] = pathAbsolutize(local_path, values[i]);
    }
}

class ModuleTarget
{
private:
public:
    ModuleTarget(Context *context, const std::string &type)
        : type (type)
    {
        getContextStringValue   (context,"LOCAL_MODULE", name);
        getContextPathValues    (context,"LOCAL_SRC_FILES", sources);
        getContextStringValues  (context,"LOCAL_CFLAGS", cFlags);
        getContextPathValues    (context,"LOCAL_C_INCLUDES", cIncludes);
        getContextStringValues  (context,"LOCAL_SHARED_LIBRARIES", cSharedLibraries);
    }

    std::string name;
    std::string type;
    std::vector<std::string> sources;
    std::vector<std::string> cFlags;
    std::vector<std::string> cIncludes;
    std::vector<std::string> cSharedLibraries;

    void dump() {
        std::cout << "==> " << name << std::endl;
        std::cerr << "    T " << type << std::endl;
        for (int i = 0; i < sources.size(); i++) {
            std::cerr << "    S " << sources[i] << std::endl;
        }
        for (int i = 0; i < cSharedLibraries.size(); i++) {
            std::cerr << "    L " << cSharedLibraries[i] << std::endl;
        }
        std::cerr << std::endl;
    }

};


void Error::pretty() const {
    std::ostream &out = std::cerr;


    out << std::endl << text << std::endl;

    if (node != 0) {
        out << node->begin.fileName << ":" << node->begin.line  << std::endl;

        const char * lineBegin = node->begin.begin - node->begin.column;
        const char * lineEnd   = strpbrk(node->begin.end, "\n");
        int lineCount  = lineEnd - lineBegin;

        std::string l1 (lineBegin, lineCount);
        out << l1 << std::endl;

        out << std::string(node->begin.column -1, ' ') << '^';
        if (node->end.column - node->begin.column > 0)
        out << std::string(node->end.column - node->begin.column, '~');
        out << std::endl;

    }
    out << std::endl;

}




static void pretty(Context *context, const std::string &name, bool isReleativePath = false) {
    auto r = context->eval(context->values[name]).split();


    auto local_path = context->eval(context->values["LOCAL_PATH"]);


    std::cerr << name << ":";
    if (r.size() < 1) {
    } else if (r.size() == 1) {
        std::cerr << " " << r[0];
    } else {
        for (int i = 0; i < r.size(); i++) {
            if (isReleativePath) {
                std::cerr << std::endl << "    " << pathAbsolutize(local_path, r[i]);
            } else {
                std::cerr << std::endl << "    " << r[i];
            }
        }
    }
    std::cerr << std::endl;
}


class AndroidIncludeHandler : public IncludeHandler
{
public:

    virtual void buildSharedLibrary(Context *context) {

    }

    virtual bool operator()(
            Context *ctx,
            IncludeNode::Ptr node,
            MakefileParser *parser
            )
    {
        for (auto i = node->expr->expression.begin(); i != node->expr->expression.end(); i++) {
            Expression::Ptr a = *i;

            if (a->type == Node::ReferenceExpression) {
                auto ii = std::static_pointer_cast<ReferenceExpression>(a);
                VString  res = ctx->eval(ii->reference);
                if (res == "CLEAR_VARS") {
                    return true;
                } else if (res == "BUILD_EXECUTABLE") {
                    (new ModuleTarget(ctx, "executable"))->dump();
                    return true;
                } else if (res == "BUILD_PREBUILT") {
                    (new ModuleTarget(ctx, "prebuilt"))->dump();
                    return true;
                } else if (res == "BUILD_STATIC_LIBRARY") {
                    (new ModuleTarget(ctx, "prebuilt"))->dump();
                    return true;
                } else if (res == "BUILD_SHARED_LIBRARY") {
                    (new ModuleTarget(ctx, "shared-library"))->dump();
                    return true;
                } else if (res == "BUILD_HOST_EXECUTABLE") {
                    (new ModuleTarget(ctx, "host-executable"))->dump();
                    return true;
                } else if (res == "BUILD_HOST_SHARED_LIBRARY") {
                    (new ModuleTarget(ctx, "host-shared-library"))->dump();
                    return true;
                } else if (res == "BUILD_HOST_STATIC_LIBRARY") {
                    (new ModuleTarget(ctx, "host-static-library"))->dump();
                    return true;
                } else if (res == "BUILD_STATIC_JAVA_LIBRARY") {
                    (new ModuleTarget(ctx, "static-java-library"))->dump();
                    return true;
                } else if (res == "BUILD_HOST_JAVA_LIBRARY") {
                    (new ModuleTarget(ctx, "host-java-library"))->dump();
                    return true;
                } else if (res == "BUILD_HOST_PREBUILT") {
                    (new ModuleTarget(ctx, "package"))->dump();
                    return true;
                } else if (res == "BUILD_JAVA_LIBRARY") {
                    (new ModuleTarget(ctx, "java-library"))->dump();
                    return true;
                } else if (res == "BUILD_CTS_PACKAGE") {
                    (new ModuleTarget(ctx, "cts-package"))->dump();
                    return true;
                } else if (res == "BUILD_PTS_PACKAGE") {
                    (new ModuleTarget(ctx, "cts-pts-package"))->dump();
                    return true;
                } else if (res == "BUILD_CTSCORE_PACKAGE") {
                    (new ModuleTarget(ctx, "cts-core-package"))->dump();
                    return true;
                } else if (res == "BUILD_PACKAGE") {
                    (new ModuleTarget(ctx, "package"))->dump();
                    return true;
                } else if (res == "BUILD_SYSTEM") {
                    (new ModuleTarget(ctx, "something-else"))->dump();
                    return true;
                }

            }
        }

        return false;
    }
};


int main(int argc, char **argv)
{
    std::string main;


    std::map<std::string, std::string> defines;

    for(int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-D", 2) == 0) {
            auto l = split(argv[i] + 2, '=');
            if (l.size() < 2) {
                defines[l[0]] = std::string();
            } else {
                defines[l[0]] = l[1];
            }
        } else {
            main = argv[i];
        }
    }

    if (main.empty()) {
        std::cerr << "usage: blurp [ -Dstuff=true ] Android.mk" << std::endl;
        return 3;
    }

    MakefileParser     parser(main);
    MakefileSemantics  sem(&parser);


    sem.context->includeHandlers.push_back(new AndroidIncludeHandler);




    for (auto i = defines.begin(); i != defines.end(); i++) {


        Expression::Ptr s(new LiteralExpression(i->second));
        sem.context->values[i->first] =  s;
    }


    try {
        sem.d();
    } catch (Error &e) {
        e.pretty();
        return e.code();
    }

    return 0;
}
