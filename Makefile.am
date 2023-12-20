SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
VERSION = 1.0.0
VERSION_INFO = 1:0:0

# ../../third-party for make distcheck happy
common_flags = -I. -Ithird-party -I../../third-party

AM_CPPFLAGS=$(common_flags)
AM_CXXFLAGS=$(common_flags) -std=c++11
# COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

ARGTABLE_SRC = third-party/argtable3/argtable3.c
AES_SRC = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c

GATEWAY_DEF = $(COMMON_CPP_FLAGS)
EXTRA_LIB =
if ENABLE_LIBUV
    GATEWAY_DEF += -DENABLE_LIBUV
    EXTRA_LIB += -luv
endif

if ENABLE_SQLITE
    GATEWAY_DEF += -DENABLE_SQLITE
    EXTRA_LIB += -lsqlite3
endif
if ENABLE_GEN
    GATEWAY_DEF += -DENABLE_GEN
endif

# Static library used for build binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

lib_LIBRARIES = liblorawan.a

lib_LTLIBRARIES = libstorage-mem.la libstorage-gen.la
if ENABLE_SQLITE
    lib_LTLIBRARIES += libstorage-sqlite.la
endif

#
# Binaries
#
bin_PROGRAMS = lorawan-service lorawan-query lorawan-query-direct

SRC_ARGTABLE = third-party/argtable3/argtable3.c

nobase_dist_include_HEADERS = \
	lorawan/lorawan-const.h lorawan/lorawan-conv.h lorawan/lorawan-date.h lorawan/lorawan-error.h \
	lorawan/lorawan-mac.h lorawan/lorawan-msg.h lorawan/lorawan-string.h lorawan/lorawan-types.h \
	lorawan/helper/file-helper.h lorawan/helper/ip-address.h lorawan/helper/ip-helper.h \
	lorawan/helper/key128gen.h lorawan/helper/sqlite-helper.h lorawan/helper/uv-mem.h \
	lorawan/storage/gateway-identity.h lorawan/storage/network-identity.h \
	lorawan/storage/client/direct-client.h lorawan/storage/client/plugin-client.h \
	lorawan/storage/client/plugin-query-client.h lorawan/storage/client/query-client.h \
	lorawan/storage/client/service-client.h lorawan/storage/client/udp-client.h \
	lorawan/storage/client/uv-client.h \
	lorawan/storage/listener/storage-listener.h lorawan/storage/listener/udp-listener.h \
	lorawan/storage/listener/uv-listener.h \
	lorawan/storage/serialization/gateway-serialization.h lorawan/storage/serialization/identity-serialization.h \
	lorawan/storage/serialization/service-serialization.h \
	lorawan/storage/service/async-wrapper-gateway-service.h lorawan/storage/service/async-wrapper-identity-service.h \
	lorawan/storage/service/gateway-service.h lorawan/storage/service/gateway-service-mem.h \
	lorawan/storage/service/gateway-service-sqlite.h lorawan/storage/service/identity-service-gen.h \
	lorawan/storage/service/identity-service.h lorawan/storage/service/identity-service-mem.h \
	lorawan/storage/service/identity-service-sqlite.h \
	cli-helper.h config.h log.h platform-specific.h \
	third-party/system/crypto/aes.h third-party/system/crypto/cmac.h \
	third-party/system/crypto/aes.c third-party/system/crypto/cmac.c \
	third-party/argtable3/argtable3.h third-party/strptime.h third-party/daemonize.h

#
# liblorawan
#
SRC_LIBLORAWAN = \
    lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp lorawan/lorawan-error.cpp lorawan/lorawan-mac.cpp \
    lorawan/lorawan-msg.cpp lorawan/lorawan-string.cpp lorawan/lorawan-types.cpp \
    lorawan/helper/file-helper.cpp lorawan/helper/ip-address.cpp lorawan/helper/ip-helper.cpp \
    lorawan/helper/key128gen.cpp lorawan/helper/sqlite-helper.cpp lorawan/helper/uv-mem.cpp \
    lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp \
    lorawan/storage/client/direct-client.cpp lorawan/storage/client/plugin-client.cpp \
    lorawan/storage/client/plugin-query-client.cpp \
    lorawan/storage/client/query-client.cpp lorawan/storage/client/service-client.cpp \
    lorawan/storage/client/udp-client.cpp lorawan/storage/client/uv-client.cpp \
    lorawan/storage/listener/storage-listener.cpp lorawan/storage/listener/udp-listener.cpp \
    lorawan/storage/listener/uv-listener.cpp \
    lorawan/storage/serialization/gateway-serialization.cpp lorawan/storage/serialization/identity-serialization.cpp \
    lorawan/storage/serialization/service-serialization.cpp \
    lorawan/storage/service/async-wrapper-gateway-service.cpp lorawan/storage/service/async-wrapper-identity-service.cpp \
    lorawan/storage/service/gateway-service.cpp lorawan/storage/service/gateway-service-mem.cpp \
    lorawan/storage/service/identity-service.cpp \
    lorawan/storage/service/identity-service-gen.cpp lorawan/storage/service/identity-service-mem.cpp \
    third-party/strptime.cpp \
    ${AES_SRC}

if ENABLE_SQLITE
    SRC_LIBLORAWAN += lorawan/storage/service/identity-service-sqlite.cpp lorawan/storage/service/gateway-service-sqlite.cpp
endif

liblorawan_a_SOURCES = $(SRC_LIBLORAWAN)
liblorawan_a_CPPFLAGS = -fPIC

lorawan_query_SOURCES = \
    cli-query-main.cpp cli-helper.cpp \
	$(SRC_ARGTABLE)
lorawan_query_LDADD = $(EXTRA_LIB) -L. -llorawan
lorawan_query_CPPFLAGS = $(GATEWAY_DEF)

lorawan_service_SOURCES = cli-main.cpp cli-helper.cpp third-party/daemonize.cpp \
    $(SRC_ARGTABLE)
lorawan_service_LDADD = $(EXTRA_LIB) -L. -llorawan
lorawan_service_CPPFLAGS = $(GATEWAY_DEF) -DLNS_VERSION=$(VERSION)

lorawan_query_direct_SOURCES = cli-query-plugin-main.cpp cli-helper.cpp \
    $(SRC_ARGTABLE)
lorawan_query_direct_LDADD = $(EXTRA_LIB) -L. -llorawan
lorawan_query_direct_CPPFLAGS = $(GATEWAY_DEF)

#
# Plugins
#
libstorage_mem_la_SOURCES = lorawan/storage/service/identity-service-mem.cpp lorawan/storage/service/gateway-service-mem.cpp
libstorage_mem_la_LIBADD = -L. -llorawan
#libstorage_mem_la_LDFLAGS = -version-info $(VERSION_INFO)

libstorage_gen_la_SOURCES = lorawan/storage/service/identity-service-gen.cpp lorawan/storage/service/gateway-service-mem.cpp \
	lorawan/helper/key128gen.cpp $(AES_SRC)
libstorage_gen_la_LIBADD = -L. -llorawan
libstorage_gen_la_CPPFLAGS = -Ithird-party
#libstorage_gen_la_LDFLAGS = -version-info $(VERSION_INFO)
if ENABLE_SQLITE
    libstorage_sqlite_la_SOURCES = lorawan/storage/service/identity-service-sqlite.cpp lorawan/storage/service/gateway-service-sqlite.cpp \
	    lorawan/helper/file-helper.cpp lorawan/helper/sqlite-helper.cpp
    libstorage_sqlite_la_LIBADD = $(EXTRA_LIB) -L. -llorawan
#    libstorage_sqlite_la_LDFLAGS = -version-info $(VERSION_INFO)
endif

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
	third-party/argtable3/README \
	tests/test-parse-packet.cpp \
	examples/example-direct.cpp examples/example-gen.cpp examples/example-gw-mem.cpp examples/example-mem.cpp
#
# Tests
#

# test_key_SOURCES = tests/test-parse-packet.cpp
#check_PROGRAMS =
#if ENABLE_TESTS
#	check_PROGRAMS += test-parse-packet
#endif
#
#TESTS = $(check_PROGRAMS)
