#include <stdio.h>
#include <math.h>
#include <espressif/esp_common.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include "esp8266_mq135.h"

#include <led_codes.h>

float r0CO = 0;
float r0CO2 = 0;
float r0Ethanol = 0;
float r0NH4 = 0;
float r0Toluene = 0;
float r0Acetone = 0;

float Rs = 0;

float CO2_val = 0;
float CO_val = 0;
float Ethanol_val = 0;
float NH4_val = 0;
float Toluene_val = 0;
float Acetone_val = 0;

uint8_t air_quality_val = 0;

void MQInit() {
    r0CO = MQCalibrationCO(MQ135PIN, atmCO);
    r0CO2 = MQCalibrationCO2(MQ135PIN, atmCO2);
    r0Ethanol = MQCalibrationEthanol(MQ135PIN, atmEthanol);
    r0NH4 = MQCalibrationNH4(MQ135PIN, atmNH4);
    r0Toluene = MQCalibrationToluene(MQ135PIN, atmToluene);
    r0Acetone = MQCalibrationAcetone(MQ135PIN, atmAcetone);
    
    printf ("Calibrated value for R0 CO is %f\n", r0CO);
    printf ("Calibrated value for R0 CO2 is %f\n", r0CO2);
    printf ("Calibrated value for R0 Ethanol is %f\n", r0Ethanol);
    printf ("Calibrated value for R0 NH4 is %f\n", r0NH4);
    printf ("Calibrated value for R0 Toluene is %f\n", r0Toluene);
    printf ("Calibrated value for R0 Acetone is %f\n", r0Acetone);
}

float get_correction_factor(float temperature, float humidity) {
    if (temperature < 20){
        return CORA * temperature * temperature - CORB * temperature + CORC - (humidity - 33.0) * CORD;
    }else{
        return CORE * temperature + CORF * humidity + CORG;
    }
}

void MQGetReadings(float temperature, float humidity) {
    
    float rs_r0CO_ratio=0;
    float rs_r0CO2_ratio=0;
    float rs_r0Ethanol_ratio=0;
    float rs_r0NH4_ratio=0;
    float rs_r0Toluene_ratio=0;
    float rs_r0Acetone_ratio=0;
    
    float correction_factor=0;
    
    Rs = MQReadResistanceCalculation(MQ135PIN);
    correction_factor = get_correction_factor(temperature, humidity);
    Rs = Rs / correction_factor;
    
    rs_r0CO_ratio = Rs/r0CO;
    rs_r0CO2_ratio = Rs/r0CO2;
    rs_r0Ethanol_ratio = Rs/r0Ethanol;
    rs_r0NH4_ratio = Rs/r0NH4;
    rs_r0Toluene_ratio = Rs/r0Toluene;
    rs_r0Acetone_ratio = Rs/r0Acetone;
    
    //INITIAL PPM + CALCULATED PPM
    CO_val = atmCO + getCalibratedCO(rs_r0CO_ratio);
    CO2_val = atmCO2 + getCalibratedCO(rs_r0CO2_ratio);
    Ethanol_val = atmEthanol + getCalibratedEthanol(rs_r0Ethanol_ratio);
    NH4_val = atmNH4 + getCalibratedNH4(rs_r0NH4_ratio);
    Toluene_val = atmToluene + getCalibratedToluene(rs_r0Toluene_ratio);
    Acetone_val = atmAcetone + getCalibratedAcetone(rs_r0Acetone_ratio);
    
    air_quality_val = 0;
    
    if ( (CO_val >=15.4 && air_quality_val < 5) || (CO2_val >=2500 && air_quality_val < 5) ){
        air_quality_val = 5;
        led_code(LED_GPIO, FUNCTION_F);
    } else if ( (CO_val >= 12.5 && air_quality_val < 4) || (CO2_val >= 1000 && air_quality_val < 4) ){
        air_quality_val = 4;
        led_code(LED_GPIO, FUNCTION_D);
    } else if ( (CO_val >= 9.5 && air_quality_val < 3) || (CO2_val >= 700 && air_quality_val < 3) ){
        air_quality_val = 3;
        led_code(LED_GPIO, FUNCTION_B);
    } else if ( (CO_val >= 4.5 && air_quality_val < 2) || (CO2_val >= 450 && air_quality_val < 2) ){
        air_quality_val = 2;
    } else if (air_quality_val == 0){
        air_quality_val = 1;
    }
    
    printf("Correction factor %f, air_quality_val %i, Rs %f, CO2 %f, CO %f, Ethanol %f, NH4 %f, Toluene %f, Ammonium %f\n", correction_factor, air_quality_val, Rs, CO2_val, CO_val, Ethanol_val, NH4_val, Toluene_val, Acetone_val);
}

float MQResistanceCalculation(int raw_adc) {
    return ( ((float)RL_VALUE*(1024-raw_adc)/raw_adc));
}

float MQReadResistanceCalculation(int mq_pin) {
    int i;
    float Rs=0;
    
    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        Rs += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(READ_SAMPLE_INTERVAL);
    }
    Rs = Rs/READ_SAMPLE_TIMES;
    
    return Rs;
}

float MQCalibrationCO(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("CO Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorCO), (1./exponentCO));
}

float MQCalibrationCO2(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("CO2 Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorCO2), (1./exponentCO2));
}

float MQCalibrationEthanol(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("Ethanol Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorEthanol), (1./exponentEthanol));
}

float MQCalibrationNH4(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("NH4 Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorNH4), (1./exponentNH4));
}

float MQCalibrationToluene(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("Toluene Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorToluene), (1./exponentToluene));
}

float MQCalibrationAcetone(int mq_pin, float ppm) {
    int i;
    float val=0;
    
    for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {
        val += MQResistanceCalculation(sdk_system_adc_read());
        vTaskDelay(CALIBRATION_SAMPLE_INTERVAL);
    }
    val = val/CALIBRATION_SAMPLE_TIMES;
    val = val/R0_CLEAN_AIR_FACTOR;
    //printf("Acetone Calibrated Value is %f\n", val);
    
    return (long)val * powf((ppm/scaleFactorAcetone), (1./exponentAcetone));
}

int getCalibratedCO(float rs_r0CO_ratio) {
    return scaleFactorCO * powf(rs_r0CO_ratio, -exponentCO);
}

int getCalibratedCO2(float rs_r0CO2_ratio) {
    return scaleFactorCO2 * powf(rs_r0CO2_ratio, -exponentCO2);
}

int getCalibratedEthanol(float rs_r0Ethanol_ratio) {
    return scaleFactorEthanol * powf(rs_r0Ethanol_ratio, -exponentEthanol);
}

int getCalibratedNH4(float rs_r0NH4_ratio) {
    return scaleFactorNH4 * powf(rs_r0NH4_ratio, -exponentNH4);
}

int getCalibratedToluene(float rs_r0Toluene_ratio) {
    return scaleFactorToluene * powf(rs_r0Toluene_ratio, -exponentToluene);
}

int getCalibratedAcetone(float rs_r0Acetone_ratio) {
    return scaleFactorAcetone * powf(rs_r0Acetone_ratio, -exponentAcetone);
}
