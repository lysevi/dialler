cmake_minimum_required (VERSION 3.1)
set(PROJECT "dialler")
project (${PROJECT} VERSION 0.0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
	add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_definitions(-DBOOST_ASIO_NO_DEPRECATED)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
 
IF(NOT CMAKE_BUILD_TYPE)
        message(STATUS "CMAKE_BUILD_TYPE is not set, set to default = RELWITHDEBINFO")
        SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
ENDIF()

MESSAGE(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

option(dialler_ENABLE_TESTS "Enable testing." ON)
option(dialler_ASIO_DEBUG "Enable asio debug flag." OFF)

MESSAGE(STATUS "Options: ")
MESSAGE(STATUS "dialler_ENABLE_TESTS - " ${dialler_ENABLE_TESTS})
MESSAGE(STATUS "dialler_ASIO_DEBUG - " ${dialler_ASIO_DEBUG})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(AddCXXCompilerFlag)
include(CompillerSettings)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

IF(dialler_ASIO_DEBUG)
 add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
ENDIF()

########## BOOST
IF(NOT HEADER_ONLY)
    list(FIND CONAN_DEFINES_BOOST "-DBOOST_USE_STATIC_LIBS" IS_STATIC)
    if(${IS_STATIC} GREATER -1)
        set(Boost_USE_STATIC_LIBS ON)
    else()
        set(Boost_USE_STATIC_LIBS OFF)
	endif()
endif()

find_package(Boost 1.69.0 COMPONENTS system date_time REQUIRED)

IF(Boost_FOUND)
else()	
  MESSAGE(FATAL_ERROR "boost not found")
  EXIT()
ENDIF()

##### THREADS
find_package (Threads)

##### MISC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#########
SET(DIALLER_LIBRARY_DIR ${Boost_LIBRARY_DIRS})
SET(DIALLER_INCLUDE_DIR   ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR} )


IF(WIN32)
 SET(DIALLER_PRIVATE_LIBRARIES  ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32 )
else()
  SET(DIALLER_PRIVATE_LIBRARIES ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
endif()


add_subdirectory(libdialler)

if (dialler_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(unittests)
endif()
