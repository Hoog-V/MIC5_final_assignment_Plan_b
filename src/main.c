#include "hardware/structs/rosc.h"
#include <stdio.h>

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "lwip/altcp_tcp.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"

#include "tusb.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include <task.h>
#include "board_definitions.hpp"

#include "u8g2.h"

#include "math.h"
#include "display_helper.h"
#include "mfrc522.h"


u8g2_t u8g2_inst;

struct MFRC522_T mfrc_Instance;

void led_task(void *arg)
{
    while (true)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(250/portTICK_PERIOD_MS);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(250/portTICK_PERIOD_MS);
        u8g2_SetDrawColor(&u8g2_inst, 1);
        u8g2_DrawHLine(&u8g2_inst, 0, 50, 128);
        u8g2_SendBuffer(&u8g2_inst);
        vTaskDelay(500/portTICK_PERIOD_MS);
        u8g2_SetDrawColor(&u8g2_inst, 0);
        u8g2_DrawHLine(&u8g2_inst, 0, 50, 128);
        u8g2_SendBuffer(&u8g2_inst);
        vTaskDelay(500/portTICK_PERIOD_MS);

    }
}


MFRC522Ptr_t mfrc_inst;


void wait_for_card_and_print() {
        printf("Waiting for card\n\r");
        while(!PICC_IsNewCardPresent(&mfrc_Instance));
        //Select the card
        printf("Selecting card\n\r");
        PICC_ReadCardSerial(&mfrc_Instance);

        //Show UID on serial monitor
        printf("PICC dump: \n\r");
        PICC_DumpToSerial(&mfrc_Instance, &(mfrc_Instance.uid));
}

int main()
{
    stdio_init_all();

    // printf("waiting for usb host");
    // while (!tud_cdc_connected()) {
    //     printf(".");
      //   sleep_ms(500);
    // }
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }

    for(uint8_t i =0; i< BUFFER_SIZE; i++) {
        mfrc_Instance.Rx_Buf[i] = 0;
        mfrc_Instance.Tx_Buf[i] = 0;
    }
        
    gpio_set_function(DISPLAY_RST, GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_RST, GPIO_OUT);

    gpio_set_function(DISPLAY_DC, GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_DC, GPIO_OUT);

    gpio_set_function(DISPLAY_CS, GPIO_FUNC_SIO);
    gpio_set_dir(DISPLAY_CS, GPIO_OUT);

    gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(RFID_MISO, GPIO_FUNC_SPI);

    gpio_set_function(DISPLAY_BL, GPIO_FUNC_PWM);

    gpio_set_function(RFID_RST, GPIO_FUNC_SIO);
    gpio_set_dir(RFID_RST, GPIO_OUT);

    gpio_set_function(RFID_CS, GPIO_FUNC_SIO);
    gpio_set_dir(RFID_CS, GPIO_OUT);

    display_init_backlight();
    display_set_backlight(100);
    
    u8g2_Setup_st7567_os12864_f(&u8g2_inst, U8G2_R2, u8x8_byte_hw_spi, u8x8_gpio_and_delay_template);
    u8g2_InitDisplay(&u8g2_inst);
    u8g2_SetPowerSave(&u8g2_inst, 0); // wake up display
    u8g2_SetContrast(&u8g2_inst, 30);
    // u8g2_DrawLine(&u8g2_inst, 128,64, 0, 0);
    u8g2_SetFont(&u8g2_inst, u8g2_font_ncenB14_tf);
    u8g2_DrawStr(&u8g2_inst, 20, 20, "Test!");
    u8g2_SendBuffer(&u8g2_inst);

    PCD_Init(&mfrc_Instance, spi1, RFID_RST, RFID_CS);
    PCD_DumpVersionToSerial(&mfrc_Instance);
      int status = xTaskCreate(led_task, "Led blinky", 2048, NULL, 1, NULL);
     if(status != pdPASS) {
        while(1);
    }

    vTaskStartScheduler();


    while (true)
    {

    }
    cyw43_arch_deinit();

    return 0;
}
