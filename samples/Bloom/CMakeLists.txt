cmake_minimum_required(VERSION 3.11)

project(Bloom LANGUAGES CXX)
message("Sample Bloom: configured")
set(SOURCE ${PROJECT_SOURCE_DIR}/Bloom.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})

add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy 
            ${RIF_INSTALL_DIR}/$<CONFIG>/${CMAKE_STATIC_LIBRARY_PREFIX}${GLOBAL_TARGET}${RIF_LIB_EXTENSION}
            ${SAMPLES_INSTALL_DIR}/$<CONFIG>/
            COMMENT "copying ${CMAKE_STATIC_LIBRARY_PREFIX}${GLOBAL_TARGET}.${RIF_LIB_EXTENSION}"
)

copy_dependencies(${SAMPLES_INSTALL_DIR}/$<CONFIG>/)
