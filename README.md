# cini

`cini` is a command-line utility that bootstraps a new C/C++ project by creating a predefined directory structure, generating build system files, and optionally initializing a Git repository, setting up documentation, and configuring testing.

## Installation

```bash
git clone https://github.com/akvachan/cini.git && cd cini
c++ -std=c++23 cini.cpp -o cini
sudo cp cini /usr/local/bin/
```

## Warning
Tool is not properly tested, experimental and intended for personal use only.

## Usage
Provide the project name as the first argument, followed by optional parameters in order:

```sh
<project_name> <linking> <language> <compiler> <strict_compiler> <build_system> <init_git_repo> <documentation> <test>
```

- **`<project_name>`** (required) - Name of the project.
- **`<linking>`** - `static` (default) or `dynamic`.
- **`<language>`** - `c++` (default) or `c`.
- **`<compiler>`** - Compiler to use (default: `clang++`).
- **`<strict_compiler>`** - Strictness level (`0`, `1` (default), `2`).
- **`<build_system>`** - `cmake` (default) or `make`.
- **`<init_git_repo>`** - `yes` (default) or `no`.
- **`<documentation>`** - `yes` (default) or `no`.
- **`<test>`** - `no` (default) or `yes`.

Alternatively, options can be provided via flags:

```sh
--name=<project_name>       # Project name (required)
--link=<static|dynamic>     # Linking type (default: static)
--lang=<c++|c>             # Language (default: c++)
--compiler=<compiler>      # Compiler to use (default: clang++)
--strict=<0|1|2>          # Strictness level (default: 1)
--build=<cmake|make>     # Build system (default: cmake)
--git                   # Initialize a Git repository (default: yes)
--docs                  # Generate documentation files (default: yes)
--test                  # Set up a testing framework (default: no)
```

Any omitted flag will use the default value specified above. The project name must be provided either as the first positional argument or with the `--name` flag.

## Mini docs

### `--name=<project_name>`
Specifies the name of the project.

### `--link=<static|dynamic>`
Chooses the type of linking:
- `static` (default) - Static linking.
- `dynamic` - Shared linking.

### `--lang=<c++|c>`
Determines the programming language. Defaults to `c++`.

### `--compiler=<compiler>`
Specifies the compiler; defaults to `clang++`.

### `--strict=<0|1|2>`
Sets the strictness level:
- `0` - Disables extra warnings/sanitizers.
- `1` - Enables basic warnings (default).
- `2` - Enables extra warnings and address sanitization.

### `--build=<cmake|make>`
Selects the build system. Default is `cmake`.

### `--git`
Initializes a Git repository in the project directory.

### `--docs`
Generates documentation files (e.g., a Doxyfile) for the project.

### `--test`
Creates a test directory with a sample test file for basic assertions.

## Examples

### Example 1:
Initialize a new project called `MyProject` using flagged mode with defaults:
```sh
cini --name=MyProject
```

### Example 2:
Initialize a project in positional mode with explicit options:
```sh
cini MyProject dynamic c++ gcc 2 make yes yes no
```
