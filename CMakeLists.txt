#
# On Windows set correct path to the VCPKG includes at line 16
#
# Options:
# -DENABLE_LIBUV=off   	enable libuv UDP/TCP. Default UDP only
# -DENABLE_DEBUG=off   	enable debugging output
# -DENABLE_GEN=on   	enable key generator (default in memory storage)
# -DENABLE_SQLITE=off  	enable SQLITE3 backend (reserved)
# -DENABLE_IPV6=off   	enable IPv6 (reserved)
#
# Example:
# cmake -DENABLE_LIBUV=on
#
cmake_minimum_required(VERSION 3.9)
if(${ESP_PLATFORM})
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
	project("lorawan-storage" VERSION 1.0)
#    get_filename_component(EXTRA_COMPONENT_DIRS esp32 ABSOLUTE)
#    set(IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})
else()
	project("lorawan-storage" VERSION 1.0)
    # ------------------------ CHANGE PATH BELOW ON WINDOWS ------------------------
    set(VCPKG_INC "/git/vcpkg/installed/x64-windows/include")
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
		set(PRG lorawan-service)
		set(LOCALE_INSTALL_DIR /usr/share/locale)
		foreach(_poFile ${PO_FILES})
			GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
			STRING(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
			STRING(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
			STRING(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
			if( _langCode )
				message("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${Intl_LIBRARY_DIRS} INCDIR ${Intl_INCLUDE_DIRS}")
				GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)

				set(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
				set(_moFile ${_moPath}/${PRG}.mo)
				message("** ** ${_moFile} install to ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/${PRG}.mo")
				#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
				if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
					message("** powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE}
						${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						command powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE}
							${_moPath} ${_moFile} ${_poFile} DEPENDS ${_poFile})
				else()
					message("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						command ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
							DEPENDS ${_poFile})
				endif()
				#			install(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
				list(APPEND MO_FILES ${_moFile})
			endif()
		endforeach(_poFile ${PO_FILES})
		add_custom_target(translations ALL DEPENDS ${MO_FILES})
	endif()

	set(CMAKE_CXX_STANDARD 11)

	option(ENABLE_IPV6 "Build with IPv6" OFF)
	option(ENABLE_DEBUG "Build with debug" OFF)
	option(ENABLE_LIBUV "Build with libuv" OFF)
	option(ENABLE_GEN "Build with generator" OFF)
	option(ENABLE_SQLITE "Build with SQLite" OFF)

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	set(ARGTABLE "third-party/argtable3/argtable3.c")
	set(AES_SRC third-party/system/crypto/aes.c third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	#
	# liblorawan
	#
	set(SRC_LIBLORAWAN
		lorawan/lorawan-types.cpp lorawan/lorawan-string.cpp lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp
		lorawan/lorawan-mac.cpp lorawan/lorawan-error.cpp lorawan/lorawan-msg.cpp
		network-identity.cpp gateway-identity.cpp
		gateway-service.cpp identity-service.cpp service-serialization.cpp
		ip-helper.cpp ip-address.cpp
		third-party/strptime.cpp
	)
	add_library(lorawan STATIC ${SRC_LIBLORAWAN})
	target_link_libraries(lorawan PRIVATE ${OS_SPECIFIC_LIBS} )
	target_include_directories(lorawan PRIVATE "third-party" "." ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	if (ENABLE_IPV6)
		set(GATEWAY_DEF ENABLE_IPV6 )
	endif()

	if (ENABLE_DEBUG)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_DEBUG)
	endif()

	if (ENABLE_LIBUV)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_LIBUV)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			set(LIBUV LibUV)
			set(LIBUVA $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,uv>)
		else()
			set(LIBUV LibUV)
			set(LIBUVA LibUV::LibUV)
		endif()

		find_package(${LIBUV})

		set(LSNR_SRC uv-listener.cpp uv-mem.cpp)
		set(CLI_SRC uv-client.cpp uv-mem.cpp)
	else()
		set(LSNR_SRC udp-listener.cpp)
		set(CLI_SRC udp-client.cpp plugin-client.cpp)
	endif()
	set(CLI_SRC ${CLI_SRC} plugin-client.cpp)

	if (ENABLE_GEN)
		set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_GEN)
		set(GATEWAY_SRC gateway-service-mem.cpp identity-service-gen.cpp key128gen.cpp ${AES_SRC})
	else()
		if (ENABLE_SQLITE)
			set(GATEWAY_DEF ${GATEWAY_DEF} ENABLE_SQLITE)
			set(GATEWAY_SRC identity-service-sqlite.cpp gateway-service-sqlite.cpp sqlite-helper.cpp)
			if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
				find_package(unofficial-sqlite3 CONFIG REQUIRED)
				set(BACKEND_DB_LIB unofficial::sqlite3::sqlite3)
			else ()
				find_package(SQLite3 REQUIRED)
				set(BACKEND_DB_LIB ${SQLite3_LIBRARIES})
				set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})
			endif ()
		else()
			set(GATEWAY_SRC identity-service-mem.cpp gateway-service-mem.cpp)
		endif()
	endif()

	#
	# Plugins
	#
	add_library(storage-mem SHARED identity-service-mem.cpp gateway-service-mem.cpp)
	target_link_libraries(storage-mem PRIVATE lorawan)
	add_library(storage-gen SHARED identity-service-gen.cpp gateway-service-mem.cpp key128gen.cpp ${AES_SRC})
	target_link_libraries(storage-gen PRIVATE lorawan)
	target_include_directories(storage-gen PRIVATE "third-party")
	add_library(storage-sqlite SHARED identity-service-sqlite.cpp gateway-service-sqlite.cpp file-helper.cpp sqlite-helper.cpp)
	target_link_libraries(storage-sqlite PRIVATE lorawan ${BACKEND_DB_LIB})
	
	#
	# lorawan-service
	#
	set(LORAWAN_SERVICE_SRC
		cli-main.cpp ${LSNR_SRC}
		${GATEWAY_SRC} gateway-serialization.cpp identity-serialization.cpp
		third-party/daemonize.cpp
		file-helper.cpp
		${ARGTABLE}
	)
	add_executable(lorawan-service ${LORAWAN_SERVICE_SRC})
	target_link_libraries(lorawan-service PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${LIBUVA} lorawan ${BACKEND_DB_LIB})
	target_compile_definitions(lorawan-service PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-service PRIVATE "third-party" "." ${VCPKG_INC} ${Intl_INCLUDE_DIRS} ${BACKEND_DB_INC})

	#
	# lorawan-query
	#
	set(LORAWAN_QUERY_SRC
		cli-query-main.cpp ${CLI_SRC}
		identity-serialization.cpp gateway-serialization.cpp lorawan/lorawan-date.cpp
		${ARGTABLE}
	)
	add_executable(lorawan-query ${LORAWAN_QUERY_SRC})
	target_link_libraries(lorawan-query PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${LIBUVA} lorawan)
	target_compile_definitions(lorawan-query PRIVATE ${GATEWAY_DEF})
	target_include_directories(lorawan-query PRIVATE "third-party" "." ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	install(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

	include(InstallRequiredSystemLibraries)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)
endif()
