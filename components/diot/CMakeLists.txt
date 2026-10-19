idf_component_register(
    SRCS "diot_button.c" "diot_bme680.c" "diot_oled.c"
    INCLUDE_DIRS "include"
    REQUIRES fsm esp_driver_gpio esp_timer esp_hw_support
)
