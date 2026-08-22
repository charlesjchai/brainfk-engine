#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "utils.hpp"

using std::cin;
using std::cout;

int main(int argc, char *argv[]) {
    constexpr std::string_view VERSION = "0.1.0";
    constexpr std::string_view HELP = R"(Usage: brainfkrun [--version] [--optimize] [--help]
<command> file

COMMANDS:

    interpret   Interpret a .bf file
    transpile   Transpile the .bf file into a .c file
    compile     Transpile a .bf file and compile it

FLAGS:
    [--version]
        Shows the current version

    [--optimize]
        Shrinks multiple instructions into a single one (+++++ becomes += 5)

    [--help]
        Shows this screen
)";

    if (argc == 1) {
        cout << HELP;
        return 1;
    }
    std::vector<std::string> args(argv + 1, argv + argc);
    std::unordered_set<Flag> flags;
    Target target;
    std::string filename;
    for (int i = 0; i < args.size() - 1; i++) {
        const auto &arg = args[i];

        // Add flags to set
        if (arg.starts_with("--")) {
            if (arg == "--help") {
                cout << HELP;
                return 0;
            } else if (arg == "--version") {
                cout << "brainfk-engine v" << VERSION << '\n';
            } else if (arg == "--optimize") {
                flags.insert(Flag::Optimize);
            } else {
                cout << "ERROR: `" << arg << "` is not a valid flag.\n";
                return 1;
            }
            continue;
        }

        // Set target flag
        if (arg == "interpret") {
            target = Target::Interpret;
        }
        else if (arg == "transpile") {
            target = Target::Transpile;
        }
        else if (arg == "compile") {
            target = Target::Compile;
        }
        else {
            cout << "ERROR: `" << arg << "` is not a valid command.\n";
            return 1;
        }
    }
    // Make sure the last element isn't a flag
    if (args.back().starts_with("--")) {
        cout << "ERROR: flags go before the file.\n";
        return 1;
    }
    filename = args.back();

    return 0;
}
