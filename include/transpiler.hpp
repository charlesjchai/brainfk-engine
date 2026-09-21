#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_set>
#include "flag.hpp"
#include <variant>
#include <vector>

namespace BrainFK {
enum class Mode {
    Interpret,
    Transpile,
    Compile,
};

namespace AST {
struct Root {};
struct Add {
    uint8_t quantity;
};
struct Sub {
    uint8_t quantity;
};
struct Left {
    std::size_t quantity;
};
struct Right {
    std::size_t quantity;
};
struct Zero {}; // Sets the current cell to 0
struct Loop {};
struct Print {};
struct Input {};
} // namespace AST

struct ASTNode {
    using NodeVariant =
        std::variant<BrainFK::AST::Root, BrainFK::AST::Add, BrainFK::AST::Sub,
                     BrainFK::AST::Left, BrainFK::AST::Right,
                     BrainFK::AST::Zero, BrainFK::AST::Loop,
                     BrainFK::AST::Print, BrainFK::AST::Input>;
    NodeVariant type;
    std::vector<std::unique_ptr<ASTNode>> children;

    ASTNode() = default;
    ASTNode(NodeVariant type) : type{type} {}
};
std::string transpile(std::string_view program,
                      const std::unordered_set<Flag> &options);
} // namespace BrainFK
