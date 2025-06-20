cmake_minimum_required(VERSION 3.25)
project(starter
        VERSION 0.0.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/khromenokroman/starter"
)

configure_file(Version.in Version @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Собирать динамическую библиотеку вместо статической" ON)
option(BUILD_EXAMPLE "Собирать приложение (пример)" ON)

include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR} CACHE PATH "Path for headers installation")
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR} CACHE PATH "Path for libraries installation")

include(CMakePackageConfigHelpers)

configure_package_config_file(
        starter-config.cmake.in
        starter-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/starter
        PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
)

write_basic_package_version_file(
        starter-configVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

if (BUILD_EXAMPLE)
    add_subdirectory(src/example)
endif ()

add_subdirectory(src/lib)

set(CPACK_GENERATOR DEB)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "roma55592@yandex.ru")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

if (BUILD_SHARED_LIBS)
    # Настройки для динамической библиотеки
    set(CPACK_DEBIAN_LIB_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}")
    set(CPACK_DEBIAN_LIB__PACKAGE_DEPENDS "libsdbus-c++2")
    set(CPACK_DEBIAN_LIB_DESCRIPTION "Starter")
    set(CPACK_DEBIAN_LIB_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_LIB_PACKAGE_SHLIBDEPS ON)

    # Настройки для dev-пакета при динамической библиотеке
    set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}-dev")
    set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR} (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})")
    set(CPACK_DEBIAN_DEV_DESCRIPTION "Starter - development files")
    set(CPACK_DEBIAN_DEV_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_DEV_PACKAGE_SHLIBDEPS ON)

    # Указываем компоненты для CPack
    set(CPACK_COMPONENTS_ALL lib dev)
else ()
    # Настройки для статической библиотеки - создаём только dev-пакет
    set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}${PROJECT_VERSION_MAJOR}-dev")
    set(CPACK_DEBIAN_DEV_DESCRIPTION "Starter - static library")
    set(CPACK_DEBIAN_DEV_MAINTAINER "roma55592@yandex.ru")
    set(CPACK_DEBIAN_DEV_PACKAGE_SHLIBDEPS ON)

    # Указываем только компонент dev для CPack
    set(CPACK_COMPONENTS_ALL dev)

    # Указываем, что это единственный компонент (важно!)
    set(CPACK_COMPONENT_DEV_REQUIRED ON)
endif ()

include(CPack)