MESSAGE(STATUS "Unittests")

FILE(GLOB HDRS "*.h")
FILE(GLOB SRC "*.cpp")

add_executable(dialler_unittests ${HDRS} ${SRC})
target_INCLUDE_DIRECTORIES(dialler_unittests PUBLIC ${DIALLER_INCLUDE_DIR} ${DIALLER_TEST_INCLUDE_DIR})
target_link_libraries(dialler_unittests PUBLIC ${DIALLER_LIBRARIES})
target_link_directories(dialler_unittests PUBLIC ${DIALLER_LIBRARY_DIR})

add_test(dialler_unittests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests) 
set_target_properties(dialler_unittests PROPERTIES FOLDER Tests)

