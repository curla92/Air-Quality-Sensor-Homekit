#include <stdio.h>
#include <math.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <esplibs/libmain.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "../common/custom_characteristics.h"
#include <wifi_config.h>

#include <led_codes.h>
#include <adv_button.h>
#include <dht/dht.h>
#include <esp8266_mq135.h>

#include <sysparam.h>
#include <rboot-api.h>

#define SHOW_SETUP_SYSPARAM "A"
#define TEMP_OFFSET "B"
#define HUM_OFFSET "C"

#define ALLOWED_FACTORY_RESET_TIME 60000

const int led_gpio = 5; // LED on pin D1
const int TEMPERATURE_SENSOR_PIN = 4; // DHT sensor on pin D2

uint8_t reset_toggle_counter = 0;

volatile float old_humidity_value = 0.0, old_temperature_value = 0.0;
float humidity_value, temperature_value;

ETSTimer device_restart_timer, change_settings_timer, reset_toggle_timer, extra_func_timer;

volatile bool paired = false;
volatile bool Wifi_Connected = false;

homekit_value_t read_ip_addr();

void show_setup_callback();
void change_settings_callback();
void reboot_callback();
void ota_firmware_callback();

// GENERAL AND CUSTOM
homekit_characteristic_t show_setup = HOMEKIT_CHARACTERISTIC_(CUSTOM_SHOW_SETUP, true, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(show_setup_callback));
homekit_characteristic_t device_type_name = HOMEKIT_CHARACTERISTIC_(CUSTOM_DEVICE_TYPE_NAME, "Air Quality Sensor", .id=107);
homekit_characteristic_t temp_offset = HOMEKIT_CHARACTERISTIC_(CUSTOM_TEMPERATURE_OFFSET, 0.0, .id = 112, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(change_settings_callback));
homekit_characteristic_t hum_offset = HOMEKIT_CHARACTERISTIC_(CUSTOM_HUMIDITY_OFFSET, 0, .id = 113, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(change_settings_callback));
homekit_characteristic_t ip_addr = HOMEKIT_CHARACTERISTIC_(CUSTOM_IP_ADDR, "", .id = 114, .getter = read_ip_addr);
homekit_characteristic_t wifi_reset = HOMEKIT_CHARACTERISTIC_(CUSTOM_WIFI_RESET, false, .id=115, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(change_settings_callback));
homekit_characteristic_t reboot_device = HOMEKIT_CHARACTERISTIC_(CUSTOM_REBOOT_DEVICE, false, .id = 116, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(reboot_callback));
homekit_characteristic_t ota_firmware = HOMEKIT_CHARACTERISTIC_(CUSTOM_OTA_UPDATE, false, .id = 117, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(ota_firmware_callback));

//temperature sensor
homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0 );
//humidity sensor
homekit_characteristic_t current_humidity = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);

//air quality sensor
homekit_characteristic_t air_quality = HOMEKIT_CHARACTERISTIC_( AIR_QUALITY, 0 );
homekit_characteristic_t carbon_monoxide_level  = HOMEKIT_CHARACTERISTIC_( CARBON_MONOXIDE_LEVEL, 0 );
homekit_characteristic_t carbon_dioxide_level  = HOMEKIT_CHARACTERISTIC_( CARBON_DIOXIDE_LEVEL, 0 );

// CUSTOM ETHANOL/AMMONIUM/TOLUENE/ACETONE
homekit_characteristic_t ethanol_level = HOMEKIT_CHARACTERISTIC_( CUSTOM_ETHANOL_LEVEL, 0 );
homekit_characteristic_t ammonium_level = HOMEKIT_CHARACTERISTIC_( CUSTOM_AMMONIUM_LEVEL, 0 );
homekit_characteristic_t toluene_level = HOMEKIT_CHARACTERISTIC_( CUSTOM_TOLUENE_LEVEL, 0 );
homekit_characteristic_t acetone_level = HOMEKIT_CHARACTERISTIC_( CUSTOM_ACETONE_LEVEL, 0 );


// IDENTIFY
void identify_task(void *_args) {
    // Identify dispositive by Pulsing LED.
    led_code (led_gpio, IDENTIFY_ACCESSORY);
    vTaskDelete(NULL);
}

void identify(homekit_value_t _value) {
    printf("Air Quality Sensor identify\n");
    xTaskCreate(identify_task, "Air Quality Sensor identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// CHANGE SETTINGS
void change_settings_callback() {
    sdk_os_timer_arm(&change_settings_timer, 1000, 0);
}

// SAVE SETTINGS
void save_settings() {
    sysparam_status_t status, flash_error = SYSPARAM_OK;
    
    status = sysparam_set_bool(SHOW_SETUP_SYSPARAM, show_setup.value.bool_value);
    if (status != SYSPARAM_OK) {
        flash_error = status;
    }
    
    status = sysparam_set_int8(HUM_OFFSET, hum_offset.value.int_value);
    if (status != SYSPARAM_OK) {
        flash_error = status;
    }
    
    status = sysparam_set_int32(TEMP_OFFSET, temp_offset.value.float_value * 100);
    if (status != SYSPARAM_OK) {
        flash_error = status;
    }
    
    
    if (flash_error != SYSPARAM_OK) {
        printf("Saving settings error -> %i\n", flash_error);
    }
}

// SETTINGS INIT
void settings_init() {
    sysparam_status_t status, flash_error = SYSPARAM_OK;
    
    bool bool_value;
    
    int8_t int8_value;
    int32_t int32_value;
    
    status = sysparam_get_bool(SHOW_SETUP_SYSPARAM, &bool_value);
    if (status == SYSPARAM_OK) {
        show_setup.value.bool_value = bool_value;
    } else {
        status = sysparam_set_bool(SHOW_SETUP_SYSPARAM, true);
        if (status != SYSPARAM_OK) {
            flash_error = status;
        }
    }
    
    status = sysparam_get_int8(HUM_OFFSET, &int8_value);
    if (status == SYSPARAM_OK) {
        hum_offset.value.int_value = int8_value;
    } else {
        status = sysparam_set_int8(HUM_OFFSET, 0);
        if (status != SYSPARAM_OK) {
            flash_error = status;
        }
    }
    
    status = sysparam_get_int32(TEMP_OFFSET, &int32_value);
    if (status == SYSPARAM_OK) {
        temp_offset.value.float_value = int32_value / 100.00f;
    } else {
        status = sysparam_set_int32(TEMP_OFFSET, 0);
        if (status != SYSPARAM_OK) {
            flash_error = status;
        }
    }
    
    if (flash_error == SYSPARAM_OK) {
    } else {
        printf("ERROR settings INIT -> %i\n", flash_error);
    }
}

// IP Address
homekit_value_t read_ip_addr() {
    struct ip_info info;
    
    if (sdk_wifi_get_ip_info(STATION_IF, &info)) {
        char *buffer = malloc(16);
        snprintf(buffer, 16, IPSTR, IP2STR(&info.ip));
        return HOMEKIT_STRING(buffer);
    }
    return HOMEKIT_STRING("");
}

// RESTART
void device_restart_task() {
    vTaskDelay(5500 / portTICK_PERIOD_MS);
    
    if (wifi_reset.value.bool_value) {
        wifi_config_reset();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    
    if (ota_firmware.value.bool_value) {
        rboot_set_temp_rom(1);
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
    
    sdk_system_restart();
    vTaskDelete(NULL);
}

void device_restart() {
    printf("Restarting device\n");
    led_code(led_gpio, RESTART_DEVICE);
    xTaskCreate(device_restart_task, "device_restart_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}

// RESET CONFIG
void reset_configuration_task() {
    led_code(led_gpio, WIFI_CONFIG_RESET);
    printf("Resetting Wifi Config\n");
    
    wifi_config_reset();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    printf("Resetting HomeKit Config\n");
    homekit_server_reset();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Restarting\n");
    
    sdk_system_restart();
    vTaskDelete(NULL);
}

void reset_mode_call(const uint8_t gpio, void *args) {
    if (xTaskGetTickCountFromISR() < ALLOWED_FACTORY_RESET_TIME / portTICK_PERIOD_MS) {
        xTaskCreate(reset_configuration_task, "Reset configuration", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    } else {
        printf("Factory reset not allowed after %ims since boot. Repower device and try again\n", ALLOWED_FACTORY_RESET_TIME);
    }
}

// RESET COUNT CALLBACK
void reset_toggle_upcount() {
    reset_toggle_counter++;
    sdk_os_timer_arm(&reset_toggle_timer, 3100, 0);
}

void reset_toggle() {
    if (reset_toggle_counter > 10) {
        reset_mode_call(0, NULL);
    }
    reset_toggle_counter = 0;
}

// REBOOT
void reboot_callback() {
    if (reboot_device.value.bool_value) {
        sdk_os_timer_setfn(&device_restart_timer, device_restart, NULL);
        sdk_os_timer_arm(&device_restart_timer, 5000, 0);
    } else {
        sdk_os_timer_disarm(&device_restart_timer);
    }
}

// OTA
void ota_firmware_callback() {
    if (ota_firmware.value.bool_value) {
        sdk_os_timer_setfn(&device_restart_timer, device_restart, NULL);
        sdk_os_timer_arm(&device_restart_timer, 5000, 0);
    } else {
        sdk_os_timer_disarm(&device_restart_timer);
    }
}

// SHOW SETUP CALLBACK
void show_setup_callback() {
    if (show_setup.value.bool_value) {
        sdk_os_timer_setfn(&device_restart_timer, device_restart, NULL);
        sdk_os_timer_arm(&device_restart_timer, 5000, 0);
    } else {
        sdk_os_timer_disarm(&device_restart_timer);
    }
    save_settings();
    reset_toggle_upcount();
}

void air_quality_sensor_task() {
    while (1) {
        
        MQGetReadings(temperature_value, humidity_value);
        printf("Got air quality level: %i\n", air_quality_val);
        
        if (CO_val < *carbon_monoxide_level.min_value ){
            CO_val = *carbon_monoxide_level.min_value;
        }
        if (CO_val > *carbon_monoxide_level.max_value ){
            CO_val = *carbon_monoxide_level.max_value;
        }
        carbon_monoxide_level.value.float_value = CO_val;
        
        if (CO2_val < *carbon_dioxide_level.min_value ){
            CO2_val = *carbon_dioxide_level.min_value;
        }
        if (CO2_val > *carbon_dioxide_level.max_value ){
            CO2_val = *carbon_dioxide_level.max_value;
        }
        carbon_dioxide_level.value.float_value = CO2_val;
        
        if (Ethanol_val < *ethanol_level.min_value ){
            Ethanol_val = *ethanol_level.min_value;
        }
        if (Ethanol_val > *ethanol_level.max_value ){
            Ethanol_val = *ethanol_level.max_value;
        }
        ethanol_level.value.float_value = Ethanol_val;
        
        if (NH4_val < *ammonium_level.min_value ){
            NH4_val = *ammonium_level.min_value;
        }
        if (NH4_val > *ammonium_level.max_value ){
            NH4_val = *ammonium_level.max_value;
        }
        ammonium_level.value.float_value = NH4_val;
        
        if (Toluene_val < *toluene_level.min_value ){
            Toluene_val = *toluene_level.min_value;
        }
        if (Toluene_val > *toluene_level.max_value ){
            Toluene_val = *toluene_level.max_value;
        }
        toluene_level.value.float_value = Toluene_val;
        
        if (Acetone_val < *acetone_level.min_value ){
            Acetone_val = *acetone_level.min_value;
        }
        if (Acetone_val > *acetone_level.max_value ){
            Acetone_val = *acetone_level.max_value;
        }
        acetone_level.value.float_value = Acetone_val;
        
        air_quality.value.int_value = air_quality_val;
        
        homekit_characteristic_notify(&air_quality, air_quality.value);
        homekit_characteristic_notify(&carbon_monoxide_level, HOMEKIT_FLOAT(CO_val));
        homekit_characteristic_notify(&carbon_dioxide_level, HOMEKIT_FLOAT(CO2_val));
        homekit_characteristic_notify(&ethanol_level, HOMEKIT_FLOAT(Ethanol_val));
        homekit_characteristic_notify(&ammonium_level, HOMEKIT_FLOAT(NH4_val));
        homekit_characteristic_notify(&toluene_level, HOMEKIT_FLOAT(Toluene_val));
        homekit_characteristic_notify(&acetone_level, HOMEKIT_FLOAT(Acetone_val));
        
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void air_quality_sensor() {
    xTaskCreate(air_quality_sensor_task, "Air Quality Sensor", 512, NULL, 2, NULL);
}

// TEMP HUM FUNCTION
void temperature_sensor() {
    // Temperature measurement
    float humidity_value, temperature_value;
    bool get_temp = false;
    
    get_temp = dht_read_float_data(DHT_TYPE_DHT22, TEMPERATURE_SENSOR_PIN, &humidity_value, &temperature_value);
    
    if (get_temp) {
        temperature_value += temp_offset.value.float_value;
        humidity_value += hum_offset.value.float_value;
        // printf("Sensor: temperature %g, humidity %g\n", temperature_value, humidity_value);
        
        if (temperature_value != old_temperature_value) {
            
            old_temperature_value = temperature_value;
            current_temperature.value = HOMEKIT_FLOAT(temperature_value);
            homekit_characteristic_notify( &current_temperature, HOMEKIT_FLOAT(temperature_value));
        }
        if (humidity_value != old_humidity_value) {
            
            old_humidity_value = humidity_value;
            current_humidity.value = HOMEKIT_FLOAT(humidity_value);
            homekit_characteristic_notify( & current_humidity, current_humidity.value);
        }
    }else{
        // led_code(led_gpio, SENSOR_ERROR);
        printf("Couldnt read data from sensor\n");
    }
}

// HOMEKIT
void on_event(homekit_event_t event) {
    
    switch (event) {
        case HOMEKIT_EVENT_SERVER_INITIALIZED:
            printf("on_homekit_event: Server initialised\n");
            break;
        case HOMEKIT_EVENT_CLIENT_CONNECTED:
            printf("on_homekit_event: Client connected\n");
            break;
        case HOMEKIT_EVENT_CLIENT_VERIFIED:
            printf("on_homekit_event: Client verified\n");
            if (!paired ){
                paired = true;
            }
            break;
        case HOMEKIT_EVENT_CLIENT_DISCONNECTED:
            printf("on_homekit_event: Client disconnected\n");
            break;
        case HOMEKIT_EVENT_PAIRING_ADDED:
            printf("on_homekit_event: Pairing added\n");
            break;
        case HOMEKIT_EVENT_PAIRING_REMOVED:
            printf("on_homekit_event: Pairing removed\n");
            if (!homekit_is_paired())
            /* if we have no more pairtings then restart */
                printf("on_homekit_event: no more pairings so restart\n");
            sdk_system_restart();
            break;
        default:
            printf("on_homekit_event: Default event %d ", event);
    }
    
}

// GLOBAL CHARACTERISTICS
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, "Curla92");
homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Air Quality Sensor");
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, "NODE MCU");
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, NULL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, "2.1.1");
homekit_characteristic_t identify_function = HOMEKIT_CHARACTERISTIC_(IDENTIFY, identify);
homekit_characteristic_t service_name1 = HOMEKIT_CHARACTERISTIC_(NAME, "Air Quality Sensor");
homekit_characteristic_t service_name2 = HOMEKIT_CHARACTERISTIC_(NAME, "Temperature Sensor");
homekit_characteristic_t service_name3 = HOMEKIT_CHARACTERISTIC_(NAME, "Humidity Sensor");
homekit_characteristic_t setup_name = HOMEKIT_CHARACTERISTIC_(NAME, "Setup");

// ACCESSORY NAME
void create_accessory_name() {
    // Accessory Name
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    
    int name_len = snprintf(NULL, 0, "Air Quality Sensor-%02X%02X%02X", macaddr[3],
                            macaddr[4], macaddr[5]);
    
    char *name_value = malloc(name_len + 1);
    snprintf(name_value, name_len + 1, "Air Quality Sensor-%02X%02X%02X", macaddr[3],
             macaddr[4], macaddr[5]);
    name.value = HOMEKIT_STRING(name_value);
    
    // Accessory Serial
    char *serial_value = malloc(13);
    snprintf(serial_value, 13, "%02X%02X%02X%02X%02X%02X", macaddr[0], macaddr[1],
             macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    serial.value = HOMEKIT_STRING(serial_value);
}

homekit_server_config_t config;

//CREATE ACCESSORY
void create_accessory() {
    
    uint8_t service_count = 5;
    
    if (show_setup.value.bool_value) {
        service_count++;
    }
    
    homekit_accessory_t **accessories = calloc(2, sizeof(homekit_accessory_t *));
    
    homekit_accessory_t *accessory = accessories[0] = calloc(1, sizeof(homekit_accessory_t));
    accessory->id = 1;
    accessory->category = homekit_accessory_category_sensor;
    accessory->services = calloc(service_count, sizeof(homekit_service_t *));
    
    homekit_service_t *accessory_info = accessory->services[0] = calloc(1, sizeof(homekit_service_t));
    accessory_info->type = HOMEKIT_SERVICE_ACCESSORY_INFORMATION;
    accessory_info->characteristics = calloc(7, sizeof(homekit_characteristic_t *));
    
    accessory_info->characteristics[0] = &name;
    accessory_info->characteristics[1] = &manufacturer;
    accessory_info->characteristics[2] = &serial;
    accessory_info->characteristics[3] = &model;
    accessory_info->characteristics[4] = &revision;
    accessory_info->characteristics[5] = &identify_function;
    accessory_info->characteristics[6] = NULL;
    
    homekit_service_t *sensor1_function = accessory->services[1] = calloc(1, sizeof(homekit_service_t));
    sensor1_function->type = HOMEKIT_SERVICE_AIR_QUALITY_SENSOR;
    sensor1_function->primary = true;
    sensor1_function->characteristics = calloc(10, sizeof(homekit_characteristic_t *));
    
    sensor1_function->characteristics[0] = &service_name1;
    sensor1_function->characteristics[1] = &air_quality;
    sensor1_function->characteristics[2] = &carbon_monoxide_level;
    sensor1_function->characteristics[3] = &carbon_dioxide_level;
    sensor1_function->characteristics[4] = &ethanol_level;
    sensor1_function->characteristics[5] = &ammonium_level;
    sensor1_function->characteristics[6] = &toluene_level;
    sensor1_function->characteristics[7] = &acetone_level;
    sensor1_function->characteristics[8] = &show_setup;
    sensor1_function->characteristics[9] = NULL;
    
    homekit_service_t *sensor2_function = accessory->services[2] = calloc(1, sizeof(homekit_service_t));
    sensor2_function->type = HOMEKIT_SERVICE_TEMPERATURE_SENSOR;
    sensor2_function->primary = false;
    sensor2_function->characteristics = calloc(3, sizeof(homekit_characteristic_t *));
    
    sensor2_function->characteristics[0] = &service_name2;
    sensor2_function->characteristics[1] = &current_temperature;
    sensor2_function->characteristics[2] = NULL;
    
    homekit_service_t *sensor3_function = accessory->services[3] = calloc(1, sizeof(homekit_service_t));
    sensor3_function->type = HOMEKIT_SERVICE_HUMIDITY_SENSOR;
    sensor3_function->primary = false;
    sensor3_function->characteristics = calloc(3, sizeof(homekit_characteristic_t *));
    
    sensor3_function->characteristics[0] = &service_name3;
    sensor3_function->characteristics[1] = &current_humidity;
    sensor3_function->characteristics[2] = NULL;
    
    if (show_setup.value.bool_value) {
        homekit_service_t *sensor_setup = accessory->services[4] = calloc(1, sizeof(homekit_service_t));
        sensor_setup->type = HOMEKIT_SERVICE_CUSTOM_SETUP;
        sensor_setup->primary = false;
        sensor_setup->characteristics = calloc(9, sizeof(homekit_characteristic_t *));
        
        sensor_setup->characteristics[0] = &setup_name;
        sensor_setup->characteristics[1] = &device_type_name;
        sensor_setup->characteristics[2] = &hum_offset;
        sensor_setup->characteristics[3] = &temp_offset;
        sensor_setup->characteristics[4] = &ip_addr;
        sensor_setup->characteristics[5] = &wifi_reset;
        sensor_setup->characteristics[6] = &reboot_device;
        sensor_setup->characteristics[7] = &ota_firmware;
        sensor_setup->characteristics[8] = NULL;
    } else {
        accessory->services[4] = NULL;
    }
    
    config.accessories = accessories;
    config.password = "277-66-227";
    config.setupId="F74L",
    config.on_event = on_event;
    
    homekit_server_init(&config);
}

// WIFI
void on_wifi_event(wifi_config_event_t event) {
    if (event == WIFI_CONFIG_CONNECTED) {
        printf("CONNECTED TO >>> WIFI <<<\n");
        Wifi_Connected = true;
        led_code(led_gpio, WIFI_CONNECTED);
        
        create_accessory_name();
        create_accessory();
    } else if (event == WIFI_CONFIG_DISCONNECTED) {
        Wifi_Connected = false;
        printf("DISCONNECTED FROM >>> WIFI <<<\n");
    }
}

void hardware_init(){
    //GPIO LED INIT
    led_create(led_gpio, false);
    // DHT GPIO INIT
    gpio_set_pullup(TEMPERATURE_SENSOR_PIN, false, false);
    
    sdk_os_timer_setfn(&extra_func_timer, temperature_sensor, NULL);
    sdk_os_timer_arm(&extra_func_timer, 10000, 1);
    //AIR QUALITY INIT
    MQInit();
    
    air_quality_sensor();
}

void user_init(void) {
    
    uart_set_baud(0, 115200);
    
    settings_init();
    hardware_init();
    
    sdk_os_timer_setfn(&reset_toggle_timer, reset_toggle, NULL);
    sdk_os_timer_setfn(&change_settings_timer, save_settings, NULL);
    
    wifi_config_init2("Air Quality Sensor", NULL, on_wifi_event);
}
