include(GenerateExportHeader)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

INCLUDE_DIRECTORIES(${DIALLER_INCLUDE_DIR})
MESSAGE(${DIALLER_INCLUDE_DIR})

FILE(GLOB GLOB_HDRS "*.h")
FILE(GLOB GLOB_SRC "*.cpp")

source_group(\\ FILES ${GLOB_HDRS} ${GLOB_SRC})

SET(SRC ${GLOB_HDRS} ${GLOB_SRC})

SET(DIALLER_LIBS dialler ${Boost_LIBRARIES} CACHE STRING "dialler libs.")

add_library(dialler ${SRC})
cotire(dialler)

IF(WIN32)
 target_link_libraries(dialler wsock32 ws2_32 ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
else()
 TARGET_LINK_LIBRARIES(dialler PUBLIC ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

GENERATE_EXPORT_HEADER(dialler
    BASE_NAME dialler
    EXPORT_MACRO_NAME EXPORT
    EXPORT_FILE_NAME exports.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)
