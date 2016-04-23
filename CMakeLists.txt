#Version 3.0 for C++11 features
cmake_minimum_required(VERSION 3.0) 

project("Titanic")

#Compile poco libraries
set(POCO_STATIC "ON")
add_subdirectory(libs/poco)

#Executables
add_subdirectory(Daemon)
add_subdirectory(Agent)
