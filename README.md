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

### lorawan-gateway-service

### lorawan-gateway-query

Options

- -c code (account#)
- -a access code hexadecimal 64-bit number e.g. 2A0000002A
- -s service address:port e.g. 10.2.104.51:4242 

Examples:
```
./lorawan-gateway-query 00550116 01450330 34313235 01450340 -c 42 -a 2A0000002A -s 10.2.104.51:4242 
ABP A 323934344a386d0c 3338470c32393434170026004a386d0c 17002600323934343338470c65717b40 1.0.0 0000000000000000 00000000000000000000000000000000 00000000000000000000000000000000 0000 000000 sh-2-1
ABP C 3434383566378112 313747123434383535003a0066378888 35003a003434383531374712656b7f47 1.0.0 0000000000000000 00000000000000000000000000000000 00000000000000000000000000000000 0000 000000 SI-13-23
ABP A 3231323549304c0a 34313235343132353431323534313235 34313235343132353431323534313235 1.0.0 0000000000000000 00000000000000000000000000000000 00000000000000000000000000000000 0000 000000 pak811-1
ABP A 1234567890102030 2b7e151628aed2a6abf7158809cf4f3c 2b7e151628aed2a6abf7158809cf4f3c 1.0.0 0000000000000000 00000000000000000000000000000000 00000000000000000000000000000000 0000 000000 oled
```

## Test

UDP
```
echo '4c0000002a000000000000002a0000000004' | xxd -r -p | nc -u 127.0.0.1 4244 -w 1 | xxd -p
```

TCP 
```
echo '4c0000002a000000000000002a0000000004' | xxd -r -p | nc 127.0.0.1 4244 -w 1 | xxd -p
```

