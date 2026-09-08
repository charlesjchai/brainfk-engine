#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <variant>

namespace BrainFK {
enum class ASTToken;
}
using ASTValue = std::variant<BrainFK::ASTToken, int>;

namespace BrainFK {
enum class Mode {
    Interpret,
    Transpile,
    Compile,
};
enum class Flag {
    Optimize,
};

// AST Node to transpile into C
enum class ASTToken {
    // Add and Sub have one child, which is the int in the std::variant
    Add,
    Sub,
    Left,
    Right,
    Print,
    Input,
    Loop,
};
struct ASTNode {
    ASTValue value;
    std::unique_ptr<ASTNode> child;
};

// Input can ONLY contain Brainf**k's 8 symbols (><+-.,[]), throws error if
// otherwise
void interpret(std::string_view program,
               const std::unordered_set<Flag> &options);
std::string transpile(std::string_view program,
                      const std::unordered_set<Flag> &options);
} // namespace BrainFK