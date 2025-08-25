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

add_test(
  NAME test_dltv2fileraw
  COMMAND $<TARGET_FILE:test_dltv2fileraw> 
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_executable(test_dltv2fileraw
    test_dltv2fileraw.cpp
)
target_link_libraries(
  test_dltv2fileraw
  PRIVATE
    GTest::gtest_main
    qdlt
)
