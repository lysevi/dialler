cmake_minimum_required (VERSION 3.1)
set(PROJECT "dialler")
project (${PROJECT} VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
set(BOOST_DEP system date_time)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

IF(WIN32 AND MSVC)
  find_package(Boost 1.60.0 COMPONENTS ${BOOST_DEP} REQUIRED)
else()
  set(BOOST_DEP ${BOOST_DEP})
  find_package(Boost  REQUIRED ${BOOST_DEP})
ENDIF()

IF(Boost_FOUND)
else()	
  MESSAGE(FATAL_ERROR "boost not found")
  EXIT()
ENDIF()

##### THREADS
find_package (Threads)

##### MISC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(dialler_ENABLE_TESTS)
   set (DIALLER_TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/Catch2/)
endif(dialler_ENABLE_TESTS)

#########
SET(DIALLER_LIBRARY_DIR ${Boost_LIBRARY_DIRS} CACHE STRING "dialler lib dir.")
SET(DIALLER_INCLUDE_DIR   ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR} CACHE STRING "dialler include dir.")


IF(WIN32)
 SET(DIALLER_PRIVATE_LIBRARIES  ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32 CACHE STRING "dialler libraries.")
else()
  SET(DIALLER_PRIVATE_LIBRARIES ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} CACHE STRING "dialler libraries.")
endif()


add_subdirectory(libdialler)

if (dialler_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(unittests)
endif()
