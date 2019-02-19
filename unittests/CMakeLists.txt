

MESSAGE(STATUS "Unittests")

FILE(GLOB HDRS "*.h")
FILE(GLOB SRC "*.cpp")

add_executable(dialler_unittests ${HDRS} ${SRC})
target_INCLUDE_DIRECTORIES(dialler PUBLIC ${DIALLER_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(dialler_unittests PUBLIC ${DIALLER_LIBS})
add_test(dialler_unittests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests) 
set_target_properties(dialler_unittests PROPERTIES FOLDER Tests)
cotire(dialler_unittests)
