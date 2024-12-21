cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.3)
project("lorawan-storage" VERSION ${VERSION_INFO})
set(CMAKE_CXX_STANDARD 11)

add_executable(test-parse-packet
	test-parse-packet.cpp
)
target_include_directories(test-parse-packet PRIVATE .. ../third-party)
target_link_libraries(test-parse-packet PRIVATE lorawan)

#
add_test(NAME test-parse-packet COMMAND "test-parse-packet")
