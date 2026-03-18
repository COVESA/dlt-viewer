# QTest-based tests for the dlt-viewer UI layer.
# Each test is a separate executable so failures are isolated and
# ctest reports them individually.

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata")

# ---------------------------------------------------------------------------
# Helper macro: create one QTest executable, link it, register with ctest.
# Usage: add_qtest(target source1 [source2 ...])
# ---------------------------------------------------------------------------
macro(add_qtest TARGET)
    add_executable(${TARGET} ${ARGN})
    target_link_libraries(${TARGET}
        PRIVATE
            dlt-viewer-lib
            ${QT_PREFIX}::Test
    )
    target_compile_definitions(${TARGET}
        PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}"
    )
    add_test(
        NAME    ${TARGET}
        COMMAND $<TARGET_FILE:${TARGET}>
    )
endmacro()

# ---------------------------------------------------------------------------
# Individual test targets
# ---------------------------------------------------------------------------
add_qtest(test_filterdialog     test_filterdialog.cpp)
add_qtest(test_exporterdialog   test_exporterdialog.cpp)
add_qtest(test_tablemodel       test_tablemodel.cpp)
