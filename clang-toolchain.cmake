# Ustawienia kompilatora Clang
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

# Opcjonalnie: Wymuœ standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opcjonalnie: Dodatkowe flagi dla Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang Compiler")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
