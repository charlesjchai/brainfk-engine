#include <iostream>
#include <vector>
#include <stack>
#include "utils.hpp"
using namespace BrainFK;

void BrainFK::interpret(std::string_view program, bool optimized) {
    std::vector<uint8_t> tape(30000);
    size_t byte_index = 0;
    std::string user_input;

    // Setup map for looping
    std::unordered_map<size_t, size_t> loop_map;
    std::stack<size_t> loop_stack;
    for (size_t ii; ii < program.length(); ii++) {
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

    for (size_t ii = 0; ii < program.length(); ii++) {
        switch (program[ii]) {
            case '>':
                byte_index++;
                if (byte_index == tape.size()) {
                    // Push back the correct amount of zeros, to prevent UB.
                    tape.insert(tape.end(), byte_index - tape.size() + 1, 0);
                }
                break;
            case '<':
                byte_index--;
                if (byte_index < 0) byte_index = 0;
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
                    std::cin >> user_input;
                    user_input.push_back('\n');
                }
                // Read and erase the first character of the string
                std::cout << user_input[0];
                user_input.erase(0, 1);
                break;
            case '[':
                if (!tape[byte_index]) { // Stop the loop if the current byte is 0
                    ii = loop_map[ii];
                }
                break;
            case ']':
                if (tape[byte_index]) { // Continue the loop if the current byte is not 0
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