# Compiler for PSI Language

This project is a compiler for the **PSI Language** written in C. The PSI Language is a simple, educational programming language designed to teach compiler design concepts. This compiler translates PSI code into an intermediate representation and then executes it or translates it to another target language.

## Table of Contents

- [Project Overview](#project-overview)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
  - [Command Line Interface](#command-line-interface)
  - [Example Usage](#example-usage)
- [PSI Language Syntax](#psi-language-syntax)
  - [Basic Syntax](#basic-syntax)
  - [Supported Data Types](#supported-data-types)
  - [Control Structures](#control-structures)
  - [Functions](#functions)
- [Code Structure](#code-structure)
- [Development](#development)
- [Testing](#testing)
- [Contributing](#contributing)
- [Acknowledgements](#acknowledgements)

## Project Overview

The **Compiler-for-PSI-language** project is a comprehensive solution for parsing, analyzing, and compiling PSI language programs. This project aims to provide a clear understanding of compiler design, including lexical analysis, syntax analysis, semantic analysis, optimization, and code generation.

### Key Objectives

- Demonstrate compiler design principles.
- Provide a practical example of a language compiler implemented in C.
- Facilitate learning through clear code organization and comments.

## Features

- **Lexical Analysis**: Tokenizes PSI source code into meaningful symbols.
- **Syntax Analysis**: Parses the token stream to validate language syntax.
- **Semantic Analysis**: Ensures type correctness and scope validation.
- **Intermediate Code Generation**: Produces an intermediate representation (IR) of PSI code.
- **Optimization**: Basic optimization techniques for improving performance.
- **Code Generation**: Translates IR into target code or interprets directly.
- **Error Handling**: Provides descriptive error messages for debugging.

## Installation

To compile and install the PSI Compiler, follow these steps:

1. **Clone the repository:**

   ```bash
   git clone https://github.com/your-username/compiler-for-psi-language.git
   cd compiler-for-psi-language
   ```

2. **Build the compiler:**

   Ensure you have a C compiler installed (e.g., `gcc`) and run the following command:

   ```bash
   make
   ```

3. **Run tests (optional):**

   To ensure everything is set up correctly, run:

   ```bash
   make test
   ```

4. **Install (optional):**

   To install the compiler globally, run:

   ```bash
   sudo make install
   ```

   This will copy the executable to `/usr/local/bin/` or your preferred directory.

## Usage

The PSI Compiler can be used from the command line. Below are the basic commands to compile and execute PSI code.

### Command Line Interface

```bash
./psi-compiler [options] source.psi
```

**Options:**

- `-o <output>`: Specify the output file for the compiled code.
- `-i`: Interactive mode for direct code execution.
- `-v`: Enable verbose mode for detailed output.
- `-h`: Display help information.

### Example Usage

**Compile and execute a PSI file:**

```bash
./psi-compiler examples/hello_world.psi
```

**Compile and specify output:**

```bash
./psi-compiler -o output.psi examples/hello_world.psi
```

**Interactive mode:**

```bash
./psi-compiler -i
```

**Help:**

```bash
./psi-compiler -h
```

## PSI Language Syntax

The PSI Language is designed to be simple yet expressive, supporting basic programming constructs.

### Basic Syntax

- **Comments**: Single line comments start with `//`.

  ```psi
  // This is a comment
  ```

- **Variables**: Declared using `var`.

  ```psi
  var x = 10;
  ```

- **Assignments**:

  ```psi
  x = x + 5;
  ```

### Supported Data Types

- **Integer**: Whole numbers.
- **Float**: Decimal numbers.
- **String**: Sequence of characters.
- **Boolean**: `true` or `false`.

### Control Structures

- **If-Else Statements**:

  ```psi
  if (x > 0) {
      // Code block
  } else {
      // Code block
  }
  ```

- **While Loops**:

  ```psi
  while (x < 10) {
      x = x + 1;
  }
  ```

### Functions

- **Function Declaration**:

  ```psi
  function add(a, b) {
      return a + b;
  }
  ```

- **Function Call**:

  ```psi
  var result = add(5, 3);
  ```

## Code Structure

The project is organized into several key directories and files:

```
compiler-for-psi-language/
│
├── src/
│   ├── lexer.c          # Lexical analyzer
│   ├── parser.c         # Syntax analyzer
│   ├── semantic.c       # Semantic analyzer
│   ├── codegen.c        # Code generator
│   ├── optimizer.c      # Code optimizer
│   └── main.c           # Entry point
│
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── semantic.h
│   ├── codegen.h
│   └── optimizer.h
│
├── examples/
│   ├── hello_world.psi
│   └── factorial.psi
│
├── tests/
│   ├── lexer_tests.c
│   ├── parser_tests.c
│   └── semantic_tests.c
│
├── Makefile
└── README.md
```

### Key Components

- **Lexical Analyzer**: Tokenizes input code into meaningful tokens.
- **Parser**: Constructs a syntax tree from tokens and checks for grammatical correctness.
- **Semantic Analyzer**: Validates types and scopes, ensuring program semantics are preserved.
- **Code Generator**: Produces an intermediate representation or directly executable code.
- **Optimizer**: Implements basic optimization techniques to enhance performance.

## Development

### Prerequisites

- **C Compiler**: Ensure a C compiler like GCC is installed.
- **Make**: Required for building the project.

### Compiling the Code

To compile the project manually, use the following command:

```bash
gcc -o psi-compiler src/*.c
```

### Debugging

For debugging purposes, compile with debugging flags:

```bash
make debug
```

This will enable additional debugging information and logging.

## Testing

Unit tests are provided to validate the compiler's functionality.

### Running Tests

Use the following command to run all tests:

```bash
make test
```

### Adding Tests

To add new tests, create a new file in the `tests/` directory and update the `Makefile` to include your test.

## Contributing

Contributions to the Compiler-for-PSI-language project are welcome! If you would like to contribute, please follow these guidelines:

1. Fork the repository and clone it to your local machine.
2. Create a new branch for your feature or bugfix.
3. Write clear and concise commit messages.
4. Open a pull request to the main branch.

For more details, please refer to [CONTRIBUTING.md](CONTRIBUTING.md).

## Acknowledgements

Special thanks to the following resources that made this project possible:

- "Compilers: Principles, Techniques, and Tools" by Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman.
- Various open-source compiler projects for inspiration and guidance.

---

### Examples

Here's a simple PSI program demonstrating variable declaration, conditionals, and loops:

```psi
// PSI Example Program
var count = 10;
var total = 0;

while (count > 0) {
    total = total + count;
    count = count - 1;
}

print(total);
```

### Troubleshooting

- **Compilation Errors**: Ensure you have the necessary compiler installed and that your code follows PSI syntax rules.
- **Runtime Errors**: Check for correct variable initialization and type usage.

### Future Work

- Implement additional optimization techniques.
- Extend language features with advanced data structures and constructs.
- Create a comprehensive standard library for PSI.

### Support

For any issues, questions, or feedback, please open an issue in the GitHub repository or contact [gorkemturkut@hotmail.com](mailto:gorkemturkut@hotmail.com).

---

This README template is designed to provide a comprehensive overview of your project, its structure, and its usage. Feel free to expand on any sections or add new ones to better fit your project's needs.
