# QTC BLAKE3 Library CMake Configuration

add_library(blake3 STATIC
    blake3.cpp
    blake3.h
)

target_include_directories(blake3
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Optimization flags for BLAKE3 performance
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(blake3 PRIVATE -O3 -march=native)
endif()

# Export for main QTC build
set_target_properties(blake3 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 17
)
