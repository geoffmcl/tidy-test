# tidy-test - root CMakeLists.txt
# CMakeLists.txt, generated gencmake.pl, on 2015/05/20 11:53:57
cmake_minimum_required( VERSION 2.8.8 )

# CMakeScripts or use the ones that come by default with CMake.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

project( tidy-test )

# The version number.
set( tidy-test_MAJOR 5 )
set( tidy-test_MINOR 0 )
set( tidy-test_POINT 0 )

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Set ON to build shared Library (DLL)" OFF )


if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach(warning)
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    # to distinguish between debug and release libs
    set( CMAKE_DEBUG_POSTFIX "d" )
    # list(APPEND extra_LIBS Psapi Shlwapi Dnsapi odbc32 odbccp32)
    list(APPEND extra_LIBS Ws2_32 Winmm)
else()
    # items for unix
    set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-unused-but-set-variable -Wno-unused-variable -Wno-switch")
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

#  TIDY_FOUND          - True if tidy found.
#  TIDY_INCLUDE_DIRS   - where to find tidy.h, etc.
#  TIDY_LIBRARIES      - List of libraries when using tidy.
find_package( Tidy )
if (TIDY_FOUND)
    message(STATUS "*** Tidy found inc ${TIDY_INCLUDE_DIRS} lib ${TIDY_LIBRARIES}")
    include_directories( ${TIDY_INCLUDE_DIRS} )
    list(APPEND add_LIBS ${TIDY_LIBRARIES})
else ()
    message(FATAL_ERROR "*** Tidy NOT found! Maybe set ENV TIDY_ROOT to install location")
endif ()

#  CURL_FOUND          - True if curl found.
#  CURL_INCLUDE_DIRS   - where to find curl/curl.h, etc.
#  CURL_LIBRARIES      - List of libraries when using curl.
#  CURL_VERSION_STRING - the version of curl found (since CMake 2.8.8)
find_package(CURL)
if (CURL_FOUND)
    message(STATUS "*** CURL found inc ${CURL_INCLUDE_DIRS} lib ${CURL_LIBRARIES}")
    include_directories( ${CURL_INCLUDE_DIRS} )
    list( APPEND add_LIBS ${CURL_LIBRARIES} )
    get_filename_component(_LIB_NAME ${CURL_LIBRARIES} NAME)
    set(_MSG "SHARED")
    if ( "${_LIB_NAME}" STREQUAL "libcurl.lib" )
        add_definitions( -DCURL_STATICLIB=1 )
        set(CURL_STATICLIB 1)
        set(_MSG "STATIC")
    endif ()
    message(STATUS "cURL version ${CURL_VERSION_STRING} type ${_MSG}")
else ()
    message(STATUS "*** CURL NOT FOUND")
endif ()

if (CURL_STATICLIB)
    #   ZLIB_INCLUDE_DIRS   - where to find zlib.h, etc.
    #   ZLIB_LIBRARIES      - List of libraries when using zlib.
    #   ZLIB_FOUND          - True if zlib found.
    find_package(ZLIB)
    if (ZLIB_FOUND)
        message(STATUS "*** ZLIB found inc ${ZLIB_INCLUDE_DIRS} lib ${ZLIB_LIBRARIES}")
        include_directories( ${ZLIB_INCLUDE_DIRS} )
        list( APPEND add_LIBS ${ZLIB_LIBRARIES} )
    else ()
        message(FATAL_ERROR "*** ZLIB NOT FOUND. Needed for CURL static!")
    endif ()
endif ()

# configuration file, if needed
# configure_file( ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h )
# add_definitions( -DHAVE_CONFIG_H )
# include_directories( ${CMAKE_BINARY_DIR} )

if(BUILD_SHARED_LIB)
   set(LIB_TYPE SHARED)
   message(STATUS "*** Building DLL library ${LIB_TYPE}")
else()
   message(STATUS "*** Option BUILD_SHARED_LIB is OFF ${LIB_TYPE}")
endif()
# Total libs 0, exes 1
#################################################
##### LIBRARIES 1 #####
#################################################
set(name utillib)
set(dir src/utils)
include_directories( ${dir} )
set(${name}_SRCS 
    ${dir}/sprtf.c
    ${dir}/utils.cxx
    )
set(${name}_HDRS 
    ${dir}/sprtf.h
    ${dir}/utils.hxx
    )
if (WIN32)
    list(APPEND ${name}_SRCS ${dir}/httpio.c)
    list(APPEND ${name}_HDRS ${dir}/httpio.h)
endif ()
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name})
# deal with install, if any...
#install( TARGETS ${name}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib )
#install(FILES ${${name}_HDRS} DESTINATION include)

#################################################
##### EXECUTABLE 1 #####
#################################################

# tidy EXECUTABLE from [src],
# have 1 C/C++ sources, 0 headers
set(name tidy-test)
set(dir src)
set(${name}_SRCS
    ${dir}/tidy.c
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

set(name tidy-opts)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.c
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

set(name test71)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

set(name test226)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

if (WIN32)
    # TODO: Not yet ported to unix
    set(name tidy-url)
    set(dir src)
    set(${name}_SRCS
        ${dir}/${name}.cxx
        )
    add_executable( ${name} ${${name}_SRCS} )
    if (add_LIBS)
        target_link_libraries( ${name} ${add_LIBS} ${extra_LIBS} )
    endif ()
    if (MSVC)
        set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
    endif ()
    # deal with install, if any...
    #install( TARGETS ${name} DESTINATION bin )
endif ()

set(name tidy-tree)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

if (CURL_FOUND)
    set(name htmltidy)
    set(dir src)
    set(${name}_SRCS
        ${dir}/${name}.c
        )
    add_executable( ${name} ${${name}_SRCS} )
    target_link_libraries( ${name} ${add_LIBS}  ${extra_LIBS} )
    if (MSVC)
        set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
    endif ()
    # deal with install, if any...
    #install( TARGETS ${name} DESTINATION bin )

    set(name url2text)
    set(dir src)
    set(${name}_SRCS
        ${dir}/${name}.cxx
        )
    add_executable( ${name} ${${name}_SRCS} )
    target_link_libraries( ${name} ${add_LIBS} ${extra_LIBS} )
    if (MSVC)
        set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
    endif ()
    # deal with install, if any...
    #install( TARGETS ${name} DESTINATION bin )
endif ()

###############################################################
### utiltity/tool to convert leading spaces in a file to tabs
set(name space2tab)
set(dir src/tools)
set(${name}_SRCS
    ${dir}/${name}.c
    )
add_executable( ${name} ${${name}_SRCS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()

################################################################
### build the sample code
set(name sample)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.c
    )
add_executable( ${name} ${${name}_SRCS} )
target_link_libraries( ${name} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()

################################################################
### test-get-options.c
set(name test-get-options)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.c
    )
add_executable( ${name} ${${name}_SRCS} )
target_link_libraries( ${name} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()

################################################################
### tidy-json.cxx
set(name tidy-json)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
target_link_libraries( ${name} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()

################################################################
### test-tidy.cxx - an idea to run tests internally
set(name test-tidy)
set(dir src)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable( ${name} ${${name}_SRCS} )
target_link_libraries( ${name} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()

# eof
