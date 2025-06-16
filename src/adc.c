#include "config.h"
#include "adc.h"
#include "esp_adc_cal.h"

// Configuración privada
#define DEFAULT_VREF        0        // Valor de referencia en mV
#define NO_OF_SAMPLES       64          // Número de muestras para promediar
#define ADC_ATTEN           ADC_ATTEN_DB_11 // Atenuación para 0-3.6V

// Offsets 
static float adc_current_offsets[NUM_ADC_CHANNELS];

// Configuración de canales
typedef struct {
  gpio_num_t gpio_num;
  adc1_channel_t adc_channel;
  const char *name;
} adc_channel_config_t;

static const adc_channel_config_t adc_channels[NUM_ADC_CHANNELS] = {
  {GPIO_NUM_32, ADC1_CHANNEL_4, "GPIO32"},
  {GPIO_NUM_33, ADC1_CHANNEL_5, "GPIO33"},
  {GPIO_NUM_34, ADC1_CHANNEL_6, "GPIO34"},
  {GPIO_NUM_35, ADC1_CHANNEL_7, "GPIO35"}
};

static esp_adc_cal_characteristics_t *adc_chars = NULL;

// Función privada para lectura con offset
static float read_single_channel(adc1_channel_t channel, uint32_t *raw_value) {
  uint32_t adc_reading = 0;

  // Promedio de muestras
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    int raw = adc1_get_raw(channel);
    if (raw == -1) {
      // Error reading ADC, you might want to handle this case
      raw = 0;
    }
    adc_reading += raw;
  }

  *raw_value = adc_reading / NO_OF_SAMPLES;
  
  // Conversión a voltaje y aplicación de offset
  float voltage = (float)esp_adc_cal_raw_to_voltage(*raw_value, adc_chars) / 1000.0f;
  uint8_t channel_idx = channel - ADC1_CHANNEL_4; // Convierte a índice 0-3

  return (voltage > adc_current_offsets[channel_idx]) 
            ? (voltage - adc_current_offsets[channel_idx]) 
            : 0.0f;
}

void adcReaderInit(void) {
  // Inicializar offsets con los valores del .h
  for (int i = 0; i < NUM_ADC_CHANNELS; i++) {
    adc_current_offsets[i] = ADC_DEFAULT_OFFSETS[i];
  }

  // Configuración hardware
  adc1_config_width(ADC_WIDTH_BIT_12);
    
  for (int i = 0; i < NUM_ADC_CHANNELS; i++) {
    // Configurar GPIO
    gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << adc_channels[i].gpio_num),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configurar canal ADC con manejo de errores
    esp_err_t ret = adc1_config_channel_atten(adc_channels[i].adc_channel, ADC_ATTEN);
    if (ret != ESP_OK) {
      // Manejar error de configuración ADC
      printf("Error configuring ADC channel %d: %s\n", i, esp_err_to_name(ret));
    }
  }

  // Calibración
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  if (adc_chars == NULL) {
    printf("Error allocating memory for ADC characteristics\n");
    return;
  }

  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, 
                                                        ADC_ATTEN, 
                                                        ADC_WIDTH_BIT_12, 
                                                        DEFAULT_VREF, 
                                                        adc_chars);
  
  // Verificar tipo de calibración (opcional)
  switch (val_type) {
    case ESP_ADC_CAL_VAL_EFUSE_TP:
      printf("ADC calibration using Two Point values\n");
      break;
    case ESP_ADC_CAL_VAL_EFUSE_VREF:
      printf("ADC calibration using eFuse Vref\n");
      break;
    case ESP_ADC_CAL_VAL_DEFAULT_VREF:
      printf("ADC calibration using default Vref\n");
      break;
    default:
      printf("ADC calibration failed\n");
      break;
  }
}

void readAllAdcChannels(adcReadingStruct results[NUM_ADC_CHANNELS]) {
  for (int i = 0; i < NUM_ADC_CHANNELS; i++) {
    results[i].name = adc_channels[i].name;
    results[i].voltage = read_single_channel(
      adc_channels[i].adc_channel, 
      &results[i].raw_value
    );
  }
}

void adc_set_channel_offset(uint8_t channel, float offset) {
  if (channel < NUM_ADC_CHANNELS) {
    adc_current_offsets[channel] = offset;
  }
}