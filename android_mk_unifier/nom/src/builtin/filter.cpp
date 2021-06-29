#include "../semantics.hpp"
#include "../error.hpp"
#include "../utils.hpp"


//from patsubst
void splitshit(const std::string &in, std::string &a, std::string &b);


class Filter : public Function
{
    VString operator() (
            Context *context,
            const std::string &functionName,
            std::vector <Expression::Ptr> args,
            Expression::Ptr callContext
            )
    {

        bool filterout = (functionName == "filter-out");

        if (args.size() != 2)
            throw Error("expecting exactly 2 arguments to function filter");

        VString arg0 = context->eval(args.at(0));
        auto arg1 = context->eval(args.at(1)).split();
        VString ret;

        std::string searchA, searchB;
        splitshit(arg0, searchA, searchB);

        for (auto i = arg1.begin(); i != arg1.end(); i++) {
            std::string &me = *i;

            size_t ea = me.find(searchA);
            // can't find first needle part in this one
            if (ea == std::string::npos) {
                if (filterout)
                    ret.push_back(me);
                continue;
            }

            ea += searchA.size();

            size_t eb = me.find(searchB, ea);
            // can't find second needle part in this one
            if (eb == std::string::npos) {
                if (filterout)
                    ret.push_back(me);
                continue;
            }

            if (!filterout)
                ret.push_back(me);
        }

        return ret;
    }
};


REGISTER_FUNCTION(filter, Filter);
REGISTER_FUNCTION_STR(filter_out, Filter, "filter-out");
