add_executable(test_dltmessagematcher
    test_dltmessagematcher.cpp
)
target_link_libraries(
  test_dltmessagematcher
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_dltmessagematcher
  COMMAND $<TARGET_FILE:test_dltmessagematcher>
)


add_executable(test_dltoptmanager
    test_dltoptmanager.cpp
)

target_link_libraries(
  test_dltoptmanager
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_dltoptmanager
  COMMAND $<TARGET_FILE:test_dltoptmanager>
)
