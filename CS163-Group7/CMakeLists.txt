cmake_minimum_required(VERSION 3.15)
project(CS163-Group7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Khai báo các file mã nguồn
set(SOURCES
    src/main.cpp
)


add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/SFML/include
)

if(MSVC)
    message(STATUS "Đang sử dụng cấu hình cho Visual Studio (MSVC)")
    
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SFML/lib/msvc)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        debug sfml-graphics-d debug sfml-window-d debug sfml-system-d debug sfml-audio-d debug sfml-network-d
        optimized sfml-graphics optimized sfml-window optimized sfml-system optimized sfml-audio optimized sfml-network
    )
    add_compile_definitions(UNICODE _UNICODE)

    file(GLOB SFML_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/SFML/bin/msvc/*.dll")

elseif(MINGW)
    message(STATUS "Đang sử dụng cấu hình cho MinGW (GCC)")
    
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SFML/lib/mingw)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics sfml-window sfml-system sfml-audio sfml-network
    )

    file(GLOB SFML_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/SFML/bin/mingw/*.dll")
endif()

if(SFML_DLLS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SFML_DLLS}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)