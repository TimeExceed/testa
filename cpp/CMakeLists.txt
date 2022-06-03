add_library(prettyprint STATIC
    prettyprint.cpp
)

add_library(testa STATIC
    testa.cpp
    testa_main.cpp
)

target_link_libraries(testa
    prettyprint
)

