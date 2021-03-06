include(GenerateExportHeader)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

FILE(GLOB GLOB_HDRS "*.h")
FILE(GLOB GLOB_SRC "*.cpp")

source_group(\\ FILES ${GLOB_HDRS} ${GLOB_SRC})

SET(SRC ${GLOB_HDRS} ${GLOB_SRC})

add_library(dialler  SHARED ${SRC})
target_INCLUDE_DIRECTORIES(dialler PUBLIC ${DIALLER_INCLUDE_DIR})
target_link_libraries(dialler PUBLIC ${DIALLER_PRIVATE_LIBRARIES})
link_directories(${DIALLER_LIBRARY_DIR})

SET(DIALLER_LIBRARIES dialler ${DIALLER_PRIVATE_LIBRARIES} CACHE STRING "dialler libs")

GENERATE_EXPORT_HEADER(dialler
    BASE_NAME dialler
    EXPORT_MACRO_NAME EXPORT
    EXPORT_FILE_NAME exports.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)
