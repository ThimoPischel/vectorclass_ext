cmake_minimum_required(VERSION 3.14)

project(vectorclass_ext LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O")

include_directories(./agner-fog_vectorclass)
add_executable(vectorclass_ext
  main.cpp
  vectorclass_ext.h
)

include(GNUInstallDirs)
install(TARGETS vectorclass_ext
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
