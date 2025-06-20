set(APP_NAME example)

add_executable(${APP_NAME}
        main.cpp
)

target_link_libraries(${APP_NAME} PRIVATE
        starter::starter
        sdbus-c++
)