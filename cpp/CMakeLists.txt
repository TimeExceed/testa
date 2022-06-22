add_library(prettyprint STATIC
    prettyprint.cpp
)

target_include_directories(prettyprint
PUBLIC
    .
)


add_library(testa STATIC
    testa.cpp
    testa_main.cpp
)

target_link_libraries(testa
    prettyprint
)

target_include_directories(testa
PUBLIC
    .
)
