set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 3.5)

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

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so)
    message(STATUS "Downloading clSPARSE library")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tmp)
    file(DOWNLOAD
        https://github.com/clMathLibraries/clSPARSE/releases/download/v0.10.2.0/clSPARSE-0.10.2.0-Linux-x64.tar.gz
        ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE.tar.gz
        SHOW_PROGRESS
        EXPECTED_MD5 4c3ab499801a1ff8211d189c01a97cda)
    message(STATUS "Decompressing clSPARSE library")
    execute_process(COMMAND
        tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE.tar.gz -C ${CMAKE_CURRENT_SOURCE_DIR}/tmp/ clSPARSE-0.10.2.0-Linux-x64/include/ )
    execute_process(COMMAND
        rsync -a ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE-0.10.2.0-Linux-x64/include/ ${CMAKE_CURRENT_SOURCE_DIR}/lib/header/)
    execute_process(COMMAND
        tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE.tar.gz -C ${CMAKE_CURRENT_SOURCE_DIR}/tmp/ clSPARSE-0.10.2.0-Linux-x64/lib64/)
    execute_process(COMMAND
        rm -rf  ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE-0.10.2.0-Linux-x64/lib64/cmake)
    execute_process(COMMAND
        rsync -a ${CMAKE_CURRENT_SOURCE_DIR}/tmp/clSPARSE-0.10.2.0-Linux-x64/lib64/ ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/)
    message(STATUS "Deleting temporary files")
    file(REMOVE_RECURSE
        ${CMAKE_CURRENT_SOURCE_DIR}/tmp)
else(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so)
    message(STATUS "clSPARSE found")
endif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so)
set(clSPARSE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclSPARSE.so ${CMAKE_CURRENT_SOURCE_DIR}/lib/bin/libclsparseTimer.so)
set(clSPARSE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/clSPARSE/src/include)


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
