cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для iOS/macOS — архитектуры
if(IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(customrates VERSION 1.0.0)

# ════════════════════════════════════════════════════════════
#  НОВЫЙ СТИЛЬ GEODE V5
# ════════════════════════════════════════════════════════════

# Находим Geode SDK (автоматически через CPM)
find_package(Geode REQUIRED)

# КРИТИЧЕСКИ ВАЖНО: настраиваем платформу
geode_configure_platform()

# Собираем все исходники из папки src
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

# Создаём библиотеку мода
geode_library(
    SOURCES ${SOURCES}
)

# Копируем ресурсы (если есть папка resources)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
    geode_copy_resources(
        TARGET ${PROJECT_NAME}
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources
    )
endif()

# Финальная настройка мода
geode_configure_mod(
    TARGET ${PROJECT_NAME}
    ID "nekroz.customrates"
    NAME "Custom Rates"
    DEVELOPER "Nekroz"
)
