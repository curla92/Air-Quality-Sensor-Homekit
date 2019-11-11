#ifndef __HOMEKIT_CUSTOM_CHARACTERISTICS__
#define __HOMEKIT_CUSTOM_CHARACTERISTICS__

#define HOMEKIT_CUSTOM_UUID(value) (value "-03a1-4971-92bf-af2b7d833922")

#define HOMEKIT_SERVICE_CUSTOM_SETUP HOMEKIT_CUSTOM_UUID("F00000FF")

#define HOMEKIT_CHARACTERISTIC_CUSTOM_SHOW_SETUP HOMEKIT_CUSTOM_UUID("F0000106")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_SHOW_SETUP(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_SHOW_SETUP, \
.description = "Show Setup", \
.format = homekit_format_bool, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.value = HOMEKIT_BOOL_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_DEVICE_TYPE_NAME HOMEKIT_CUSTOM_UUID("F0000107")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_DEVICE_TYPE_NAME(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_DEVICE_TYPE_NAME, \
.description = "1) Actual Dev Type", \
.format = homekit_format_string, \
.permissions = homekit_permissions_paired_read, \
.value = HOMEKIT_STRING_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_ETHANOL_LEVEL HOMEKIT_CUSTOM_UUID("F0000108")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_ETHANOL_LEVEL(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_ETHANOL_LEVEL, \
.description = "Ethanol Level", \
.format = homekit_format_float, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_notify, \
.min_value = (float[]) {10}, \
.max_value = (float[]) {300}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL HOMEKIT_CUSTOM_UUID("F0000109")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_AMMONIUM_LEVEL, \
.description = "Ammonium Level", \
.format = homekit_format_float, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_notify, \
.min_value = (float[]) {10}, \
.max_value = (float[]) {300}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_TOLUENE_LEVEL HOMEKIT_CUSTOM_UUID("F0000110")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_TOLUENE_LEVEL(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_TOLUENE_LEVEL, \
.description = "Toluene Level", \
.format = homekit_format_float, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_notify, \
.min_value = (float[]) {0}, \
.max_value = (float[]) {100}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_ACETONE_LEVEL HOMEKIT_CUSTOM_UUID("F0000111")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_ACETONE_LEVEL(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_ACETONE_LEVEL, \
.description = "Acetone Level", \
.format = homekit_format_float, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_notify, \
.min_value = (float[]) {0}, \
.max_value = (float[]) {100}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_TEMPERATURE_OFFSET HOMEKIT_CUSTOM_UUID("F0000112")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_TEMPERATURE_OFFSET(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_TEMPERATURE_OFFSET, \
.description = "2) Offset TEMP", \
.format = homekit_format_float, \
.unit = homekit_unit_celsius, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.min_value = (float[]) {-15}, \
.max_value = (float[]) {15}, \
.min_step = (float[]) {0.1}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_HUMIDITY_OFFSET HOMEKIT_CUSTOM_UUID("F0000113")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_HUMIDITY_OFFSET(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_HUMIDITY_OFFSET, \
.description = "3) Offset HUM", \
.format = homekit_format_float, \
.unit = homekit_unit_percentage, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.min_value = (float[]) {-15}, \
.max_value = (float[]) {15}, \
.min_step = (float[]) {1}, \
.value = HOMEKIT_FLOAT_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR HOMEKIT_CUSTOM_UUID("F0000114")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_IP_ADDR(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR, \
.description = "4) Wifi IP Addr", \
.format = homekit_format_string, \
.permissions = homekit_permissions_paired_read, \
.value = HOMEKIT_STRING_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RESET HOMEKIT_CUSTOM_UUID("F0000115")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_WIFI_RESET(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RESET, \
.description = "5) Wifi Reset", \
.format = homekit_format_bool, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.value = HOMEKIT_BOOL_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE HOMEKIT_CUSTOM_UUID("F0000116")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE, \
.description = "6) Reboot", \
.format = homekit_format_bool, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.value = HOMEKIT_BOOL_(_value), \
##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_UPDATE HOMEKIT_CUSTOM_UUID("F0000117")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_OTA_UPDATE(_value, ...) \
.type = HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_UPDATE, \
.description = "7) Firmware Update", \
.format = homekit_format_bool, \
.permissions = homekit_permissions_paired_read \
| homekit_permissions_paired_write \
| homekit_permissions_notify, \
.value = HOMEKIT_BOOL_(_value), \
##__VA_ARGS__

#endif
