set(AES_SRC 
        ../third-party/system/crypto/aes.c
        ../third-party/system/crypto/cmac.c
)

set(IDENTITY_SRC
        ../lorawan-types.cpp
        ../lorawan-string.cpp
        ../lorawan-conv.cpp
        ../network-identity.cpp
        ../identity-service.cpp
        ../udp-listener.cpp
        ../identity-serialization.cpp
        ../udp-listener.cpp
        ../ip-helper.cpp
)

if(CONFIG_ESP_KEY_GEN)
        set(IDENTITY_SRC ${IDENTITY_SRC} ../identity-service-gen.cpp ../key128gen.cpp ${AES_SRC})
else()
        set(IDENTITY_SRC ${IDENTITY_SRC} ../identity-service-mem.cpp)        
endif()

idf_component_register(
SRCS
        esp32-main.cpp
        wifi-station.c
        ${IDENTITY_SRC}
INCLUDE_DIRS
        "." ".." "../third-party"
REQUIRES
        nvs_flash
        esp_wifi        
)
