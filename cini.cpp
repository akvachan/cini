#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

// Helper function to check for a prefix in a string
bool startsWith(const std::string &str, const std::string &prefix) {
  return str.substr(0, prefix.size()) == prefix;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage (positional): cini <project_name> [linking] [language] "
                 "[compiler] [strict_compiler] [build_system] [init_git_repo] "
                 "[documentation] [test]\n";
    std::cerr << "Or use flagged mode:\n";
    std::cerr << "  cini [project_name or --name=<name>] [--link=static|dynamic] "
                 "[--lang=c++|c] [--compiler=clang++|...]\n";
    std::cerr << "       [--strict=0|1|2] [--build=cmake|make] [--git] [--docs] [--test]\n";
    return 1;
  }

  // Defaults (for both positional and flagged modes)
  std::string projectArg = "";
  std::string linking = "static";
  std::string language = "c++";
  std::string compiler = "clang++"; // default compiler is now clang++
  int strict_compiler = 1;          // default is 1; 2 adds extra warnings & sanitizers
  std::string build_system = "cmake";
  bool init_git_repo = true;        // default: initialize git repo
  bool documentation = true;        // default: generate documentation files
  bool test_flag = false;           // default: no tests

  // Determine if any argument is flagged (starts with "--")
  bool flaggedMode = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (startsWith(arg, "--"))
      flaggedMode = true;
  }

  if (flaggedMode) {
    // Flagged mode parsing: defaults remain as above if flags are omitted.
    for (int i = 1; i < argc; ++i) {
      std::string arg(argv[i]);
      if (startsWith(arg, "--name=")) {
        projectArg = arg.substr(7);
      } else if (startsWith(arg, "--link=")) {
        linking = arg.substr(7);
      } else if (startsWith(arg, "--lang=")) {
        language = arg.substr(7);
      } else if (startsWith(arg, "--compiler=")) {
        compiler = arg.substr(11);
      } else if (startsWith(arg, "--strict=")) {
        strict_compiler = std::stoi(arg.substr(9));
      } else if (startsWith(arg, "--build=")) {
        build_system = arg.substr(8);
      } else if (arg == "--git") {
        init_git_repo = true;
      } else if (arg == "--docs") {
        documentation = true;
      } else if (arg == "--test") {
        test_flag = true;
      } else if (!startsWith(arg, "--") && projectArg.empty()) {
        // First non-flag argument as project name if --name was not provided.
        projectArg = arg;
      }
    }
    if (projectArg.empty()) {
      std::cerr << "Error: project name must be specified (either as a "
                   "positional argument or with --name=<name>).\n";
      return 1;
    }
  } else {
    // Positional mode parsing
    projectArg = argv[1];
    if (argc > 2)
      linking = argv[2];
    if (argc > 3)
      language = argv[3];
    if (argc > 4)
      compiler = argv[4];
    if (argc > 5)
      strict_compiler = std::stoi(argv[5]); // default 1 if missing
    if (argc > 6)
      build_system = argv[6];
    if (argc > 7)
      init_git_repo = (std::string(argv[7]) == "yes");
    else
      init_git_repo = true; // default "yes" in positional mode
    if (argc > 8)
      documentation = (std::string(argv[8]) == "yes");
    else
      documentation = true; // default "yes"
    if (argc > 9)
      test_flag = (std::string(argv[9]) == "yes");
    else
      test_flag = false; // default "no"
  }

  // Determine project directory and name:
  fs::path projectPath;
  std::string projectName;
  if (projectArg == ".") {
    projectPath = fs::current_path();
    projectName = projectPath.filename().string();
  } else if (projectArg.find('/') != std::string::npos ||
             projectArg.find('\\') != std::string::npos) {
    projectPath = fs::path(projectArg);
    projectName = projectPath.filename().string();
    if (!fs::exists(projectPath)) {
      fs::create_directories(projectPath);
    }
  } else {
    projectName = projectArg;
    projectPath = fs::current_path() / projectName;
    fs::create_directory(projectPath);
  }

  // Create project directories: src, inc, vendor, build
  fs::create_directories(projectPath / "src");
  fs::create_directories(projectPath / "inc");
  fs::create_directories(projectPath / "vendor");
  fs::create_directories(projectPath / "build");

  // Create main file in src/ (main.cpp for C++ or main.c for C)
  fs::path mainFile;
  if (language == "c") {
    mainFile = projectPath / "src" / "main.c";
    std::ofstream mainStream(mainFile);
    mainStream << "#include <stdio.h>\n\n"
               << "int main() {\n"
               << "    printf(\"Hello, World!\\n\");\n"
               << "    return 0;\n"
               << "}\n";
    mainStream.close();
  } else { // default is C++
    mainFile = projectPath / "src" / "main.cpp";
    std::ofstream mainStream(mainFile);
    mainStream << "#include <iostream>\n\n"
               << "int main() {\n"
               << "    std::cout << \"Hello, World!\" << std::endl;\n"
               << "    return 0;\n"
               << "}\n";
    mainStream.close();
  }

  // Prepare linking flag (if dynamic linking is chosen, add -shared)
  std::string linkingFlag = "";
  if (linking == "dynamic") {
    linkingFlag = " -shared";
  }

  // Create build system files in the project root.
  if (build_system == "make") {
    // Generate a Makefile with targets: build, run, and clear.
    fs::path makefile = projectPath / "Makefile";
    std::ofstream makeStream(makefile);
    std::string srcFile = (language == "c" ? "main.c" : "main.cpp");
    std::string flags = "";
    if (strict_compiler == 1) {
      flags = " -Wall";
    } else if (strict_compiler == 2) {
      flags = " -Wall -Wextra -pedantic -fsanitize=address";
    }
    
    // Build command including debug symbols (-g) and proper standard flag:
    std::string compileCommand;
    if (language == "c") {
      compileCommand = compiler + flags + linkingFlag + " -g -std=c11 -o build/" +
                       projectName + " src/" + srcFile;
    } else {
      compileCommand = compiler + flags + linkingFlag + " -g -std=c++23 -o build/" +
                       projectName + " src/" + srcFile;
    }
    
    // Write the Makefile with the desired targets.
    makeStream << ".PHONY: build run clear\n\n";
    makeStream << "build:\n\t" << compileCommand << "\n\n";
    makeStream << "run: build\n\t./build/" << projectName << "\n\n";
    makeStream << "clear:\n\t@rm -f build/" << projectName << "\n";
    makeStream.close();
  } else { // cmake build system
    // Create CMakeLists.txt in the project root.
    fs::path cmakeFile = projectPath / "CMakeLists.txt";
    std::ofstream cmakeStream(cmakeFile);
    cmakeStream << "cmake_minimum_required(VERSION 3.10)\n";
    cmakeStream << "project(" << projectName << ")\n";
    if (language == "c") {
      cmakeStream << "enable_language(C)\n";
    } else {
      cmakeStream << "enable_language(CXX)\n";
    }
    cmakeStream << "set(CMAKE_BUILD_TYPE Debug)\n";
    // Export compile_commands.json for IDEs/LSPs
    cmakeStream << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n";
    // Set compiler flags based on strictness:
    if (language == "c") {
      if (strict_compiler == 1) {
        cmakeStream << "set(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} -Wall\")\n";
      } else if (strict_compiler == 2) {
        cmakeStream << "set(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -fsanitize=address\")\n";
      }
      cmakeStream << "set(CMAKE_C_STANDARD 11)\n";
      cmakeStream << "set(CMAKE_C_STANDARD_REQUIRED ON)\n";
    } else {
      if (strict_compiler == 1) {
        cmakeStream << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wall\")\n";
      } else if (strict_compiler == 2) {
        cmakeStream << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=address\")\n";
      }
      cmakeStream << "set(CMAKE_CXX_STANDARD 23)\n";
      cmakeStream << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    }
    // Add executable target.
    std::string srcFile = (language == "c" ? "src/main.c" : "src/main.cpp");
    cmakeStream << "add_executable(" << projectName << " " << srcFile << ")\n";
    // Include inc/ directory
    cmakeStream << "target_include_directories(" << projectName << " PRIVATE inc)\n";
    // Set dynamic linking if requested.
    if (linking == "dynamic") {
      cmakeStream << "set_target_properties(" << projectName << " PROPERTIES LINK_FLAGS \"-shared\")\n";
    }
    // If tests are enabled (and language is C++), add testing
    if (language != "c" && test_flag) {
      cmakeStream << "enable_testing()\n";
      cmakeStream << "add_subdirectory(test)\n";
    }
    cmakeStream.close();
  }

  // Create test directory and sample test file if language is C++ and tests enabled.
  if (language != "c" && test_flag) {
    fs::create_directories(projectPath / "test");
    fs::path testFile = projectPath / "test" / "test.cpp";
    std::ofstream testStream(testFile);
    testStream << "#include <iostream>\n"
               << "#include <cassert>\n\n"
               << "int main() {\n"
               << "    // Sample test: basic assertion\n"
               << "    assert(1 == 1);\n"
               << "    std::cout << \"Test passed!\" << std::endl;\n"
               << "    return 0;\n"
               << "}\n";
    testStream.close();
  }

  // Initialize git repository if requested.
  if (init_git_repo) {
    std::string command = "cd " + projectPath.string() + " && git init";
    system(command.c_str());
    // Create a .gitignore with common C/C++ ignores.
    fs::path gitignoreFile = projectPath / ".gitignore";
    std::ofstream gitignoreStream(gitignoreFile);
    gitignoreStream << "# Compiled object files\n"
                    << "*.o\n\n"
                    << "# Precompiled Headers\n"
                    << "*.gch\n\n"
                    << "# Libraries\n"
                    << "*.lib\n"
                    << "*.a\n"
                    << "*.so\n\n"
                    << "# Executables\n"
                    << "build/\n\n"
                    << "# CMake Files\n"
                    << "CMakeFiles/\n"
                    << "CMakeCache.txt\n"
                    << "cmake_install.cmake\n"
                    << "Makefile\n";
    gitignoreStream.close();
  }

  // Create a README with build instructions and tips.
  fs::path readmeFile = projectPath / "README.md";
  std::ofstream readmeStream(readmeFile);
  readmeStream << "# " << projectName << "\n\n"
               << "## Build Instructions\n\n";
  if (build_system == "make") {
    readmeStream << "To compile the project using Make, run:\n\n"
                 << "```\nmake build\n```\n\n"
                 << "To run the project, run:\n\n"
                 << "```\nmake run\n```\n\n"
                 << "To clean the built binary, run:\n\n"
                 << "```\nmake clear\n```\n\n";
  } else {
    readmeStream << "To compile the project using CMake, run:\n\n"
                 << "```\ncmake -B build\ncmake --build build\n```\n\n"
                 << "To run the project, run:\n\n"
                 << "```\n./build/" << projectName << "\n```\n\n";
  }
  if (documentation && language != "c") {
    readmeStream << "## Documentation\n\n"
                 << "Generate documentation with Doxygen:\n\n"
                 << "```\ndoxygen Doxyfile\n```\n\n";
  }
  if (language != "c" && test_flag) {
    readmeStream << "## Running Tests\n\n"
                 << "Run tests with:\n\n"
                 << "```\nctest\n```\n\n";
  }
  readmeStream.close();

  // Create Doxyfile for documentation if enabled and language is C++
  if (documentation && language != "c") {
    fs::path doxyfile = projectPath / "Doxyfile";
    std::ofstream doxyStream(doxyfile);
    doxyStream << "PROJECT_NAME = \"" << projectName << "\"\n"
               << "INPUT = src/main.cpp\n"
               << "OUTPUT_DIRECTORY = docs\n"
               << "GENERATE_LATEX = NO\n";
    doxyStream.close();
  }

  std::cout << "Project '" << projectName << "' initialized successfully at "
            << projectPath.string() << std::endl;
  return 0;
}
