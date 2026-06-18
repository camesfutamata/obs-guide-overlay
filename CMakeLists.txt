cmake_minimum_required(VERSION 3.16)
project(obs-guide-overlay VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)

add_library(obs-guide-overlay MODULE src/guide-overlay.c)

target_include_directories(obs-guide-overlay PRIVATE
    ${LIBOBS_INCLUDE_DIR}
    ${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api
)

if(APPLE)
    target_link_options(obs-guide-overlay PRIVATE
        -undefined dynamic_lookup
    )
else()
    target_link_libraries(obs-guide-overlay PRIVATE
        ${LIBOBS_LIB}
        ${OBS_FRONTEND_API_LIB}
    )
endif()

set_target_properties(obs-guide-overlay PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)
