#ifndef RWE_COBTHREAD_H
#define RWE_COBTHREAD_H

#include "CobFunction.h"
#include <boost/variant.hpp>
#include <rwe/util.h>
#include <stack>
#include <vector>

namespace rwe
{
    class CobThread
    {
    public:
        std::string name;

        std::stack<int> stack;

        unsigned int signalMask{0};

        std::stack<CobFunction> callStack;

        int returnValue;

    public:
        explicit CobThread(const std::string& name);
    };
}

#endif
