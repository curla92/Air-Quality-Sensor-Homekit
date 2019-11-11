#define MQ135PIN 0
#define LED_GPIO 5

#define RL_VALUE 10
#define R0_CLEAN_AIR_FACTOR 3.59

#define CALIBRATION_SAMPLE_TIMES 50
#define CALIBRATION_SAMPLE_INTERVAL 5

#define READ_SAMPLE_INTERVAL 50
#define READ_SAMPLE_TIMES 5

#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define	CORD 0.0018
#define CORE 0.003333333
#define CORF -0.001923077
#define CORG 1.130128205

/// Parameters Equation for CO
#define scaleFactorCO 662.9382
#define exponentCO 4.0241
/// Parameters Equation for CO2
#define scaleFactorCO2 116.6020682
#define exponentCO2 2.769034857
/// Paremeters Equation for Ethanol
#define scaleFactorEthanol 75.3103
#define exponentEthanol 3.1459
/// Parameters Equation for NH4
#define scaleFactorNH4 102.694
#define exponentNH4 2.48818
/// Parameters Equation for Toluene
#define scaleFactorToluene 43.7748
#define exponentToluene 3.42936
/// Parameters Equation for Acetone
#define scaleFactorAcetone 33.1197
#define exponentAcetone 3.36587

/// Atmospheric CO Level for calibration purposes
#define atmCO 1
/// Atmospheric CO2 level for calibration purposes
#define atmCO2 408.55
/// Atmospheric Ethanol Level for calibration purposes
#define atmEthanol 22.5
/// Atmospheric NH4 level for calibration purposes
#define atmNH4 15
/// Atmospheric Toluene level for calibration purposes
#define atmToluene 0.00739
/// Atmospheric Acetone level for calibration purposes
#define atmAcetone 16

extern float CO_val;
extern float CO2_val;
extern float Ethanol_val;
extern float NH4_val;
extern float Toluene_val;
extern float Acetone_val;

extern uint8_t air_quality_val;

void MQInit();
void MQGetReadings(float temparature, float humidity);

float MQResistanceCalculation(int raw_adc);
float MQReadResistanceCalculation(int mq_pin);

float MQCalibrationCO(int mq_pin, float ppm);
float MQCalibrationCO2(int mq_pin, float ppm);
float MQCalibrationEthanol(int mq_pin, float ppm);
float MQCalibrationNH4(int mq_pin, float ppm);
float MQCalibrationToluene(int mq_pin, float ppm);
float MQCalibrationAcetone(int mq_pin, float ppm);

int getCalibratedCO(float rs_r0CO_ratio);
int getCalibratedCO2(float rs_r0CO2_ratio);
int getCalibratedEthanol(float rs_r0Ethanol_ratio);
int getCalibratedNH4(float rs_r0NH4_ratio);
int getCalibratedToluene(float rs_r0Toluene_ratio);
int getCalibratedAcetone(float rs_r0Acetone_ratio);