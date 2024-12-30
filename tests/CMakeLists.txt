cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.3)
project("lorawan-storage" VERSION ${VERSION_INFO})
set(CMAKE_CXX_STANDARD 11)

add_executable(test-parse-packet
	test-parse-packet.cpp
)
target_include_directories(test-parse-packet PRIVATE .. ../third-party)
target_link_libraries(test-parse-packet PRIVATE lorawan)
target_compile_definitions(test-parse-packet PRIVATE ${GATEWAY_DEF})

add_executable(test-identity-service
	test-identity-service.c
)
target_include_directories(test-identity-service PRIVATE .. ../third-party)
target_link_libraries(test-identity-service PRIVATE lorawan)
set_property(TARGET test-identity-service PROPERTY C_STANDARD 99)
target_compile_definitions(test-identity-service PRIVATE ${GATEWAY_DEF})
#
add_test(NAME test-parse-packet COMMAND "test-parse-packet")
add_test(NAME test-identity-service COMMAND "test-identity-service")
