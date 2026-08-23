# brainfk-engine
## Description
brainfk-engine is a command that executes Brainf**k scripts. It has 3 modes, which are ways it runs the program.
- Interpret: Interprets a given .bf script
- Transpile: Transpiles the script into a .c file
- Compile: Runs transpilation and compiles the .c file into a binary

## Run it
### Dependencies
- `git`
- `cmake`
- A C++ compiler (g++, clang++, etc.)
### Installation
```
git clone https://github.com/charlesjchai/brainfk-engine
cd brainfk-engine
mkdir build && cd build
cmake ..
cmake --build .
./brainfkrun
```
### Executing brainfk-engine
Usage: `brainfkrun <flags> <mode> file`
### Flags
- `--version` prints the current version.
- `--help` displays the help screen.
- `--optimize` optimizes multiple instructions into one. For example, `>>>>>` becomes `index += 5`, and `[-]` is simplified to `arr[index] = 0` in transpile and compile mode. This flag has the least effect in interpret mode.
## License
This project is licensed under the MIT License.
