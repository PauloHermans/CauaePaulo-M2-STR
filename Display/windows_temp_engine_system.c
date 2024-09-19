#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/temperature_sensor.h"
#include "esp_attr.h"

static const char *TAG = "example";

IRAM_ATTR static bool temp_sensor_monitor_cbs(temperature_sensor_handle_t tsens, const temperature_sensor_threshold_event_data_t *edata, void *user_data)
{
    ESP_DRAM_LOGI("tsens", "Temperature value is higher or lower than threshold, value is %d\n...\n\n", edata->celsius_value);
    return false;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Install temperature sensor, expected temp range: -10~90 ℃");
    temperature_sensor_handle_t temp_sensor = NULL;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 90);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));

    temperature_sensor_event_callbacks_t cbs = {
        .on_threshold = temp_sensor_monitor_cbs,
    };

    temperature_sensor_abs_threshold_config_t threshold_cfg = {
        .high_threshold = 90,
        .low_threshold = -10,
    };
    ESP_ERROR_CHECK(temperature_sensor_set_absolute_threshold(temp_sensor, &threshold_cfg));

    ESP_ERROR_CHECK(temperature_sensor_register_callbacks(temp_sensor, &cbs, NULL));

    ESP_LOGI(TAG, "Enable temperature sensor");
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

    ESP_LOGI(TAG, "Monitor temperature every 19ms");
    float tsens_value;

    while (1) {
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
        ESP_LOGI(TAG, "Temperature value %.02f ℃", tsens_value);

        // Verificar se a temperatura está fora do intervalo de -10 a 90 graus Celsius
        if (tsens_value < -10 || tsens_value > 90) {
            ESP_LOGW(TAG, "Temperature is out of range! Current value: %.02f ℃", tsens_value);
        }

        // Esperar 19ms antes da próxima leitura
        vTaskDelay(19 / portTICK_PERIOD_MS);
    }
}
