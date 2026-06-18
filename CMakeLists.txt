cmake_minimum_required(VERSION 3.16...3.26)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(obs-guide-overlay VERSION 1.0.0)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/guide-overlay.c)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    OBS::libobs
    OBS::obs-frontend-api
)

set_target_properties_obs(${CMAKE_PROJECT_NAME} PROPERTIES
    FOLDER "plugins/${CMAKE_PROJECT_NAME}"
    PREFIX ""
)
