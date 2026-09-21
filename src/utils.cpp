#include "utils.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace BrainFK;

namespace {
std::unordered_map<std::size_t, std::size_t>
build_loop_table(std::string_view program) {
    std::unordered_map<std::size_t, std::size_t> loop_map;
    std::stack<std::size_t> loop_stack;

    // The value of the index of a '[' is equal to the index of its matching
    // ']'.
    for (std::size_t ii{}; ii < program.length(); ii++) {
        const char &instruction{program[ii]};
        if (instruction == '[') {
            loop_stack.push(ii);
        } else if (instruction == ']') {
            std::size_t loop_start_index = loop_stack.top();
            loop_stack.pop();
            loop_map[loop_start_index] = ii;
            loop_map[ii] = loop_start_index;
        }
    }
    return loop_map;
}
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
                    // Decrement to compensate for the for loop's increment
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
            // Decrement to compensate for the for loop's increment
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
void BrainFK::interpret(std::string_view program,
                        const std::unordered_set<Flag> &options) {

    const bool &optimized = options.contains(Flag::Optimize);

    std::vector<uint8_t> tape(1024);
    std::size_t byte_index{};
    std::string user_input;

    std::unordered_map<std::size_t, std::size_t> loop_map =
        build_loop_table(program);

    for (std::size_t ii{}; ii < program.length(); ii++) {
        const char &instruction = program[ii];
        switch (instruction) {
        case '>':
            byte_index++;
            if (byte_index == tape.size()) {
                tape.resize(tape.size() * 2);
            }
            break;
        case '<':
            if (!byte_index) {
                throw std::out_of_range("Index below 0");
            }
            byte_index--;
            break;
        case '+':
            tape[byte_index]++;
            break;
        case '-':
            tape[byte_index]--;
            break;
        case '.':
            std::cout << tape[byte_index];
            break;
        case ',':
            if (user_input.empty()) {
                std::getline(std::cin, user_input);
                user_input.push_back('\n');
            }
            // Use user_input as a buffer for incoming chars
            tape[byte_index] = user_input[0];
            user_input.erase(0, 1);
            break;
        case '[':
            if (program.substr(ii, 3) == "[-]" && optimized) {
                tape[byte_index] = 0;
                ii += 2;
                break;
            }
            if (!tape[byte_index]) { // Stop the loop if the current byte is 0
                ii = loop_map[ii];
            }
            break;
        case ']':
            if (tape[byte_index]) { // Continue the loop if the current byte is
                                    // not 0
                ii = loop_map[ii];
            }
            break;
        default:
            std::cerr << "Undefined symbol: '" << instruction << "'"
                      << std::endl;
            throw std::runtime_error("Undefined symbol");
            break;
        }
    }
}

std::string BrainFK::transpile(std::string_view program,
                               const std::unordered_set<Flag> &options) {
    std::unordered_map<std::size_t, std::size_t> loop_map =
        build_loop_table(program);
    std::unique_ptr<BrainFK::ASTNode> rootNode =
        std::make_unique<BrainFK::ASTNode>(BrainFK::AST::Root{});

    std::stack<std::unique_ptr<BrainFK::ASTNode>> nodeStack;
    nodeStack.push(std::move(rootNode)); // The top of the stack is the current parent node

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
