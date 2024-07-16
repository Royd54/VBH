set(TARGET_NAME w6x00_loopback)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        tcp_api.c
        tcp_server.c
        UART_Communication.c
        )

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        ETHERNET_FILES
        IOLIBRARY_FILES
        LOOPBACK_FILES
        TIMER_FILES
        cJSON_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

target_compile_definitions(${TARGET_NAME} PRIVATE
        PICO_DEFAULT_UART_RX_PIN=16
        PICO_DEFAULT_UART_TX_PIN=17
)

pico_add_extra_outputs(${TARGET_NAME})
