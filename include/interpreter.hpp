#pragma once
#include "flag.hpp"
#include <unordered_set>

namespace BrainFK {
// Input can ONLY contain Brainf**k's 8 symbols (><+-.,[]), throws error if
// otherwise
void interpret(std::string_view program,
               const std::unordered_set<Flag> &options);
} // namespace BrainFK
