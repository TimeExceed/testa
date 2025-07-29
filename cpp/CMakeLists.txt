add_library(prettyprint INTERFACE IMPORTED GLOBAL)
target_include_directories(prettyprint
INTERFACE
    .)

add_library(testa STATIC
    testa.cpp
    testa_main.cpp)
target_include_directories(testa
PUBLIC
    .)
target_link_directories(testa
PUBLIC
    prettyprint)

function(add_executable_testa NAME)
    add_executable(${ARGV})
    add_test(
        NAME    "${NAME}"
        COMMAND "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../runtests.py"
                -l
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../lang.json"
                "${NAME}")
endfunction()
