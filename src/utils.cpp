#include "utils.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace BrainFK;

static std::unordered_map<size_t, size_t> build_loop_table(std::string_view program) {
    std::unordered_map<size_t, size_t> loop_map;
    std::stack<size_t> loop_stack;

    // The value of the index of a '[' is equal to the index of its matching ']'.
    for (size_t ii = 0; ii < program.length(); ii++) {
        uint8_t instruction = program[ii];
        if (instruction == '[') {
            loop_stack.push(ii);
        } else if (instruction == ']') {
            size_t loop_start_index = loop_stack.top();
            loop_stack.pop();
            loop_map[loop_start_index] = ii;
            loop_map[ii] = loop_start_index;
        }
    }
    return loop_map;
}

void BrainFK::interpret(std::string_view program,
                        const std::unordered_set<Flag> &options) {

    const bool &optimized = options.contains(Flag::Optimize);

    std::vector<uint8_t> tape(1024);
    size_t byte_index = 0;
    std::string user_input;

    std::unordered_map<size_t, size_t> loop_map = build_loop_table(program);

    for (size_t ii = 0; ii < program.length(); ii++) {
        switch (program[ii]) {
        case '>':
            byte_index++;
            if (byte_index == tape.size()) {
                tape.resize(tape.size() * 2);
            }
            break;
        case '<':
            if (byte_index == 0) {
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
            std::cerr << "Undefined symbol: " << program[ii] << std::endl;
            throw std::runtime_error("Undefined symbol");
            break;
        }
    }
}

std::string BrainFK::transpile(std::string_view program, const std::unordered_set<Flag> &options) {
    std::unique_ptr<BrainFK::ASTNode> asTree;
    std::unordered_map<size_t, size_t> loop_map = build_loop_table(program);
    

    std::string target;
    return target;
}