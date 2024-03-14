# Install script for directory: D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/RP2040-v6-HAT-C")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/aes.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/aria.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/asn1.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/asn1write.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/base64.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/bignum.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/build_info.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/camellia.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ccm.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/chacha20.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/chachapoly.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/check_config.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/cipher.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/cmac.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/compat-2.x.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/config_psa.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ctr_drbg.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/debug.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/des.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/dhm.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ecdh.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ecdsa.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ecjpake.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ecp.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/entropy.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/error.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/gcm.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/hkdf.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/hmac_drbg.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/mbedtls_config.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/md.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/md5.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/net_sockets.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/nist_kw.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/oid.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/pem.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/pk.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/pkcs12.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/pkcs5.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/platform.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/platform_time.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/platform_util.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/poly1305.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/private_access.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/psa_util.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ripemd160.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/rsa.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/sha1.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/sha256.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/sha512.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ssl.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ssl_cache.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ssl_cookie.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/ssl_ticket.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/threading.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/timing.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/version.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/x509.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/x509_crl.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/x509_crt.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/mbedtls/x509_csr.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_builtin_composites.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_builtin_primitives.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_compat.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_config.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_driver_common.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_extra.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_platform.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_se_driver.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_sizes.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_struct.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_types.h"
    "D:/RP2040/RP2040-v6-HAT-C/libraries/mbedtls/include/psa/crypto_values.h"
    )
endif()

