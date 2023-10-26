# LoRaWAN gateway storage

This is a simple example of a gateway database service for a tlns project.

## Requisites

### Linux

- CMake or Autotools/Automake
- C++ compiler and development tools bundle

### Windows

- CMake
- Visual Studio
- vcpkg

### ESP32

- SDK idf tools or
- Visual Studio Code with Espressif IDF plugin

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
cd lorawan-storage
./autogen.sh
./configure
make
```

Set ./configure command line options:

- --enable-libuv use libuv
- --enable-debug debug print on
- --enable-gen enable key generator
- --enable-sqlite enable SSQLite backend
- --enable-ipv6 enable IPv6 (reserved for future use)
- --enable-tests enable tests

For instance

```
cd lorawan-storage
./autogen.sh
./configure --enable-sqlite=yes --enable-gen=no 
make
```

enables SQLite backend

### CMake

Configure by default: 
```
cd lorawan-storage
mkdir build
cd build
cmake ..
make
```

Enable/disable options:
```
cd lorawan-storage
mkdir build
cd build
cmake -DENABLE_LIBUV=off -DENABLE_DEBUG=off -DENABLE_GEN=on -DENABLE_SQLITE=off -DENABLE_IPV6=off ..
make
```

Options are:

- -DENABLE_LIBUV use libuv
- -DENABLE_DEBUG debug print on
- -DENABLE_SQLITE enable SQLite backend
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

SDK

```
cd lorawan-storage
idf_get
idf.py menuconfig
idf.py build
```

Visual Studio Code

- Press F1; select ESP-IDF: Set Espressif device target; select lorawan-storage; select ESP32; select ESP32 chip (via ESP USB bridge)
- Press F1; select ESP-IDF: Build your project
- Press F1; select ESP-IDF: Flush your project

### Dependencies

- gettext
- libuv

```
sudo apt install libuv1-dev gettext 
```

## Usage

- lorawan-service
- lorawan-query

### lorawan-service

### lorawan-query

Manipulate device records by commands:

- address <identifier>
- identifier <address>
- assign {<record>}
- list
- remove <address> | <identifier>

record is a comma-separated string consists of
- address
- activation type: ABP or OTAA
- device class;: A, B or C
- device EUI identifier (up to 16 hexadecimal digits, 8 bytes)
- nwkSKey- shared session key, 16 bytes
- appSKey- private key, 16 bytes
- LoRaWAN version, e.g. 1.0.0
- appEUI OTAA application identifier
- appKey OTAA application private key
- nwkKey OTAA network key
- devNonce last device nonce, 2 bytes
- joinNonce last Join nonce, 3 bytes
- device name (up to 8 ASCII characters)

e.g.
```
aabbccdd,OTAA,A,2233445566778899,112233445566778899aabbccddeeff00,55000000000000000000000000000066,1.0.0,ff000000000000ff,cc0000000000000000000000000000dd,77000000000000000000000000000088,1a2b,112233,DeviceNam
```
is a record of device with assigned address 'aabbccdd'. 

Create an empty record with reserved address 11aa22bb: 

```
./lorawan-query assign 11aa22bb
```

In the example above all properties except address skipped.  

Query device record by address:
```
./lorawan-query identifier aabbccdd
```

Remove record by address: 
```
./lorawan-query remove aabbccdd
```

List records
```
./lorawan-query list
```

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
