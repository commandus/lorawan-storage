SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

VERSION = 0.1

# ../../third-party for make distcheck happy
common_flags = -I. -Ithird-party -I../../third-party

AM_CPPFLAGS=$(common_flags)
AM_CXXFLAGS=$(common_flags) -std=c++11
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

ARGTABLE_SRC = third-party/argtable3/argtable3.c
AES_SRC = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c
CLI_SRC = cli-query-main.cpp identity-serialization.cpp gateway-serialization.cpp

LSNR_SRC =
GATEWAY_DEF = $(COMMON_CPP_FLAGS)
EXTRA_LIB =
if ENABLE_LIBUV
LSNR_SRC += uv-listener.cpp uv-mem.cpp
CLI_SRC += uv-client.cpp uv-mem.cpp
GATEWAY_DEF += -DENABLE_LIBUV
EXTRA_LIB += -luv
else
LSNR_SRC += udp-listener.cpp
CLI_SRC += udp-client.cpp
endif

if ENABLE_SQLITE
GATEWAY_DEF += -DENABLE_SQLITE
GATEWAY_SRC = identity-service-sqlite.cpp gateway-service-sqlite.cpp sqlite-helper.cpp
EXTRA_LIB_SVC = -lsqlite3
else
if ENABLE_GEN
GATEWAY_DEF += -DENABLE_GEN
GATEWAY_SRC = identity-service-gen.cpp gateway-service-mem.cpp key128gen.cpp $(AES_SRC)
else
GATEWAY_SRC = identity-service-mem.cpp gateway-service-mem.cpp
endif
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
bin_PROGRAMS = lorawan-service lorawan-query lorawan-query-plugin

SRC_AES = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c
SRC_ARGTABLE = third-party/argtable3/argtable3.c

nobase_dist_include_HEADERS = \
	query-client.h log-intf.h \
	identity-service.h async-wrapper-identity-service.h identity-service-mem.h identity-service-sqlite.h \
	gateway-service.h async-wrapper-gateway-service.h gateway-service-mem.h gateway-service-sqlite.h log.h \
	identity-service-gen.h key128gen.h \
	lorawan/lorawan-error.h lorawan/lorawan-types.h lorawan/lorawan-const.h \
	lorawan/lorawan-msg.h lorawan/lorawan-conv.h lorawan/lorawan-string.h \
	lorawan/lorawan-date.h \
	udp-client.h uv-listener.h \
	storage-listener.h ip-helper.h sqlite-helper.h  \
	network-identity.h udp-listener.h uv-mem.h \
	service-serialization.h gateway-serialization.h identity-serialization.h \
	platform-specific.h uv-client.h \
	gateway-identity.h ip-address.h file-helper.h \
	third-party/system/crypto/aes.h third-party/system/crypto/cmac.h \
	third-party/system/crypto/aes.c third-party/system/crypto/cmac.c \
	third-party/argtable3/argtable3.h third-party/strptime.h third-party/daemonize.h

#
# liblorawan
#
SRC_LIBLORAWAN = \
    lorawan/lorawan-types.cpp lorawan/lorawan-string.cpp lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp \
    lorawan/lorawan-mac.cpp lorawan/lorawan-error.cpp lorawan/lorawan-msg.cpp \
    network-identity.cpp gateway-identity.cpp \
    gateway-service.cpp identity-service.cpp service-serialization.cpp \
    ip-helper.cpp ip-address.cpp \
    third-party/strptime.cpp

LORAWAN_SERVICE_SRC = cli-main.cpp $(LSNR_SRC) $(GATEWAY_SRC) \
    identity-serialization.cpp gateway-serialization.cpp \
    file-helper.cpp third-party/daemonize.cpp $(SRC_ARGTABLE)

liblorawan_a_SOURCES = $(SRC_LIBLORAWAN)
liblorawan_a_CPPFLAGS = -fPIC

lorawan_query_SOURCES = \
	$(CLI_SRC) cli-helper.cpp $(SRC_LIBLORAWAN) \
	$(SRC_ARGTABLE)
lorawan_query_LDADD = $(EXTRA_LIB)
lorawan_query_CPPFLAGS = $(GATEWAY_DEF)

lorawan_service_SOURCES = \
	$(LORAWAN_SERVICE_SRC) $(SRC_LIBLORAWAN)
lorawan_service_LDADD = $(EXTRA_LIB_SVC) $(EXTRA_LIB)
lorawan_service_CPPFLAGS = $(GATEWAY_DEF) -DLNS_VERSION=$(VERSION)

lorawan_query_plugin_SOURCES = \
    cli-query-plugin-main.cpp plugin-client.cpp cli-helper.cpp \
    file-helper.cpp identity-serialization.cpp gateway-serialization.cpp \
    service-client.cpp identity-service-mem.cpp gateway-service-mem.cpp identity-service-gen.cpp key128gen.cpp \
    $(AES_SRC) $(SRC_ARGTABLE) $(SRC_LIBLORAWAN)
lorawan_query_plugin_LDADD = $(EXTRA_LIB)
lorawan_query_plugin_CPPFLAGS = $(GATEWAY_DEF)
if ENABLE_SQLITE
    lorawan_query_plugin_SOURCES += identity-service-sqlite.cpp gateway-service-sqlite.cpp sqlite-helper.cpp
endif

#
# Plugins
#
libstorage_mem_la_SOURCES = identity-service-mem.cpp gateway-service-mem.cpp
libstorage_mem_la_LIBADD = -L. -llorawan
libstorage_gen_la_SOURCES = identity-service-gen.cpp gateway-service-mem.cpp key128gen.cpp $(AES_SRC)
libstorage_gen_la_LIBADD = -L. -llorawan
libstorage_gen_la_CPPFLAGS = -Ithird-party
if ENABLE_SQLITE
    libstorage_sqlite_la_SOURCES = identity-service-sqlite.cpp gateway-service-sqlite.cpp file-helper.cpp sqlite-helper.cpp
    libstorage_sqlite_la_LIBADD = -L. -llorawan $(EXTRA_LIB_SVC)
endif

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
	third-party/argtable3/README \
	tests/test-parse-packet.cpp
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
