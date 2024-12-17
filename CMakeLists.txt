cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spreadsheet)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

add_definitions(
        -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

FetchContent_Declare(
        antlr4
        GIT_REPOSITORY https://github.com/antlr/antlr4.git
        GIT_TAG 4.13.1
)
FetchContent_MakeAvailable(antlr4)

add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/_deps/antlr4-src/runtime/Cpp")

include_directories(
        gen
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/antlr4-src/runtime/Cpp/runtime/src"
)

add_executable(
        spreadsheet
        bail_error_listener.cpp
        cell.cpp
        common.cpp
        utils.cpp
        my_formula.cpp
        main.cpp
        sheet.cpp
        expr_shrink_listener.cpp
        shifted_formula_listener.cpp
        tree_shape_listener.cpp
        referenced_cells_listener.cpp
        gen/FormulaBaseListener.cpp
        gen/FormulaBaseVisitor.cpp
        gen/FormulaParser.cpp
        gen/FormulaLexer.cpp
        gen/FormulaVisitor.cpp
        gen/FormulaListener.cpp
        cell_data.cpp
        error_handle_listener.cpp
        sheet_size_monitor.cpp
)

target_link_libraries(spreadsheet antlr4_static)
