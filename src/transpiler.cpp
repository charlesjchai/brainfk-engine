#include "transpiler.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace BrainFK;
using std::size_t;

namespace {
std::string brainfk_ast_optimize(std::string_view program) {
    std::string new_program(program.length(), ' ');
    // Two variables are needed because new_program can be shorter than program
    for (size_t program_ii{}, new_program_ii{}; program_ii < program.length();
         program_ii++, new_program_ii++) {
        const char &instruction{program[program_ii]};
        switch (instruction) {
        case '>':
            [[fallthrough]];
        case '<': {
            std::ptrdiff_t movement_sum{};
            while (true) {
                // Add up the sum of the movements
                if (program[program_ii] == '<') {
                    movement_sum--;
                } else if (program[program_ii] == '>') {
                    movement_sum++;
                } else {
                    // Decrement on the last iteration to compensate for the
                    // loop's increment
                    program_ii--;
                    break;
                }
                program_ii++;
            }
            // Apply the sum to the string
            for (; movement_sum > 0; movement_sum--, new_program_ii++) {
                new_program[new_program_ii] = '>';
            }
            for (; movement_sum < 0; movement_sum++, new_program_ii++) {
                new_program[new_program_ii] = '<';
            }
            // Decrement to compensate for the loop's increment
            new_program_ii--;
            break;
        }
        case '+':
            [[fallthrough]];
        case '-': {
            std::ptrdiff_t movement_sum{};
            while (true) {
                if (program[program_ii] == '-') {
                    movement_sum--;
                } else if (program[program_ii] == '+') {
                    movement_sum++;
                } else {
                    program_ii--;
                    break;
                }
                program_ii++;
            }
            for (; movement_sum > 0; movement_sum--, new_program_ii++) {
                new_program[new_program_ii] = '+';
            }
            for (; movement_sum < 0; movement_sum++, new_program_ii++) {
                new_program[new_program_ii] = '-';
            }
            new_program_ii--;
            break;
        }
        case '[':
            if (program.substr(program_ii, 3) == "[-]") {
                new_program[new_program_ii] = '0';
                // +2 and not +3 to compensate for the for loop increment
                program_ii += 2;
                break;
            }
        default:
            new_program[new_program_ii] = program[program_ii];
        }
    }
    // Trim whitespace
    new_program.erase(new_program.find_last_not_of(' ') + 1);
    return new_program;
}
std::unique_ptr<ASTNode> generate_ast(std::string_view program,
                                      const std::unordered_set<Flag> &options) {

    std::stack<ASTNode *> nodeStack;
    std::unique_ptr<ASTNode> rootNode = std::make_unique<ASTNode>(AST::Root{});
    nodeStack.push(rootNode.get());

    const bool &optimized = options.contains(Flag::Optimize);
    std::string parsed_program;
    if (optimized) {
        parsed_program = brainfk_ast_optimize(program);
    } else {
        parsed_program = program;
    }

    std::cerr << parsed_program << '\n';
    for (size_t ii{}; ii < program.length(); ii++) {
        const auto &parent_node = nodeStack.top();
        const char &instruction = program[ii];
        std::cerr << "Encountered instruction " << instruction << '\n';
        switch (instruction) {
        case '<':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Left{1}));
            } else {
                // Count how many '<' are there
                size_t instruction_count{};
                for (size_t i = ii; i < program.length(); i++) {
                    if (program[i] != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Left{instruction_count}));
                // Skip over the other instructions, but compensate for the for
                // loop
                ii += instruction_count - 1;
            }
            break;
        case '>':
            if (!optimized) {
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Right{1}));
            } else {
                size_t instruction_count{};
                for (size_t i = ii; i < program.length(); i++) {
                    if (program[i] != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Right{instruction_count}));
                ii += instruction_count - 1;
            }
            break;
        case '+':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Add{1}));
            } else {
                size_t instruction_count{};
                for (size_t i = ii; i < program.length(); i++) {
                    if (program[i] != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(std::make_unique<ASTNode>(
                    AST::Add{static_cast<uint8_t>(instruction_count)}));
                ii += instruction_count - 1;
            }
            break;
        case '-':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Sub{1}));
            } else {
                size_t instruction_count{};
                for (size_t i = ii; i < program.length(); i++) {
                    if (program[i] != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(std::make_unique<ASTNode>(
                    AST::Sub{static_cast<uint8_t>(instruction_count)}));
                ii += instruction_count - 1;
            }
            break;
        case '[': {
            parent_node->add_child(std::make_unique<ASTNode>(AST::Loop{}));
            nodeStack.push(parent_node->children.back().get());
            // Descend into the loop
            break;
        }
        case ']':
            nodeStack.pop();
            // We are done with the loop, back up we go
            break;
        case '.':
            parent_node->add_child(std::make_unique<ASTNode>(AST::Print{}));
            break;
        case ',':
            parent_node->add_child(std::make_unique<ASTNode>(AST::Input{}));
            break;
        default:
            std::cerr << "Undefined symbol: '" << instruction << "'"
                      << std::endl;
            throw std::runtime_error("Undefined symbol");
        }
    }
    return std::move(rootNode);
}
} // namespace

std::string BrainFK::transpile(std::string_view program,
                               const std::unordered_set<Flag> &options) {
    std::unique_ptr<ASTNode> asTree = generate_ast(program, options);
    std::string target;

    

    return target;
}
