cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.3)
project("lorawan-storage" VERSION ${VERSION_INFO})
set(CMAKE_CXX_STANDARD 11)

if (ENABLE_SQLITE)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		find_package(unofficial-sqlite3 CONFIG REQUIRED)
		set(BACKEND_DB_LIB unofficial::sqlite3::sqlite3)
		set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})
	else ()
		find_package(SQLite3 REQUIRED)
		set(BACKEND_DB_LIB ${SQLite3_LIBRARIES})
		set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})
	endif ()
endif()

add_executable(test-parse-packet
	test-parse-packet.cpp
)
target_include_directories(test-parse-packet PRIVATE .. ../third-party)
target_link_libraries(test-parse-packet PRIVATE lorawan ${BACKEND_DB_LIB})

add_executable(test-identity-service
	test-identity-service.c
)
target_include_directories(test-identity-service PRIVATE .. ../third-party)
target_link_libraries(test-identity-service PRIVATE lorawan ${BACKEND_DB_LIB})
set_property(TARGET test-identity-service PROPERTY C_STANDARD 99)
#
add_test(NAME test-parse-packet COMMAND "test-parse-packet")
add_test(NAME test-identity-service COMMAND "test-identity-service")
