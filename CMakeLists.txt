cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

########################################################################################################################
## Define project
########################################################################################################################
project(
        MuImpactFramework
        VERSION 0.7.4
        DESCRIPTION "MuImpactFramework by Acro"
        LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

include(GNUInstallDirs)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY
)

set(headers
        include/Utility.hpp
        include/nifUtil.h
        include/PapyrusScripts.h
        include/Hook.h
        include/Condition.h
        include/Config.h
        include/ImpactManagerImpl.h
        include/ImpactManager.h
)

set(sources
        src/nifUtil.cpp
        src/PapyrusScripts.cpp
        src/Hook.cpp
        src/Condition.cpp
        src/Config.cpp
        src/ImpactManagerImpl.cpp
        src/ImpactManager.cpp
        src/Main.cpp

        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources})

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(CommonLibSSE REQUIRED)
find_package(ryml CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_path(ARTICUNO_INCLUDE_DIRS "articuno/articuno.h")

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_compile_features(
    ${PROJECT_NAME}
    PUBLIC
        cxx_std_23
)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
        _UNICODE
        UNICODE
        "$<$<BOOL:${USE_COMMONLIBSSE_NG}>:USE_COMMONLIBSSE_NG=1>"
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>
        ${DETOURS_INCLUDE_DIRS}
        ${ARTICUNO_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ryml::ryml
        magic_enum::magic_enum
)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        include/PCH.h)

target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/Zi>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

########################################################################################################################
## Automatic plugin deployment
########################################################################################################################
# Automatic deployment to FOMOD directory.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_NAME "Debug")
else()
    set(BUILD_NAME "Release")
endif()
install(DIRECTORY DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Papyrus${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
if(${CMAKE_BUILD_TYPE} STREQUAL Release OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/Plugin${BUILD_NAME}/")
endif()
file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
