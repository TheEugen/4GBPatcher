# 4GBPatcher

## Overview
4GBPatcher is a C++ utility designed to modify 32-bit Windows executables, enabling them to utilize up to 4GB of RAM. This tool is particularly useful for applications that require more memory, such as games or large-scale processing software. In addition to patching, the utility allows users to view the patch status of an executable and provides an option to unpatch it if needed.

## Features
- **Memory Limit Expansion**: Increases the maximum addressable memory for 32-bit applications to 4GB.
- **Executable Patching**: Modifies the header of 32-bit Windows executables.
- **Patch Status Check**: Displays whether an executable is already patched.
- **Unpatch Option**: Allows the user to revert the patch, restoring the executable to its original state.
- **Command-Line Interface**: Simple and efficient CLI for easy integration into build processes or automated scripts.
- **Cross-Compatibility**: Supports all 32-bit Windows executables.

## How to Use
1. Clone the repository and navigate to the project directory.
2. Compile the project using a C++ compiler.
3. Run the application with the target executable as an argument: `4GBPatcher.exe <your_executable.exe>`.
4. Use the provided options to patch, unpatch, or check the patch status of the executable.

## Future Improvements
- Add a graphical user interface (GUI) for easier use.
- Implement batch processing for multiple executables at once.
- Add support for logging and error handling during patching.

## License
This project is licensed under the MIT License.
