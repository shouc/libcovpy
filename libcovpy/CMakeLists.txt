include_directories(include/)
add_library(cov SHARED coverage.c)
target_link_libraries(cov rt)
