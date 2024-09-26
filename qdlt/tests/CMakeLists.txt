add_executable(test_tools
    test_dltmessagematcher.cpp
)
target_link_libraries(
  test_tools
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_tools
  COMMAND $<TARGET_FILE:test_tools>
)

