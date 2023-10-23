# LoRaWAN gateway storage

This is a simple example of a gateway database service for a tlns project.

## Requisites

### Linux

- CMake or Autotools/Automake
- C++ compiler and development tools bundle

#### Tools

Make sure you have automake or CMake installed:

```
apt install autoconf libtool build-essential 
```

or 

```
apt install cmake build-essential 
```

## Build

### Autotools

First install dependencies (see below) and then configure and make project using Autotools:

```
cd lorawan-gateway-storage
./autogen.sh
./configure
make
```

Set ./configure command line options 
- --enable-libuv use libuv
- --enable-debug debug print on
- --enable-gen enable key generator
- --enable-ipv6 enable IPv6 (reserved for future use)
- --enable-tests enable tests

### CMake

Configure by default: 
```
cd lorawan-gateway-storage
mkdir build
cd build
cmake ..
make
```

Enable/disable options:
```
cd lorawan-gateway-storage
mkdir build
cd build
cmake -DENABLE_LIBUV=off -DENABLE_DEBUG=off -DENABLE_GEN=on -DENABLE_IPV6=off ..
make
```

Options are:

- -DENABLE_LIBUV use libuv
- -DENABLE_DEBUG debug print on
- -DENABLE_GEN enable key generator
- -DENABLE_IPV6 enable IPv6 (reserved for future use)

#### clang instead of gcc

Export CC and CXX environment variables points to specific compiler binaries.
For instance, you can use Clang instead of gcc:

```
cd lorawan-gateway-storage
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake -DENABLE_WS=on -DENABLE_JWT=on -DENABLE_PKT2=on -DENABLE_MQTT=on -DENABLE_LOGGER_HUFFMAN=on ..
make
```

### Windows

- Visual Studio with C++ profile installed
- CMake
- vcpkg

### ESP32

```
cd lorawan-gateway-storage
idf_get
idf.py menuconfig
idf.py build
```

### Dependencies

- gettext
- libuv

```
sudo apt install libuv1-dev gettext 
```

## Usage

- lorawan-gateway-service
- lorawan-gateway-query

### lorawan-service

### lorawan-query

Manipulate gateway records by commands:

- gw-address <identifier>
- gw-identifier <address> 
- gw-assign {<address> <identifier>}
- gw-list
- gw-remove <address> | <identifier> 

gw-remove remove record by address or identifier

Options:

- -c code (account#)
- -a access code hexadecimal 64-bit number e.g. 2A0000002A
- -s service address:port e.g. 10.2.104.51:4242
- -o offset default 0. Applicable for gw-list only
- -z size default 10. Applicable for gw-list only

Examples:

```
./lorawan-query gw-assign 11 1.2.3.4:5 12 1.2.3.4:5 13 1.2.3.4:5 14 1.2.3.4:5 15 1.2.3.4:5 16 1.2.3.4:5 17 1.2.3.4:5 18 1.2.3.4:5 19 1.2.3.4:5 20 1.2.3.4:5 21 1.2.3.4:5 22 1.2.3.4:5 23 1.2.3.4:5 24 1.2.3.4:5 25 1.2.3.4:5 26 1.2.3.4:5 27 1.2.3.4:5 28 1.2.3.4:5 29 1.2.3.4:5 30 1.2.3.4:5 -v
./lorawan-query gw-list
./lorawan-query gw-list -o 9
./lorawan-query gw-remove 12
./lorawan-query gw-list
./lorawan-query gw-address 11
./lorawan-query gw-address 1.2.3.4:5
./lorawan-query gw-identifier 1.2.3.4:5
```

## Tests

### Test gateway storage

UDP
```
echo '4c0000002a000000000000002a000000000a' | xxd -r -p | nc -u 127.0.0.1 4244 -w 1 | xxd -p
```

TCP 
```
echo '4c0000002a000000000000002a000000000a' | xxd -r -p | nc 127.0.0.1 4244 -w 1 | xxd -p
```

