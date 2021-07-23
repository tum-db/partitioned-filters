# - Find Boost
#
# Copyright (c) 2016 Thiago Barroso Perrotta
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# This module finds if Boost is installed and determines where the
# executables are. It sets the following variables:
#
#  BOOST_FOUND : boolean            - system has Boost
#  BOOST_LIBRARIES : list(filepath) - the libraries needed to use Boost
#  BOOST_INCLUDE_DIRS : list(path)  - the Boost include directories
#
# If Boost is not found, this module downloads it according to the
# following variables:
#
#  BOOST_ROOT_DIR : path                - the Path where Boost will be installed on
#  BOOST_REQUESTED_VERSION : string     - the Boost version to be downloaded
#
# You can also specify its components:
#
#  find_package(Boost COMPONENTS program_options system)
#
# which are stored in Boost_FIND_COMPONENTS : list(string)
#
# You can also specify its behavior:
#
#  BOOST_USE_STATIC_LIBS : boolean (default: OFF)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost ${BOOST_REQUESTED_VERSION} COMPONENTS ${BOOST_FIND_COMPONENTS})

if (Boost_FOUND)

    if (NOT ${BOOST_FIND_COMPONENTS})
        include_directories(${Boost_INCLUDE_DIRS})
        add_library(boost STATIC IMPORTED)
        set_property(TARGET boost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
        foreach (library ${Boost_LIBRARIES})
            set_property(TARGET boost PROPERTY IMPORTED_LOCATION library)
        endforeach ()
    else ()
        add_library(boost INTERFACE)
        set_property(TARGET boost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIR})
    endif ()

else ()

    set(BOOST_USE_STATIC_LIBS false)

    # Set the library prefix and library suffix properly.
    if (BOOST_USE_STATIC_LIBS)
        set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_STATIC_LIBRARY_PREFIX})
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
        set(LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
        set(LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    else ()
        set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_SHARED_LIBRARY_PREFIX})
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
        set(LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif ()

    # Create a list(string) for the build command (e.g. --with-program_options;--with-system)
    # and assigns it to BOOST_COMPONENTS_FOR_BUILD
    foreach (component ${BOOST_FIND_COMPONENTS})
        list(APPEND BOOST_COMPONENTS_FOR_BUILD --with-${component})
    endforeach ()

    if (NOT BOOST_REQUESTED_VERSION)
        message(FATAL_ERROR "BOOST_REQUESTED_VERSION is not defined.")
    endif ()

    string(REPLACE "." "_" BOOST_REQUESTED_VERSION_UNDERSCORE ${BOOST_REQUESTED_VERSION})

    set(BOOST_MAYBE_STATIC)
    if (BOOST_USE_STATIC_LIBS)
        set(BOOST_MAYBE_STATIC "link=static")
    endif ()

    include(ExternalProject)

    # Get gflags
    ExternalProject_Add(
            boost_src
            PREFIX "vendor/boost"
            URL https://downloads.sourceforge.net/project/boost/boost/${BOOST_REQUESTED_VERSION}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.zip
            UPDATE_COMMAND ""
            BUILD_IN_SOURCE true
            CONFIGURE_COMMAND ./bootstrap.sh --prefix=<INSTALL_DIR>
            BUILD_COMMAND ./b2 ${BOOST_MAYBE_STATIC} --prefix=<INSTALL_DIR> ${BOOST_COMPONENTS_FOR_BUILD} install
            INSTALL_COMMAND ""
    )


    ExternalProject_Get_Property(boost_src install_dir)
    set(BOOST_INCLUDE_DIR ${install_dir}/include)
    set(BOOST_LIBRARY_DIR ${install_dir}/lib)

    file(MAKE_DIRECTORY ${BOOST_INCLUDE_DIR})
    if (NOT ${BOOST_FIND_COMPONENTS})
        add_library(boost STATIC IMPORTED)
        set_property(TARGET boost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIR})
        foreach (component ${BOOST_FIND_COMPONENTS})
            set_property(TARGET boost PROPERTY IMPORTED_LOCATION ${BOOST_LIBRARY_DIR}/${LIBRARY_PREFIX}boost_${component}.a)
        endforeach ()
    else ()
        add_library(boost INTERFACE)
        set_property(TARGET boost APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${BOOST_INCLUDE_DIR})
    endif ()

    # Dependencies
    add_dependencies(boost boost_src)
endif ()
