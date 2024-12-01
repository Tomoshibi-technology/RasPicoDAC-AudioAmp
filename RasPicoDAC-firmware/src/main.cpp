#include <Arduino.h>
#include <I2S.h>

#define PIN_I2S_BCLK 20
#define PIN_I2S_LRCLK (PIN_I2S_BCLK + 1)
#define PIN_I2S_DOUT 22
#define PIN_I2S_MUTE 19

#define BUFFER_SIZE 256

I2S i2s(OUTPUT);
const int32_t sampleRate = 48000;
int16_t buffer[BUFFER_SIZE];
uint32_t phase       = 0;
uint32_t phase_delta = 0;

int16_t triangle(uint32_t phase) {
  phase += (1 << 30);
  return ((phase < (uint32_t)(1 << 31)) ? (phase >> 16) : ((1 << 16) - (phase >> 16) - 1)) - 16383;
}

void generate_triangle(int16_t *buffer, size_t size, uint32_t *phase, uint32_t phase_delta) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = triangle(*phase);
    *phase += phase_delta;
  }
}

void setup() {
  pinMode(PIN_I2S_MUTE, OUTPUT);

  i2s.setBCLK(PIN_I2S_BCLK);
  i2s.setDATA(PIN_I2S_DOUT);
  i2s.setBitsPerSample(16);
  i2s.begin(sampleRate);

  phase_delta = 440.0 * (float)(1ULL << 32) / sampleRate;

  digitalWrite(PIN_I2S_MUTE, HIGH);
}

void loop() {
  static size_t buffer_index = 0;

  if (buffer_index == BUFFER_SIZE) {
    generate_triangle(buffer, BUFFER_SIZE, &phase, phase_delta);

    buffer_index = 0;
  }

  while (buffer_index < BUFFER_SIZE) {
    i2s.write(buffer[buffer_index]);  // L
    i2s.write(buffer[buffer_index]);  // R
    buffer_index++;
  }
}