cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ════════════════════════════════════════════════════════════
#  ПРАВИЛЬНАЯ НАСТРОЙКА АРХИТЕКТУР ДЛЯ iOS И macOS
# ════════════════════════════════════════════════════════════
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(customrates VERSION 1.0.0)

# ════════════════════════════════════════════════════════════
#  СТАРЫЙ СТИЛЬ GEODE (СОВМЕСТИМ С ЭКШЕНОМ)
# ════════════════════════════════════════════════════════════

# 1. Проверяем, что экшен установил путь к SDK
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable is not set!")
endif()

# 2. Подключаем SDK напрямую
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# 3. Собираем все исходники
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

# 4. Создаем библиотеку мода
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# 5. Настраиваем мод (старая функция)
setup_geode_mod(${PROJECT_NAME})
