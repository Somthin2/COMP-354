# COMP-354 Shell Implementation

This repository contains a simple shell implementation written in C++. The shell supports basic command execution, path management, directory navigation, output redirection, and background process handling.

## Documentation

This project is documented using Doxygen and hosted on GitHub Pages.

[Click here to view the full HTML documentation](https://Somthin2.github.io/COMP-354/)

## Features

- Execute commands from standard paths
- Handle path management via the "path" command
- Support changing directories with "cd"
- Support output redirection with ">"
- Handle batch script execution from files
- Support background processes using "&"

## Getting Started

### Prerequisites

- C++ compiler with C++11 support
- Unix-like operating system (Linux, macOS)

### Building

```bash
g++ -std=c++11 wish.cpp -o wish
```

### Running

Interactive mode:
```bash
./wish
```

Batch mode:
```bash
./wish script.txt
```

## Repository Structure

- `wish.cpp`: Main shell implementation
- `docs/`: Documentation generated with Doxygen
  - `html/`: HTML documentation
    - `index.html`: Main documentation page

## License

This project is licensed under the [MIT License](LICENSE).
