
# NIST-developed software is provided by NIST as a public service.
# You may use, copy and distribute copies of the software in any
# medium, provided that you keep intact this entire notice.  You may
# improve, modify and create derivative works of the software or any
# portion of the software, and you may copy and distribute such
# modifications or works.  Modified works should carry a notice
# stating that you changed the software and should note the date and
# nature of any such change.  Please explicitly acknowledge the
# National Institute of Standards and Technology as the source of the
# software.

# NIST-developed software is expressly provided "AS IS."  NIST MAKES
# NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
# OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# NON-INFRINGEMENT AND DATA ACCURACY.  NIST NEITHER REPRESENTS NOR
# WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR
# ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED.  NIST DOES NOT
# WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
# SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
# CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.

# You are solely responsible for determining the appropriateness of
# using and distributing the software and you assume all risks
# associated with its use, including but not limited to the risks and
# costs of program errors, compliance with applicable laws, damage to
# or loss of data, programs or equipment, and the unavailability or
# interruption of operation.  This software is not intended to be used
# in any situation where a failure could cause risk of injury or
# damage to property.  The software developed by NIST employees is not
# subject to copyright protection within the United States.


# - Find HTGS includes and required compiler flags and library dependencies
# Dependencies: C++11 support and threading library
#
# The HTGS_CXX_FLAGS should be added to the CMAKE_CXX_FLAGS
#
# This module defines
#  HTGS_INCLUDE_DIR
#  HTGS_LIBRARIES
#  HTGS_CXX_FLAGS
#  HTGS_FOUND
#

SET(HTGS_FOUND ON)

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if (COMPILER_SUPPORTS_CXX11)
    set(HTGS_CXX_FLAGS "${HTGS_CXX_FLAGS} -std=c++11")
else()
    if (HTGS_FIND_REQUIRED)
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler for HTGS.")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler for HTGS.")
    endif()

    SET(HTGS_FOUND OFF)
endif()


find_package(Threads QUIET)

if (Threads_FOUND)
    if(CMAKE_USE_PTHREADS_INIT)
        if(NOT APPLE)
            set(HTGS_CXX_FLAGS "${HTGS_CXX_FLAGS} -pthread")
        endif()
    endif(CMAKE_USE_PTHREADS_INIT)

    set(HTGS_LIBRARIES "${HTGS_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}")

else()
    if (HTGS_FIND_REQUIRED)
        message(FATAL_ERROR "Unable to find threads. HTGS must have a threading library i.e. pthreads.")
    else()
        message(STATUS "Unable to find threads. HTGS must have a threading library i.e. pthreads.")
    endif()
    SET(HTGS_FOUND OFF)
endif()


FIND_PATH(HTGS_INCLUDE_DIR htgs/api/TaskGraphConf.hpp
        /usr/include
        /usr/local/include
        )

#    Check include files
IF (NOT HTGS_INCLUDE_DIR)
    SET(HTGS_FOUND OFF)
    MESSAGE(STATUS "Could not find HTGS includes. HTGS_FOUND now off")
ENDIF ()

IF (HTGS_FOUND)
    IF (NOT HTGS_FIND_QUIETLY)
        MESSAGE(STATUS "Found HTGS include: ${HTGS_INCLUDE_DIR}")
    ENDIF (NOT HTGS_FIND_QUIETLY)
ELSE (HTGS_FOUND)
    IF (HTGS_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find HTGS header files")
    ENDIF (HTGS_FIND_REQUIRED)
ENDIF (HTGS_FOUND)

string(STRIP ${HTGS_LIBRARIES} HTGS_LIBRARIES)
string(STRIP ${HTGS_CXX_FLAGS} HTGS_CXX_FLAGS)


MARK_AS_ADVANCED(HTGS_INCLUDE_DIR)
