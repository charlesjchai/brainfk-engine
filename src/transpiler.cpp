#include "transpiler.hpp"
#include <cstddef>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace BrainFK;

namespace {
std::string brainfk_ast_optimize(std::string_view program) {
    std::string new_program(program.length(), ' ');
    // Two variables are needed because new_program can be shorter than program
    for (std::size_t program_ii{}, new_program_ii{};
         program_ii < program.length(); program_ii++, new_program_ii++) {
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
} // namespace

std::string BrainFK::transpile(std::string_view program,
                               const std::unordered_set<Flag> &options) {

    std::unique_ptr<BrainFK::ASTNode> rootNode =
        std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Root{});

    std::stack<std::unique_ptr<BrainFK::ASTNode>> nodeStack;
    nodeStack.push(
        std::move(rootNode)); // The top of the stack is the current parent node

    const bool &optimized = options.contains(Flag::Optimize);
    std::string parsed_program;
    if (optimized) {
        parsed_program = brainfk_ast_optimize(program);
    } else {
        parsed_program = program;
    }
    
    std::cerr << parsed_program << '\n';
    for (int ii{}; ii < program.length(); ii++) {
        const char &instruction = program[ii];
        std::cerr << "Encountered instruction '" << instruction << "'\n";
        if (!optimized) {
            switch (instruction) {
            case '<':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Left{1}));
                break;
            case '>':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Right{1}));
                break;
            case '+':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Add{1}));
                break;
            case '-':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Sub{1}));
                break;
            case '[': {
                auto loop_node =
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Loop{});
                nodeStack.push(std::move(loop_node));
                break;
            }
            case ']':
                nodeStack.pop();
                break;
            case '.':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Print{}));
                break;
            case ',':
                nodeStack.top()->children.push_back(
                    std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Input{}));
                break;
            }
        } else {
        }
    }

    std::string target;
    return target;
}
