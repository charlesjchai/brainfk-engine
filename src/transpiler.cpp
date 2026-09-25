#include "transpiler.hpp"
#include <cassert>
#include <cstddef>
#include <format>
#include <iostream>
#include <stack>
using namespace BrainFK;
using std::size_t;
using std::ptrdiff_t;
using std::string;
using std::string_view;
using std::unique_ptr;

namespace {
string brainfk_ast_optimize(string_view program) {
    string new_program(program.length(), ' ');
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
unique_ptr<ASTNode> generate_ast(string_view program,
                                 const std::unordered_set<Flag> &options) {

    std::stack<ASTNode *> nodeStack;
    unique_ptr<ASTNode> rootNode = std::make_unique<ASTNode>(AST::Root{});
    nodeStack.push(rootNode.get());

    const bool &optimized = options.contains(Flag::Optimize);
    string parsed_program;
    if (optimized) {
        parsed_program = brainfk_ast_optimize(program);
    } else {
        parsed_program = program;
    }

    for (size_t ii{}; ii < parsed_program.length(); ii++) {
        const auto &parent_node = nodeStack.top();
        const char instruction = parsed_program[ii];
        switch (instruction) {
        case '<':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Left{1}));
            } else {
                // Count how many '<' are there
                size_t instruction_count{};
                for (size_t i = ii; i < parsed_program.length(); i++) {
                    if (parsed_program.at(i) != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Left{instruction_count}));
                // Skip over the other instructions, but compensate for the for
                // loop only if a character was found
                ii += instruction_count == 0 ? 0 : instruction_count - 1;
            }
            break;
        case '>':
            if (!optimized) {
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Right{1}));
            } else {
                size_t instruction_count{};
                for (size_t i{ii}; i < parsed_program.length(); i++) {
                    if (parsed_program.at(i) != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(
                    std::make_unique<ASTNode>(AST::Right{instruction_count}));
                ii += instruction_count == 0 ? 0 : instruction_count - 1;
            }
            break;
        case '+':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Add{1}));
            } else {
                size_t instruction_count{};
                for (size_t i{ii}; i < parsed_program.length(); i++) {
                    if (parsed_program.at(i) != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(std::make_unique<ASTNode>(
                    AST::Add{static_cast<uint8_t>(instruction_count)}));
                ii += instruction_count == 0 ? 0 : instruction_count - 1;
            }
            break;
        case '-':
            if (!optimized) {
                parent_node->add_child(std::make_unique<ASTNode>(AST::Sub{1}));
            } else {
                size_t instruction_count{};
                for (size_t i{ii}; i < parsed_program.length(); i++) {
                    if (parsed_program.at(i) != instruction) {
                        instruction_count = i - ii;
                        break;
                    }
                }
                parent_node->add_child(std::make_unique<ASTNode>(
                    AST::Sub{static_cast<uint8_t>(instruction_count)}));
                ii += instruction_count == 0 ? 0 : instruction_count - 1;
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
string ast_to_C(ASTNode *node, string &target, size_t &insert_position) {
    string string_to_add;
    ptrdiff_t insert_offset{};
    if (std::holds_alternative<AST::Add>(node->type)) {
        string_to_add = std::format("arr[ptr] += {}; ",
                                    std::get<AST::Add>(node->type).quantity);
    } else if (std::holds_alternative<AST::Sub>(node->type)) {
        string_to_add = std::format("arr[ptr] -= {}; ",
                                    std::get<AST::Sub>(node->type).quantity);
    } else if (std::holds_alternative<AST::Left>(node->type)) {
        string_to_add = std::format("ptr -= {}; if (ptr >= 30000) {{ return 1; }} ",
                                    std::get<AST::Left>(node->type).quantity);
    } else if (std::holds_alternative<AST::Right>(node->type)) {
        string_to_add = std::format("ptr += {}; if (ptr >= 30000) {{ return 1; }} ",
                                    std::get<AST::Right>(node->type).quantity);
    } else if (std::holds_alternative<AST::Print>(node->type)) {
        string_to_add = R"(printf("%c", arr[ptr]); )";
    } else if (std::holds_alternative<AST::Input>(node->type)) {
        string_to_add = R"(scanf("%c", arr[ptr]) )";
    } else if (std::holds_alternative<AST::Zero>(node->type)) {
        string_to_add = "arr[ptr] = 0; ";
    } else if (std::holds_alternative<AST::Loop>(node->type)) {
        string_to_add = "while (arr[ptr] != 0) { } ";
        // Insert before the bracket and the space
        insert_offset = -2;
        std::cerr << insert_offset;
    }
    target.insert(insert_position, string_to_add);
    insert_position += string_to_add.size() + insert_offset;
    for (const auto &child : node->children) {
        ast_to_C(child.get(), target, insert_position);
    }
    return target;
}
} // namespace

string BrainFK::transpile(string_view program,
                          const std::unordered_set<Flag> &options) {
    unique_ptr<ASTNode> asTree{generate_ast(program, options)};
    string target{R"(#include <stdio.h>
#include <stdint.h>
int main(void) { uint8_t arr[30000]; size_t ptr = 0; })"};
    // Insert in between the brackets
    size_t insert_position{target.rfind('}')};

    target = ast_to_C(asTree.get(), target, insert_position);

    std::cout << target;
    return target;
}