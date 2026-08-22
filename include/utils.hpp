#pragma once
#include <string>

namespace BrainFK {
    enum class Mode {
        Interpret,
        Transpile,
        Compile,
    };
    enum class Flag {
        Optimize,
    };

    // Input can ONLY contain Brainf**k's 8 symbols (><+-.,[])
    void interpret(std::string_view, bool);
}