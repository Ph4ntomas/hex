cmake_minimum_required(VERSION 3.21)

set(CMAKE_CTEST_ARGUMENTS --verbose)
include(CTest)

find_package(Criterion)

add_executable(Hex_sparse_array_tests)

target_sources(Hex_sparse_array_tests
    PRIVATE
    hex/containers/sparse_array.cpp
)

target_include_directories(Hex_sparse_array_tests
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/srcs>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            ${CRITERION_INCLUDE_DIRS}
)

target_compile_features(Hex_sparse_array_tests PRIVATE cxx_std_17)

target_compile_options(
    Hex_sparse_array_tests
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:Clang>>:--coverage>
            $<$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:Clang>>:-fprofile-arcs>
)

target_link_libraries(Hex_sparse_array_tests 
    PRIVATE ${CRITERION_LIBRARIES}
)

target_link_options(Hex_sparse_array_tests 
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:Clang>>:--coverage>
)

add_test(NAME Hex_sparse_array_tests COMMAND Hex_sparse_array_tests --verbose)
