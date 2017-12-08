set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 3.5)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# Name of project
project (SimultIte)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

set(doxy_main_page ${CMAKE_SOURCE_DIR}/README.md)

# Generate documentation
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

find_package(OpenCL REQUIRED)
find_package(MPI REQUIRED)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)
    message(STATUS "Downloading clSPARSE library")
    execute_process(COMMAND git clone https://github.com/clMathLibraries/clSPARSE.git ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE OUTPUT_FILE /dev/null)
    execute_process(COMMAND cmake ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE -B${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/ -DBUILD_gMock=NO -DGMOCK_ROOT=/dev/null)
else(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)
    message(STATUS "clSPARSE source files found")
endif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)
set(clSPARSE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclsparseTimer.so)

message(STATUS "Compiling clSPARSE")
execute_process(COMMAND make 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/)


include_directories(
    header
    lib/header
    OpenCL_INCLUDE_DIRS
)

# Declaration of executables (main file + postprocess)
add_executable(
	SimultIte
	src/main.c
    src/executable_options.c
    src/matrix_reader.c
    lib/src/mmio.c
)

# Linkage
target_link_libraries(
	SimultIte
    ${OpenCL_LIBRARIES}
    ${MPI_LIBRARIES}
    ${clSPARSE_LIBRARIES}
)
