# brainfk-engine
## Description
brainfk-engine is a command that executes Brainf**k scripts. It has 3 modes, which are ways it runs the program.
- Interpret: Interprets a given .bf script
- Transpile: Transpiles the script into a .c file
- Compile: Runs transpilation and compiles the .c file into a binary

## Run it
### Dependencies
`git cmake make gcc/clang`
### Installation
```
git clone https://github.com/charlesjchai/brainfk-engine
cd brainfk-engine
cmake -S . -B build
cd build
make
./brainfkrun
```
### Executing brainfk-engine
Usage: `brainfkrun <flags> <mode> file`
### Flags
- `--version` prints the current version.
- `--help` displays the help screen.
- `--optimize` optimizes multiple instructions into one. For example, `>>>>>` becomes `index += 5`, and `[-]` is simplified to `arr[index] = 0`.
## License
This project is licensed under the MIT License.