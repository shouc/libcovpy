add_executable(test test.cpp)
target_link_libraries(test rt)
target_compile_options(test PRIVATE -fsanitize-coverage=trace-pc-guard)
