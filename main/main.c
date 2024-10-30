#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "driver/gpio.h"

#define LED_PIN 27
#define PUSH_BUTTON_PIN 33

TaskHandle_t myTaskHandle = NULL;
QueueHandle_t queue;

void Task(void *arg)
{

    char rxBuffer;
    bool led_status = false; 

    while (1)
    {

        if (xQueueReceive(queue, &(rxBuffer), (TickType_t) 5))
        {

            led_status = !led_status;
            gpio_set_level(LED_PIN, led_status);
            printf("Button pressed! LED %s\n", led_status ? "ON" : "OFF");
            vTaskDelay(1000 / portTICK_PERIOD_MS); 
        }
    }
}

void IRAM_ATTR button_isr_handler(void *arg)
{

    char cIn;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    cIn = '1'; 
    xQueueSendFromISR(queue, &cIn, &xHigherPriorityTaskWoken);
}

void app_main(void)
{

    esp_rom_gpio_pad_select_gpio(PUSH_BUTTON_PIN);
    esp_rom_gpio_pad_select_gpio(LED_PIN);

    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_intr_type(PUSH_BUTTON_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(PUSH_BUTTON_PIN, button_isr_handler, NULL);


    queue = xQueueCreate(1, sizeof(char));


    if (queue == 0)
    {
        printf("Failed to create queue= %p\n", queue);
    }


    xTaskCreatePinnedToCore(Task, "My_Task", 4096, NULL, 10, &myTaskHandle, 1);
}
