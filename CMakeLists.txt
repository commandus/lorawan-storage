#
# On Windows set correct path to the VCPKG includes at line 16
#
# Options:
# -DENABLE_LIBUV=off   	enable libuv UDP/TCP. Default UDP only. libuv dependency required.
# -DENABLE_DEBUG=off   	enable debugging output
# -DENABLE_GEN=on   	enable key generator (default in memory storage)
# -DENABLE_HTTP=off		enable HTTP service. libmicrohttpd dependency required.
# -DENABLE_QRCODE=off	enable HTTP QRCode URN service.
# -DENABLE_SQLITE=off  	enable SQLITE3 backend (reserved)
# -DENABLE_IPV6=off   	enable IPv6 (reserved)
# -DENABLE_JSON=off		Build with JSON file
#
# Example:
# cmake -DENABLE_LIBUV=on
#
cmake_minimum_required(VERSION 3.9)

set(VERSION_INFO 1.0.3)

if(${ESP_PLATFORM})
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
	project("lorawan-storage" VERSION ${VERSION_INFO})
#    get_filename_component(EXTRA_COMPONENT_DIRS esp32 ABSOLUTE)
#    set(IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})
else()
	project("lorawan-storage" VERSION ${VERSION_INFO})
    # ------------------------ CHANGE PATH BELOW ON WINDOWS ------------------------
    set(VCPKG_INC "C:\\git\\vcpkg\\installed\\x64-windows\\include")
	set(MINGW_LIB "C:\\git\\vcpkg\\installed\\x64-windows\\lib")	# for MinGW only
    # ------------------------ CHANGE PATH ABOVE ON WINDOWS ------------------------
	set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
	include_directories(${VCPKG_INC})

	#
	# Setting up i18l & l10n
	# see https://stackoverflow.com/questions/10521635/using-intltool-with-cmake
	#
	find_package(Intl)
	find_package(Gettext)
	if(Intl_LIBRARY)
		set(LIBINTL Intl::Intl)
	endif()
	link_directories(${Intl_LIBRARY_DIRS})
	find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
	if(NOT GETTEXT_MSGFMT_EXECUTABLE)
		message("NOTE: msgfmt not found. Translations will *not* be installed")
	else()
		# Build the .po files
		set(PO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/po)
		# Input files
		file(GLOB PO_FILES ${PO_PATH}/*.po)
		# Generated file list
		set(MO_FILES)
		set(PRG lorawan-identity-service)
		set(LOCALE_INSTALL_DIR /usr/share/locale)
		FOREACH(_poFile ${PO_FILES})
			GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
			STRING(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
			STRING(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
			STRING(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
			IF( _langCode )
				MESSAGE("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${INTL_LIBRARY_DIRS} INCDIR ${INTL_INCLUDE_DIRS}")
				GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)

				SET(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
				SET(_moFile ${_moPath}/${PRG}.mo)
				MESSAGE("** ** ${_moFile} install to ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/${PRG}.mo")
#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	MESSAGE("** powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}")
	ADD_CUSTOM_COMMAND(OUTPUT ${_moFile}
		COMMAND powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}
		DEPENDS ${_poFile})
else()
	MESSAGE("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")
	ADD_CUSTOM_COMMAND(OUTPUT ${_moFile}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
		DEPENDS ${_poFile})
endif()
#			INSTALL(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
				LIST(APPEND MO_FILES ${_moFile})
			ENDIF()
		ENDFOREACH(_poFile ${PO_FILES})
	add_custom_target(translations ALL DEPENDS ${MO_FILES})
	endif()

	set(CMAKE_CXX_STANDARD 11)

	option(ENABLE_IPV6 "Build with IPv6" OFF)
	option(ENABLE_DEBUG "Build with debug" OFF)
	option(ENABLE_LIBUV "Build with libuv" OFF)
	option(ENABLE_GEN "Build with generator" OFF)
	option(ENABLE_HTTP "Build with HTTP" OFF)
	option(ENABLE_QRCODE "Build with HTTP QRCode URN" OFF)
	option(ENABLE_SQLITE "Build with SQLite" OFF)
	option(ENABLE_JSON "Build with JSON file" ON)

	set(ARGTABLE "third-party/argtable3/argtable3.c")
	set(AES_SRC third-party/system/crypto/aes.c third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	set(SRC_LIBLORAWAN
		lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp lorawan/lorawan-error.cpp lorawan/lorawan-mac.cpp
		lorawan/lorawan-msg.cpp lorawan/lorawan-string.cpp lorawan/lorawan-types.cpp lorawan/lorawan-key.cpp
		lorawan/lorawan-mic.cpp lorawan/lorawan-packet-storage.cpp
		lorawan/helper/aes-helper.cpp lorawan/helper/file-helper.cpp lorawan/helper/ip-address.cpp lorawan/helper/ip-helper.cpp
		lorawan/helper/key128gen.cpp lorawan/helper/sqlite-helper.cpp
		lorawan/helper/crc-helper.cpp
		lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp
		lorawan/storage/client/direct-client.cpp lorawan/storage/client/plugin-client.cpp
		lorawan/storage/client/plugin-query-client.cpp
		lorawan/storage/client/query-client.cpp lorawan/storage/client/service-client.cpp
		lorawan/storage/client/udp-client.cpp
		lorawan/storage/client/sync-query-client.cpp lorawan/storage/client/sync-response-client.cpp
		lorawan/storage/listener/storage-listener.cpp lorawan/storage/listener/udp-listener.cpp
		lorawan/storage/serialization/serialization.cpp
		lorawan/storage/serialization/gateway-serialization.cpp
		lorawan/storage/serialization/gateway-binary-serialization.cpp
		lorawan/storage/serialization/identity-serialization.cpp lorawan/storage/serialization/identity-binary-serialization.cpp
		lorawan/storage/serialization/identity-text-urn-serialization.cpp
		lorawan/storage/serialization/service-serialization.cpp
		lorawan/storage/serialization/urn-helper.cpp
		lorawan/storage/service/async-wrapper-gateway-service.cpp lorawan/storage/service/async-wrapper-identity-service.cpp
		lorawan/storage/service/gateway-service.cpp
		lorawan/storage/service/gateway-service-json.cpp
		lorawan/storage/service/gateway-service-mem.cpp
		lorawan/storage/service/identity-service.cpp
		lorawan/storage/service/identity-service-gen.cpp lorawan/storage/service/identity-service-json.cpp
		lorawan/storage/service/identity-service-mem.cpp
		lorawan/storage/service/identity-service-udp.cpp
		third-party/base64/base64.cpp
		third-party/strptime.cpp
		${AES_SRC}
	)

	if(ENABLE_HTTP)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/storage/listener/http-listener.cpp
		)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_HTTP)
		find_library(LIBMICROHTTPD NAMES microhttpd libmicrohttpd-dll.lib HINTS /usr/lib/x86_64-linux-gnu/ ${VCPKG_LIB}
				${MINGW_LIB}
		)
	endif()

	if(ENABLE_QRCODE)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_QRCODE)
	endif()

	if (ENABLE_IPV6)
		set(GATEWAY_DEF ENABLE_IPV6 )
	endif()

	if (ENABLE_DEBUG)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_DEBUG)
	endif()

	if (ENABLE_LIBUV)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/helper/uv-mem.cpp 
			lorawan/storage/client/uv-client.cpp lorawan/storage/listener/uv-listener.cpp
		)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_LIBUV)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			set(LIBUV LibUV)
			set(LIBUVA $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,uv>)
		else()
			set(LIBUV LibUV)
			set(LIBUVA LibUV::LibUV)
		endif()
		find_package(${LIBUV})
	endif()

	if (ENABLE_GEN)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_GEN)
	else()
		if (ENABLE_SQLITE)
			set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_SQLITE)
		endif()
	endif()

	if (ENABLE_SQLITE)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/storage/service/identity-service-sqlite.cpp
			lorawan/storage/service/gateway-service-sqlite.cpp
		)
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

	if (ENABLE_JSON)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_JSON)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/storage/serialization/json-helper.cpp
			lorawan/storage/serialization/identity-text-json-serialization.cpp
			lorawan/storage/serialization/gateway-text-json-serialization.cpp
				lorawan/helper/crc-helper.h
				lorawan/helper/crc-helper.cpp
		)
	endif()

	if (ENABLE_QRCODE)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/storage/serialization/qr-helper.cpp
			third-party/nayuki/qrcodegen.cpp
		)
	endif()

	#
	# liblorawan
	#
	add_library(lorawan STATIC ${SRC_LIBLORAWAN})
	target_link_libraries(lorawan PRIVATE ${OS_SPECIFIC_LIBS} ${LIBMICROHTTPD} ${BACKEND_DB_LIB})
	target_include_directories(lorawan PRIVATE "third-party" "." ${VCPKG_INC} ${Intl_INCLUDE_DIRS})
	# enable qr code generation by conditional variable
	target_compile_definitions(lorawan PRIVATE ${GATEWAY_DEF})
	set_property(TARGET lorawan PROPERTY POSITION_INDEPENDENT_CODE ON)

	#
	# Plugins
	#
	add_library(storage-mem SHARED lorawan/storage/service/identity-service-mem.cpp)
	target_link_libraries(storage-mem PRIVATE lorawan)
	target_include_directories(storage-mem PRIVATE ".")
	set_target_properties(storage-mem PROPERTIES SOVERSION ${VERSION_INFO})

	add_library(storage-json SHARED lorawan/storage/service/identity-service-json.cpp lorawan/storage/service/gateway-service-json.cpp)
	target_link_libraries(storage-json PRIVATE lorawan)
	target_include_directories(storage-json PRIVATE "." "third-party")
	set_target_properties(storage-json PROPERTIES SOVERSION ${VERSION_INFO})

	add_library(storage-gen SHARED lorawan/storage/service/identity-service-gen.cpp lorawan/helper/key128gen.cpp ${AES_SRC})
	target_link_libraries(storage-gen PRIVATE lorawan)
	target_include_directories(storage-gen PRIVATE "." "third-party")
	set_target_properties(storage-gen PROPERTIES SOVERSION ${VERSION_INFO})

	if (ENABLE_SQLITE)
		add_library(storage-sqlite SHARED 
			lorawan/storage/service/identity-service-sqlite.cpp 
			lorawan/storage/service/gateway-service-sqlite.cpp
			lorawan/helper/file-helper.cpp
			lorawan/helper/sqlite-helper.cpp
		)
		target_link_libraries(storage-sqlite PRIVATE lorawan ${BACKEND_DB_LIB})
		target_include_directories(storage-sqlite PRIVATE ".")
		set_target_properties(storage-sqlite PROPERTIES SOVERSION ${VERSION_INFO})
	endif()

	add_library(storage-udp SHARED lorawan/storage/service/identity-service-udp.cpp)
	target_link_libraries(storage-udp PRIVATE lorawan)
	target_include_directories(storage-udp PRIVATE ".")
	set_target_properties(storage-udp PROPERTIES SOVERSION ${VERSION_INFO})

	#
	# C to C++ connector to identity serialization routines
	#
	add_library(identity-c SHARED lorawan/wrapper/connector-identity-serialization.cpp)
	target_link_libraries(identity-c PRIVATE lorawan)
	target_include_directories(identity-c PRIVATE ".")
	set_target_properties(identity-c PROPERTIES SOVERSION ${VERSION_INFO})

	#
	# lorawan-identity-service
	#
	set(LORAWAN_IDENTITY_SERVICE_SRC
		cli-main.cpp cli-helper.cpp
		third-party/daemonize.cpp
		lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp
		${ARGTABLE}
	)

	add_executable(lorawan-identity-service ${LORAWAN_IDENTITY_SERVICE_SRC})
	target_link_libraries(lorawan-identity-service PRIVATE ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} ${LIBMICROHTTPD} ${LIBINTL} ${LIBUVA} lorawan)
	target_compile_definitions(lorawan-identity-service PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-identity-service PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	#
	# lorawan-identity-query
	#
	add_executable(lorawan-identity-query cli-query-main.cpp cli-helper.cpp
		lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp
		${ARGTABLE}
	)
	target_link_libraries(lorawan-identity-query PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${LIBUVA} lorawan)
	target_compile_definitions(lorawan-identity-query PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-identity-query PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	#
	# lorawan-query-identity-direct
	#
	add_executable(lorawan-query-identity-direct cli-query-plugin-main.cpp cli-helper.cpp
		lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp
		${ARGTABLE}
	)
	target_link_libraries(lorawan-query-identity-direct PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${BACKEND_DB_LIB} lorawan)
	target_compile_definitions(lorawan-query-identity-direct PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-query-identity-direct PRIVATE "." "third-party" ${VCPKG_INC} ${BACKEND_DB_INC} ${Intl_INCLUDE_DIRS})

	#
	# lorawan-identity-print
	#
	add_executable(lorawan-identity-print cli-print.cpp cli-helper.cpp
		${ARGTABLE}
	)
	target_link_libraries(lorawan-identity-print PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${BACKEND_DB_LIB} lorawan)
	target_include_directories(lorawan-identity-print PRIVATE "." "third-party" ${VCPKG_INC} ${BACKEND_DB_INC} ${Intl_INCLUDE_DIRS})

	#
	# lorawan-tag
	#
	add_executable(lorawan-tag cli-tag.cpp cli-helper.cpp ${ARGTABLE}
		third-party/nayuki/qrcodegen.cpp
	)
	target_link_libraries(lorawan-tag PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan)
	target_compile_definitions(lorawan-tag PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-tag PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	install(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION systemd)

	include(InstallRequiredSystemLibraries)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)

	# Print config
	message("")
	message("Options (on- enabled, off- disabled):")
	message("")
	message("-DENABLE_LIBUV=${ENABLE_LIBUV} \t enable libuv UDP/TCP")
	message("-DENABLE_DEBUG=${ENABLE_DEBUG} \t enable debugging output")
	message("-DENABLE_GEN=${ENABLE_GEN} \t enable key generator (default in memory storage)")
	message("-DENABLE_HTTP=${ENABLE_HTTP} \t enable HTTP service. libmicrohttpd dependency required")
	message("-DENABLE_QRCODE=${ENABLE_QRCODE} \t enable HTTP QRCode URN service")
	message("-DENABLE_SQLITE=${ENABLE_SQLITE} \t enable SQLITE3 backend (reserved)")
	message("-DENABLE_IPV6=${ENABLE_IPV6} \t enable IPv6 (reserved)")
	message("-DENABLE_JSON=${ENABLE_JSON} \t build with JSON file backend. -DENABLE_HTTP=on required")
	message("")

	add_subdirectory(tests)
endif()
