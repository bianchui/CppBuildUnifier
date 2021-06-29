#define EXPECT_EQ(a,b) {\
   auto vA = a; \
   auto vB = b; \
   if (vA != vB) {\
     std::stringstream aV; aV << vA;\
     std::stringstream bV; bV << vB;\
     this->expectFailed(__FILE__, __LINE__, #a, #b, aV.str(), bV.str());\
   }\
}


#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>


class Failed : public std::exception
{
};


struct NOTCase
{
    NOTCase(const char *Group, const char *Function);
    virtual void run() = 0;

    const char *Function;
    const char *Group;

    void expectFailed(const char *file, int line, const char *aE, const char *bE, std::string vA, std::string vB);

    std::stringstream log;
    bool result;

};
extern std::vector<NOTCase *> NotCases;

inline NOTCase::NOTCase(const char *Group, const char *Function)
    : Group(Group)
    , Function(Function)
    , result(true)
{
    NotCases.push_back(this);
}

inline void NOTCase::expectFailed(const char *file, int line, const char *aE, const char *bE, std::string vA, std::string vB)
{
    result = false;
    log << "    " << file << ":" << line << ": expect_eq failed" << std::endl;
    log << "      > " << aE << " = " << vA << std::endl;
    log << "      > " << bE << " = " << vB << std::endl;
    throw Failed();
}

#define TEST(Group, Function) \
    struct NOTCase_##Group##Function : NOTCase { \
        NOTCase_##Group##Function () : NOTCase( #Group, #Function) {};\
        virtual void run(); }; \
    static NOTCase_##Group##Function INSTANCE_NOTCase_##Group##Function; \
    void NOTCase_##Group##Function::run ()


int NotMain()
{
    std::cerr << "running " <<  NotCases.size()   <<  " cases" << std::endl;
    for (auto i = NotCases.begin(); i != NotCases.end(); i++) {
        std::cerr << "[...] "    << (*i)->Group << "::" << (*i)->Function << "";;

        try {
            (*i)->run();

            if ((*i)->result) {
                std::cerr << "\r[PASS] "    << (*i)->Group << "::" << (*i)->Function << std::endl;
            } else {
                std::cerr << "\r[FAIL] "    << (*i)->Group << "::" << (*i)->Function << std::endl;
                std::cerr << (*i)->log.str();
            }
        } catch (const Failed &e) {
                std::cerr << "\r[FAIL] "    << (*i)->Group << "::" << (*i)->Function << std::endl;
                std::cerr << (*i)->log.str();

        } catch (const std::exception &e) {
                std::cerr << "\r[FAIL] "    << (*i)->Group << "::" << (*i)->Function << std::endl;
                std::cerr << "exception thrown" << std::endl;
                std::cerr << e.what() << std::endl;
        }
    }
}

#define NOT_MAIN \
std::vector<NOTCase *> NotCases; \
int main() { NotMain(); }


