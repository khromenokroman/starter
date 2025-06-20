set(LIB_NAME starter)

set(INCLUDE_DIR include)
set(SOURCES dbus.cpp)

if (BUILD_SHARED_LIBS)
    add_library(${LIB_NAME} SHARED ${SOURCES})

    set_target_properties(${LIB_NAME} PROPERTIES
            VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    install(TARGETS ${LIB_NAME}
            EXPORT ${LIB_NAME}Targets
            COMPONENT lib
            LIBRARY DESTINATION lib
            NAMELINK_COMPONENT dev
    )
else ()
    add_library(${LIB_NAME} STATIC ${SOURCES})

    set_target_properties(${LIB_NAME} PROPERTIES
            VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    )

    install(TARGETS ${LIB_NAME}
            EXPORT ${LIB_NAME}Targets
            COMPONENT dev
            ARCHIVE DESTINATION lib
    )
endif ()

target_include_directories(${LIB_NAME} PRIVATE ${INCLUDE_DIR})

add_library(${LIB_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

target_link_libraries(${LIB_NAME} PRIVATE sdbus-c++)

install(DIRECTORY ${INCLUDE_DIR}/
        COMPONENT dev
        DESTINATION include
)

install(EXPORT ${LIB_NAME}Targets
        FILE ${LIB_NAME}Targets.cmake
        NAMESPACE ${LIB_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/starter
        COMPONENT dev
)

install(FILES
        ${CMAKE_BINARY_DIR}/starter-configVersion.cmake
        ${CMAKE_BINARY_DIR}/starter-config.cmake
        DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/starter
        COMPONENT dev
)
