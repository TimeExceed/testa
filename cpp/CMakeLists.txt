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
