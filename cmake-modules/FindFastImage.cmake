# NIST-developed software is provided by NIST as a public service.
# You may use, copy and distribute copies of the  software in any  medium,
# provided that you keep intact this entire notice. You may improve,
# modify and create derivative works of the software or any portion of the
# software, and you may copy and distribute such modifications or works.
# Modified works should carry a notice stating that you changed the software
# and should note the date and nature of any such change. Please explicitly
# acknowledge the National Institute of Standards and Technology as the
# source of the software.
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY
# OF ANY KIND, EXPRESS, IMPLIED, IN FACT  OR ARISING BY OPERATION OF LAW,
# INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST
# NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION  OF THE SOFTWARE WILL
# BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST
# DOES NOT WARRANT  OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
# SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
# CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
# You are solely responsible for determining the appropriateness of using
# and distributing the software and you assume  all risks associated with
# its use, including but not limited to the risks and costs of program
# errors, compliance  with applicable laws, damage to or loss of data,
# programs or equipment, and the unavailability or interruption of operation.
# This software is not intended to be used in any situation where a failure
# could cause risk of injury or damage to property. The software developed
# by NIST employees is not subject to copyright protection within
# the United States.

# - Find HTGS includes and required compiler flags and library dependencies
# Dependencies: C++14 support and threading library
#
# The FastImage_CXX_FLAGS should be added to the CMAKE_CXX_FLAGS
#
# This module defines
#  FastImage_INCLUDE_DIR
#  FastImage_LIBRARIES
#  FastImage_CXX_FLAGS
#  FastImage_FOUND
#

SET(FastImage_FOUND ON)

include(CheckCXXCompilerFlag)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_XX_STANDARD_REQUIRED ON)

# Compiler Test
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX11)
if (NOT COMPILER_SUPPORTS_CXX11)
    if (FastImage_FIND_REQUIRED)
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler for FastImage.")
    else ()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler for FastImage.")
    endif ()
    SET(FastImage_FOUND OFF)
endif ()

#Chech FastImage dependencies
if (FastImage_FIND_QUIETLY)
    find_package(HTGS QUIET)
    if (HTGS_FOUND)
        set(FastImage_LIBRARIES "${FastImage_LIBRARIES}${HTGS_LIBRARIES}")
        set(FastImage_CXX_FLAGS "${FastImage_CXX_FLAGS}${HTGS_CXX_FLAGS}")
        set(FastImage_INCLUDE_DIR "${FastImage_INCLUDE_DIR}" "${HTGS_INCLUDE_DIR}")
    else ()
        message(STATUS "HTGS library has not been found, please install it to use FastImage.")
        SET(FastImage_FOUND OFF)
    endif (HTGS_FOUND)

    find_package(TIFF QUIET)
    if (TIFF_FOUND)
        set(FastImage_LIBRARIES "${FastImage_LIBRARIES} ${TIFF_LIBRARIES}")
        set(FastImage_CXX_FLAGS "${FastImage_CXX_FLAGS} ${TIFF_CXX_FLAGS}")
        set(FastImage_INCLUDE_DIR "${FastImage_INCLUDE_DIR}" "${TIFF_INCLUDE_DIR}")
    else ()
        message(STATUS "libtiff library has not been found, please install it to use FastImage.")
        SET(FastImage_FOUND OFF)
    endif (TIFF_FOUND)
else ()
    find_package(HTGS REQUIRED)
    if (HTGS_FOUND)
        set(FastImage_LIBRARIES "${FastImage_LIBRARIES} ${HTGS_LIBRARIES}")
        set(FastImage_CXX_FLAGS "${FastImage_CXX_FLAGS} ${HTGS_CXX_FLAGS}")
        set(FastImage_INCLUDE_DIR "${FastImage_INCLUDE_DIR}" "${HTGS_INCLUDE_DIR}")
    else ()
        message(FATAL_ERROR "HTGS library has not been found, please install it to use FastImage.")
        SET(FastImage_FOUND OFF)
    endif (HTGS_FOUND)

    find_package(TIFF REQUIRED)
    if (TIFF_FOUND)
        set(FastImage_LIBRARIES "${FastImage_LIBRARIES} ${TIFF_LIBRARIES}")
        set(FastImage_CXX_FLAGS "${FastImage_CXX_FLAGS} ${TIFF_CXX_FLAGS}")
        set(FastImage_INCLUDE_DIR "${FastImage_INCLUDE_DIR}" "${TIFF_INCLUDE_DIR}")
    else ()
        message(FATAL_ERROR "libtiff library has not been found, please install it to use FastImage.")
        SET(FastImage_FOUND OFF)
    endif (TIFF_FOUND)
endif (FastImage_FIND_QUIETLY)

#    Check include files
FIND_PATH(FastImage_INCLUDE_DIR FastImage/api/FastImage.h
        /usr/include
        /usr/local/include
        )

IF (NOT FastImage_INCLUDE_DIR)
    SET(FastImage_FOUND OFF)
    MESSAGE(STATUS "Could not find FastImage includes. FastImage_FOUND now off")
ENDIF ()

IF (FastImage_FOUND)
    IF (NOT FastImage_FIND_QUIETLY)
        MESSAGE(STATUS "Found FastImage include: ${FastImage_INCLUDE_DIR}")
    ENDIF (NOT FastImage_FIND_QUIETLY)
ELSE (FastImage_FOUND)
    IF (FastImage_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find FastImage header files")
    ENDIF (FastImage_FIND_REQUIRED)
ENDIF (FastImage_FOUND)

string(STRIP ${FastImage_LIBRARIES} FastImage_LIBRARIES)
string(STRIP ${FastImage_CXX_FLAGS} FastImage_CXX_FLAGS)

MARK_AS_ADVANCED(FastImage_INCLUDE_DIR)