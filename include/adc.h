#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>

#define NUM_ADC_CHANNELS 4

static const float ADC_DEFAULT_OFFSETS[NUM_ADC_CHANNELS] = {
  0.035f,   // Offset para GPIO32 (Canal 0)
  0.035f,   // Offset para GPIO33 (Canal 1)
  0.035f,   // Offset para GPIO34 (Canal 2)
  0.035f    // Offset para GPIO35 (Canal 3)
};

typedef struct {
  float voltage;
  uint32_t raw_value;
  const char *name;
} adcReadingStruct;

void adcReaderInit(void);
void readAllAdcChannels(adcReadingStruct results[NUM_ADC_CHANNELS]);
void adc_set_channel_offset(uint8_t channel, float offset);

#endif 

