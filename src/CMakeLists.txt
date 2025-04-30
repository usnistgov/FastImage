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
file(WRITE FastImage/api/Version.h "#include <string> \n\n")
file(APPEND FastImage/api/Version.h "/// @file Version.h\n")
file(APPEND FastImage/api/Version.h "/// @author Alexandre Bardakoff - Timothy Blattner\n")
file(APPEND FastImage/api/Version.h "/// @date  09/18/18\n")
file(APPEND FastImage/api/Version.h "/// @brief FastImage version\n\n")
file(APPEND FastImage/api/Version.h "namespace fi{\n")
file(APPEND FastImage/api/Version.h "/// \\namespace fi FastImage namespace\n\n")
file(APPEND FastImage/api/Version.h "// Fast Image Version number\n")
file(APPEND FastImage/api/Version.h "\tconst int MAJOR_VERSION = ${PROJECT_VERSION_MAJOR};\n")
file(APPEND FastImage/api/Version.h "\tconst int MINOR_VERSION = ${PROJECT_VERSION_MINOR};\n")
file(APPEND FastImage/api/Version.h "\tconst int PATCH_VERSION = ${PROJECT_VERSION_PATCH};\n")
file(APPEND FastImage/api/Version.h "\tconst std::string FULL_VERSION = \"${PROJECT_VERSION}\"; // NOLINT\n")
file(APPEND FastImage/api/Version.h "}\n")

# Add a dummy target to make the export work
add_library(FastImage INTERFACE)
target_include_directories(FastImage INTERFACE $<INSTALL_INTERFACE:include>)

# Install the target
install(TARGETS FastImage EXPORT FastImageTargets)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/FastImageConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/FastImageConfig.cmake"
        @ONLY
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FastImageConfig.cmake"
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/FastImage"
)

install(
        EXPORT
        FastImageTargets
        NAMESPACE
        FastImage::
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/FastImage"
        FILE FastImageTargets.cmake
)


install(DIRECTORY FastImage
        DESTINATION include
        COMPONENT headers)

if (BUILD_MAIN)
    add_definitions(-DPROFILE)
    find_package(HTGS REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(OpenCV REQUIRED)

    include_directories(${HTGS_INCLUDE_DIR})
    include_directories(${TIFF_INCLUDE_DIRS})
    include_directories(${OpenCV_INCLUDE_DIRS})

    link_libraries(${TIFF_LIBRARIES})
    link_libraries(${HTGS_LIBRARIES})
    link_libraries(${OpenCV_LIBRARIES})
    file(GLOB_RECURSE INC_ALL "*.hpp" "*.h")

    include_directories(".")
    add_definitions(-DPROFILE)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HTGS_CXX_FLAGS} ${OpenCV_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -Wno-reorder -Wno-unused-parameter")

    if (NOT APPLE AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi")
    endif (NOT APPLE AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    add_executable(main "main.cpp" ${INC_ALL})
endif (BUILD_MAIN)
