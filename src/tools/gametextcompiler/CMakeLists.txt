set(CMAKE_CXX_STANDARD 17)

set(GAMETEXTCOMPILER_SOURCES
    src/commands.cpp
    src/commands.h
    src/log.cpp
    src/log.h
    src/main.cpp
    src/processor.cpp
    src/processor.h
)

set(GAMETEXTCOMPILER_INCLUDES src)

add_executable(gametextcompiler ${GAMETEXTCOMPILER_SOURCES})
target_include_directories(gametextcompiler PRIVATE ${GAMETEXTCOMPILER_INCLUDES})
target_link_libraries(gametextcompiler PRIVATE thyme_lib)
