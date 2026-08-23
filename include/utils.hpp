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

    // Input can ONLY contain Brainf**k's 8 symbols (><+-.,[]), throws error if otherwise
    void interpret(std::string_view, const std::unordered_set<Flag>&);
}