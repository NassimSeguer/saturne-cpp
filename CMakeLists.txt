cmake_minimum_required(VERSION 3.20)
project(Saturne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Le rendu fait des millions de calculs : on compile en mode optimisé par défaut
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# Avertissements du compilateur, pour repérer les erreurs au plus tôt.
# Avec Visual Studio, /utf-8 indique que les fichiers sources sont en UTF-8.
if (MSVC)
    add_compile_options(/W4 /utf-8)
else ()
    add_compile_options(-Wall -Wextra)
endif ()

# Fichiers du programme, partagés entre l'exécutable et les tests
set(SOURCES_SATURNE
    src/couleur.cpp
    src/vecteur.cpp
    src/image.cpp
    src/etoiles.cpp
    src/scene.cpp
    src/rendu.cpp
)

# Le programme
add_executable(saturne src/main.cpp ${SOURCES_SATURNE})
target_include_directories(saturne PRIVATE include)

# Les tests : lancer "ctest" dans le dossier de compilation
enable_testing()
add_executable(saturne_tests tests/tests.cpp ${SOURCES_SATURNE})
target_include_directories(saturne_tests PRIVATE include)
add_test(NAME tests_saturne COMMAND saturne_tests)
