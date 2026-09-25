#include "interpreter.hpp"
#include "transpiler.hpp"
#include <fstream>
#include <iostream>

using std::cout;
using namespace BrainFK;

int main(int argc, char *argv[]) {
    constexpr std::string_view VERSION = "0.1.0";
    constexpr std::string_view HELP =
        R"(Usage: brainfkrun [--version] [--optimize] [--output <FILE>] [--help]
<MODE> FILE

MODES:

    interpret   Interpret a .bf file
    transpile   Transpile the .bf file into a .c file
    compile     Transpile a .bf file and compile it

FLAGS:

    [--version]
        Shows the current version

    [--help]
        Shows this screen

    [--optimize]
        Shrinks multiple instructions into a single one (>>>>> becomes cell_index += 5)

    [--output <FILE>]
        Outputs to a file in transpile and compile mode ONLY
)";
    // TODO: Change `const` to `constexpr` in C++26
    const std::unordered_set<char> BRAINFK_SYMBOLS = {'+', '-', '>', '<',
                                                      '.', ',', '[', ']'};

    if (argc == 1) {
        std::cerr << HELP;
        return 1;
    }
    std::vector<std::string> args(argv + 1, argv + argc);
    std::unordered_set<Flag> flags;
    Mode mode{-1}; // mode is invalid until declared
    bool areFlagsEnded{};
    std::optional<std::string> output{std::nullopt};
    std::string filename;

    // Parse arguments
    for (std::size_t i{}; i < args.size(); i++) {
        const auto &arg{args[i]};

        // Add flags to set
        if (arg.starts_with("--") && !areFlagsEnded) {
            if (arg == "--") {
                // End of flags delimiter
                areFlagsEnded = true;
            } else if (arg == "--help") {
                cout << HELP;
                return 0;
            } else if (arg == "--version") {
                cout << "brainfk-engine v" << VERSION << '\n';
            } else if (arg == "--optimize") {
                flags.insert(Flag::Optimize);
            } else if (arg == "--output") {
                output = args[i + 1];
                flags.insert(Flag::Output);
                i++;
            } else {
                std::cerr << "ERROR: " << arg << "' is not a valid flag.\n";
                return 1;
            }
            continue;
        }

        // Set target mode
        if (mode == static_cast<Mode>(-1)) {
            if (arg == "interpret") {
                mode = Mode::Interpret;
            } else if (arg == "transpile") {
                mode = Mode::Transpile;
            } else if (arg == "compile") {
                mode = Mode::Compile;
            } else {
                std::cerr << "ERROR: '" << arg << "' is not a valid mode.\n";
                return 1;
            }
        }
        if (i == args.size() - 1) {
            filename = args.back();
        }
    }
    if (mode == static_cast<Mode>(-1)) {
        std::cerr << "ERROR: Mode not set!\n";
        return 1;
    }
    if (flags.contains(Flag::Output) && mode == Mode::Interpret) {
        std::cerr << "ERROR: Cannot use output flag with interpret\n";
        return 1;
    }

    bool optimized{flags.contains(Flag::Optimize)};
    std::ifstream program_file(filename);
    std::string line;

    std::string program;

    if (!program_file.is_open()) {
        std::cerr << "Error opening '" << filename << "' for reading\n";
        return 1;
    }
    char ch;
    // Get rid of everything that isn't an instruction
    while (program_file.get(ch)) {
        if (BRAINFK_SYMBOLS.contains(ch)) {
            program.push_back(ch);
        }
    }
    uint64_t before{static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count())};
    switch (mode) {
    case Mode::Interpret:
        interpret(program, flags);
        break;
    case Mode::Transpile:
        transpile(program, flags);
        break;
    case Mode::Compile:
        // TBA
        break;
    }
    uint64_t now{static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count())};
    std::cerr << "Time elapsed: " << now - before << " µs\n";
    return 0;
}
