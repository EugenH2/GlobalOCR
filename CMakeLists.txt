cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GlobalOCR VERSION 1.0.0 LANGUAGES CXX)
set(EXECUTABLE_NAME GlobalOCR)

add_executable(${EXECUTABLE_NAME} WIN32)
target_sources(${EXECUTABLE_NAME} PRIVATE src/GlobalOCR.cpp
                                          src/pch.cpp
					  include/pch.h
					  include/framework.h
                                          include/GlobalOCR.h
                                          include/Resource.h
                                          include/DataLoader.h
                                          include/Model.h
                                          resource/GlobalOCR.ico
                                          resource/GlobalOCR.rc       
                                          resource/small.ico                                   
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE include)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")