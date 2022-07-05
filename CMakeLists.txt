set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cmake_minimum_required(VERSION 2.8.3)
project(keyboard_controller)
find_package(catkin REQUIRED COMPONENTS roscpp std_msgs)

SET(CMAKE_BUILD_TYPE RelWithDebInfo)


include_directories(include ${catkin_INCLUDE_DIRS})
catkin_package(
  INCLUDE_DIRS 
  LIBRARIES 
	${PROJECT_NAME}
  CATKIN_DEPENDS
)

add_executable(keyboard_controller src/keyboard_controller.cpp)
target_link_libraries(keyboard_controller ${catkin_LIBRARIES})
