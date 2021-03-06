set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 3.5)

# Name of project
project (SimultIte)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)


OPTION(DOUBLE_PRECISION "Enable Double Precision (Warning: check first that your system supports it)" OFF)
OPTION(COMPILE_CLSPARSE "Set whether to recompile clSPARSE" ON)

IF(DOUBLE_PRECISION)
    ADD_DEFINITIONS(-DDOUBLE_PRECISION)
ENDIF(DOUBLE_PRECISION)

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

message(STATUS "Cleaning up previous files...")
execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/
      ERROR_FILE /dev/null)
execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/
      ERROR_FILE /dev/null)



if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)
    message(STATUS "Downloading clSPARSE library")
    execute_process(COMMAND git clone https://github.com/NicolasDerumigny/clSPARSE.git ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/
        ERROR_FILE /dev/null)
else(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)
    message(STATUS "clSPARSE source files found")
endif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE)

if (COMPILE_CLSPARSE)
    message(STATUS "Compiling clSPARSE")
    execute_process(COMMAND cmake ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE -B${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/)
    execute_process(COMMAND make 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/)
endif()

set(clSPARSE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/src/include/ ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/clSPARSE-build/library/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin)
execute_process(COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/clSPARSE-build/library/libclSPARSE.so ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so ERROR_FILE /dev/null)
execute_process(COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/build/clSPARSE-build/library/libclSPARSE.so.1 ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so.1 ERROR_FILE /dev/null)
set(clSPARSE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so)

include_directories(
    header
    lib/header
    ${OpenCL_INCLUDE_DIRS}
    ${clSPARSE_INCLUDE_DIRS}
    ${MPI_C_INCLUDE_PATH}
)

# Declaration of executables
add_executable(
	SimultIte
	src/main.c
	src/executable_options.c
	src/matrix_reader.c
	src/cl_utils.c
	src/gram_schmidt.c
	lib/src/mmio.c
)

# Linkage
target_link_libraries(
	SimultIte
    ${OpenCL_LIBRARIES}
    ${MPI_C_LIBRARIES}
    ${clSPARSE_LIBRARIES}
)
