set(AES_SRC
        ../third-party/system/crypto/aes.c
        ../third-party/system/crypto/cmac.c
)

set(IDENTITY_SRC
        ../lorawan/lorawan-types.cpp
        ../lorawan/lorawan-string.cpp
        ../lorawan/lorawan-conv.cpp
        ../network-identity.cpp
        ../gateway-identity.cpp
        ../identity-service.cpp
        ../gateway-service.cpp
        ../gateway-service-mem.cpp
        ../udp-listener.cpp
        ../identity-serialization.cpp
        ../gateway-serialization.cpp
        ../udp-listener.cpp
        ../service-serialization.cpp
        ../ip-helper.cpp
        ../ip-address.cpp
)

if(CONFIG_ESP_KEY_GEN)
        set(IDENTITY_SRC ${IDENTITY_SRC} ../identity-service-gen.cpp ../key128gen.cpp ${AES_SRC})
else()
        set(IDENTITY_SRC ${IDENTITY_SRC} ../identity-service-mem.cpp)        
endif()

idf_component_register(
SRCS
        esp32-storage.cpp
        wifi-station.c
        ${IDENTITY_SRC}
INCLUDE_DIRS
        "." ".." "../third-party"
REQUIRES
        nvs_flash
        esp_wifi        
)
