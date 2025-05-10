message(STATUS "Copying otr files...")

# Copy and rename mm.zip to mm.o2r
if(NOT ONLY2SHIPOTR AND EXISTS ${SOURCE_DIR}/mm/mm.o2r)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy mm.o2r ${SOURCE_DIR}/mm.o2r)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy mm.o2r ${BINARY_DIR}/mm/mm.o2r)
    message(STATUS "Copied mm.zip")
endif()
if(EXISTS ${SOURCE_DIR}/mm/2ship.o2r)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy 2ship.o2r ${SOURCE_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy 2ship.o2r ${BINARY_DIR}/mm/)
    message(STATUS "Copied 2ship.o2r")
endif()



if(NOT ONLY2SHIPOTR AND (NOT EXISTS ${SOURCE_DIR}/mm.o2r))
    message(FATAL_ERROR "Failed to copy. No O2R files found.")
endif()
if(NOT EXISTS ${SOURCE_DIR}/2ship.o2r)
    message(FATAL_ERROR "Failed to copy. No 2ship O2R found.")
endif()
