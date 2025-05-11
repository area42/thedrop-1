#include <Arduino.h>
#include <DacESP32.h>
#include <ezButton.h>

// put function declarations here:
float revsPerMinute();
void RPMLED(float);

// RPM switch
const int input_RPM_pin = 14;
ezButton rpm_button(input_RPM_pin);

const int timeout = 2500;
unsigned long last_switch_time;
unsigned long last_isr;
unsigned long switch_diff;

// RPM light
const int output_RPM_led = 15;
unsigned long last_led_time;

// DAC
DacESP32 dac1(GPIO_NUM_25),
         dac2(GPIO_NUM_26);

float RPM;

// RPM interupt
void IRAM_ATTR isr() {

}

void setup() {
  // put your setup code here, to run once:
	Serial.begin(115200);
	pinMode(input_RPM_pin, INPUT_PULLUP);
  pinMode(output_RPM_led, OUTPUT);

	// attachInterrupt(input_RPM_pin, isr, RISING);
  rpm_button.setDebounceTime();
  last_switch_time = 0;
  last_isr = 0;
  switch_diff = 0;

  last_led_time = millis();
  digitalWrite(output_RPM_led, HIGH);
  RPM = 0;

  dac1.outputCW(200);
  dac2.outputCW(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  rpm_button.loop(); // MUST call the loop() function first

  if (rpm_button.isPressed())
  {
    // Get timings
    unsigned long current_time = millis();
    switch_diff = current_time - last_switch_time;
    last_switch_time = current_time;

    // Trigger LED
    digitalWrite(output_RPM_led, HIGH);
    last_led_time = current_time;
  }

  if (last_switch_time != 0)
  {
    RPM = revsPerMinute();
    RPMLED(RPM);

    uint32_t freq = pow(RPM, 2) / 5;
    if (freq > 10 || freq < 20000) {
      Serial.printf("%i \n", freq);
      dac1.outputCW(freq);
      dac2.outputCW(freq);
    }
  }

  // sleep(0.3);
}

float revsPerMinute() {
  // Check to see if stopped
  if (millis() - last_switch_time > timeout) {
    switch_diff = 0;
    last_switch_time = 0;
    return 0;
  } else {
    return 60000.0 / (int)switch_diff;
  }
}

void RPMLED(float RPM) {
  if (RPM == 0) return;

  // If it's been long enough, turn off the LED
  if (millis() - last_led_time > switch_diff / 2.0) {
    digitalWrite(output_RPM_led, LOW);
  }
}