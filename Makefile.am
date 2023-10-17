SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

VERSION = 0.1

# ../../third-party for make distcheck happy
common_flags = -I. -Ithird-party -I../../third-party

AM_CPPFLAGS=$(common_flags)
AM_CXXFLAGS=$(common_flags) -std=c++11
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

ARGTABLE_SRC = \
	third-party/argtable3/argtable3.c
AES_SRC = \
	third-party/system/crypto/aes.c third-party/system/crypto/cmac.c
CLI_SRC = \
	cli-query-main.cpp service-serialization.cpp gateway-serialization.cpp

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
GATEWAY_SRC = identity-service-mem.cpp gateway-service-mem.cpp
endif

# Static library used for build binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

lib_LIBRARIES = liblorawan.a

#
# Binaries
#
bin_PROGRAMS = lorawan-service lorawan-query

SRC_AES = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c
SRC_ARGTABLE = third-party/argtable3/argtable3.c

nobase_dist_include_HEADERS = \
	gateway-client.h log-intf.h \
	identity-service.h identity-service-mem.h identity-service-sqlite.h \
	gateway-service.h gateway-service-mem.h gateway-service-sqlite.h log.h \
	lorawan-error.h lorawan-types.h udp-client.h uv-listener.h \
	storage-listener.h ip-helper.h sqlite-helper.h lorawan-const.h \
	lorawan-msg.h network-identity.h udp-listener.h uv-mem.h \
	service-serialization.h gateway-serialization.h identity-serialization.h \
	lorawan-conv.h lorawan-string.h platform-specific.h uv-client.h \
	lorawan-date.h gateway-identity.h ip-address.h file-helper.h \
	third-party/system/crypto/aes.h third-party/system/crypto/cmac.h \
	third-party/system/crypto/aes.c third-party/system/crypto/cmac.c \
	third-party/argtable3/argtable3.h third-party/strptime.h third-party/daemonize.h

#
# liblorawan
#
SRC_LIBLORAWAN = \
	lorawan-types.cpp lorawan-string.cpp lorawan-conv.cpp network-identity.cpp \
	gateway-identity.cpp lorawan-date.cpp gateway-service.cpp identity-service.cpp \
	ip-helper.cpp ip-address.cpp \
	third-party/strptime.cpp

LORAWAN_SERVICE_SRC = cli-main.cpp $(LSNR_SRC) $(GATEWAY_SRC) \
    identity-serialization.cpp service-serialization.cpp gateway-serialization.cpp \
	file-helper.cpp third-party/daemonize.cpp $(SRC_ARGTABLE)

liblorawan_a_SOURCES = $(SRC_LIBLORAWAN)
liblorawan_a_CPPFLAGS = -fPIC

lorawan_query_SOURCES = \
	$(CLI_SRC) $(SRC_LIBLORAWAN) \
	$(SRC_ARGTABLE)
lorawan_query_LDADD = $(EXTRA_LIB)
lorawan_query_CPPFLAGS = $(GATEWAY_DEF)

lorawan_service_SOURCES = \
	$(LORAWAN_SERVICE_SRC) $(SRC_LIBLORAWAN)
lorawan_service_LDADD = $(EXTRA_LIB_SVC) $(EXTRA_LIB)
lorawan_service_CPPFLAGS = $(GATEWAY_DEF) -DLNS_VERSION=$(VERSION)

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
