cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.3)
project("lorawan-storage" VERSION ${VERSION_INFO})
set(CMAKE_CXX_STANDARD 11)

option(ENABLE_MINIZ "Build with miniz" OFF)

if (ENABLE_MINIZ)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		find_package(miniz CONFIG REQUIRED)
		set(EXTRA_LIBS ${EXTRA_LIBS} miniz::miniz)
		set(EXTRA_DEF ${EXTRA_DEF} ENABLE_MINIZ)
	else()
		set(EXTRA_LIBS ${EXTRA_LIBS} minizip)
		set(EXTRA_DEF ${EXTRA_DEF} ENABLE_MINIZIP)
	endif()
endif()

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

add_executable(test-heatshrink
	test-heatshrink.cpp
	../third-party/heatshrink/heatshrink_encoder.c
	../third-party/heatshrink/heatshrink_decoder.c
)
target_include_directories(test-heatshrink PRIVATE .. ../third-party)
target_link_libraries(test-heatshrink PRIVATE lorawan)

add_executable(test-miniz
	test-miniz.cpp
)
target_include_directories(test-miniz PRIVATE .. ../third-party)
target_link_libraries(test-miniz PRIVATE lorawan ${EXTRA_LIBS})
target_compile_definitions(test-miniz PRIVATE ${EXTRA_DEF})

#
add_test(NAME test-parse-packet COMMAND "test-parse-packet")
add_test(NAME test-identity-service COMMAND "test-identity-service")
add_test(NAME test-heatshrink COMMAND "test-heatshrink")
add_test(NAME test-miniz COMMAND "test-miniz")

message("-DENABLE_MINIZ=${ENABLE_MINIZ} \t build with miniz.")
