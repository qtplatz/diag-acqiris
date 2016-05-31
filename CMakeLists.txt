#
cmake_minimum_required (VERSION 2.8.9)

project ( diag-acqiris )

if ( WIN32 )
  include(CheckSymbolExists)

  check_symbol_exists( "_M_AMD64" "" RTC_ARCH_X64 )
  if ( NOT RTC_ARCH_X64 )
    check_symbol_exists( "_M_X64" "" RTC_ARCH_X64 )
  endif()
  if ( NOT RTC_ARCH_X64 )
    check_symbol_exists( "_M_IX86" "" RTC_ARCH_X86 )
  endif()

  ## lookup boost install directory
  find_path( _boost NAMES boost HINTS "C:/Boost/include/boost-1_59" )
  if ( NOT _boost )
    message( FATAL_ERROR "boost not found" )
  endif()
  set( BOOST_ROOT ${_boost} )
  if ( RTC_ARCH_X64 )
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib" )
  else()
    set( Boost_LIBRARY_DIR "C:/Boost/lib" )    
  endif()
  set( Boost_INCLUDE_DIR ${_boost} )
  set( Boost_USE_STATIC_LIBS ON )

  ## lookup Acqiris install cirectory
  find_path( _acqirisDir NAMES include/AcqirisD1Import.h HINTS
    "$ENV{PROGRAMFILES}" "$ENV{PROGRAMFILES} (x86)" )
  if ( NOT _acqirisDir )
    message( FATAL_ERROR ${_acqirisDir} )
  endif()

  message( STATUS "Found Acqiris in " ${_acqirisDir} )
  set( ACQRS_INCLUDE_DIRS ${_acqirisDir}/include )
  set( ACQRS_LIBRARY_DIRS ${_acqirisDir}/lib )

  add_library( AqDrv4 SHARED IMPORTED )
  #set_target_properties( AqDrv4 PROPERTIES IMPORTED_IMPLIB "${_acqirisDir}/lib/AqDrv4_x64.lib" )
  set_target_properties( AqDrv4 PROPERTIES IMPORTED_IMPLIB "${_acqirisDir}/lib/AqDrv4.lib" )

  add_definitions( -D_ACQIRIS )
  add_definitions( -wd4996 )
  
else()
  set( BOOST_VERSION boost-1_59 )
  set( Boost_INCLUDE_DIR "/usr/local/${BOOST_VERSION}/include")
  set( Boost_LIBRARY_DIR "/usr/local/${BOOST_VERSION}/lib")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  add_definitions( -D_LINUX -D_ACQIRIS )
  set( oslibs dl )
endif()

find_package(Boost 1.57 REQUIRED COMPONENTS thread system date_time filesystem )

include_directories( ${Boost_INCLUDE_DIRS} ${ACQRS_INCLUDE_DIRS} )
link_directories( ${AQRS_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} )

set( SOURCES main.cpp )

add_executable( ${PROJECT_NAME} main.cpp acqiris.cpp acqiris.hpp )

target_link_libraries( ${PROJECT_NAME} AqDrv4 ${Boost_LIBRARIES} ${oslibs} )

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications )

message( STATUS "---------- SUMMARY ------------" )
message( STATUS "# Boost_INCLUDE_DIRS    : " ${Boost_INCLUDE_DIRS} )
message( STATUS "# Boost_LIBRARY_DIRS    : " ${Boost_LIBRARY_DIRS} )
message( STATUS "# ACQRS_INCLUDE_DIRS    : " ${ACQRS_INCLUDE_DIRS} )
message( STATUS "# ACQRS_LIBRARY_DIRS    : " ${ACQRS_LIBRARY_DIRS} )
message( STATUS "---------- END SUMMARY ------------" )



