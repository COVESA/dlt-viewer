find_package(GTest REQUIRED)

enable_testing()

add_executable(test_tools
    test_dltmessagematcher.cpp
)
target_link_libraries(
  test_tools
  PRIVATE
    GTest::gtest_main
    tools
)

