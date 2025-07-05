add_executable(test_dltmessagematcher
    test_dltmessagematcher.cpp
    test_dltctrlmsg.cpp
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
    test_dltctrlmsg.cpp
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

add_executable(test_qdltargument
    test_qdltargument.cpp
)

target_link_libraries(
  test_qdltargument
  PRIVATE
    GTest::gtest_main
    qdlt
)

add_test(
  NAME test_qdltargument
  COMMAND $<TARGET_FILE:test_qdltargument>
)


add_executable(test_parse_dlt_file
    test_parse_dlt_file.cpp
)
target_link_libraries(
  test_parse_dlt_file
  PRIVATE
    GTest::gtest_main
    qdlt
)
add_test(
  NAME test_parse_dlt_file
  COMMAND $<TARGET_FILE:test_parse_dlt_file>
)
file(COPY testfile_10.dlt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

