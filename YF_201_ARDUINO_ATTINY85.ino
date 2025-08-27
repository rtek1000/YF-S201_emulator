// YF-S201 emulator
//
// ATtinyCore
// ATtiny85
// 16MHz PLL

#include <avr/io.h>
#include <avr/interrupt.h>

// ATtiny85 pinout
// VCC 5V:                  Pin 8
// GND:                     Pin 4
// RESET:                   Pin 1

#define POT_DIF_pin A3   // Pin 2
#define POT_SPEED_pin A2 // Pin 3
#define OUT_COLD_pin 0   // Pin 5
#define OUT_HOT_pin 1    // Pin 6
#define LED_pin 2        // Pin 7

#define FLOW_MAX_HZ 65  // Hertz (8l/min: 65.5Hz)

#define TIMER1_RECALL 131  // 1ms (1000Hz)

unsigned int dif_adc = 0;
unsigned int speed_adc = 0;

int freq_COLD = 0;
int freq_HOT = 0;

volatile bool LED_state = false;

uint32_t LED_millis = 0;

int timer1_cold_cnt = 0;
volatile int timer1_cold_hz = 0;

int timer1_hot_cnt = 0;
volatile int timer1_hot_hz = 0;

volatile bool OUT_COLD_enable = false;
volatile bool OUT_HOT_enable = false;

bool enable_flag = false;

void initTimerCounter1(void);

void setup() {
  // put your setup code here, to run once:

  pinMode(POT_DIF_pin, INPUT);
  pinMode(POT_SPEED_pin, INPUT);

  pinMode(OUT_COLD_pin, OUTPUT);
  pinMode(OUT_HOT_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);

  initTimerCounter1();
}

void loop() {
  // put your main code here, to run repeatedly:

  if ((millis() - LED_millis) >= 2000) {
    LED_millis = millis();

    LED_state = 1; // !LED_state;
  }

  dif_adc = 0;
  speed_adc = 0;

  for (byte i = 0; i < 5; i++) {
    dif_adc += analogRead(POT_DIF_pin);
    speed_adc += analogRead(POT_SPEED_pin);
  }

  dif_adc /= 5;
  speed_adc /= 5;

  int dif_cold = 1023 - dif_adc;

  timer1_cold_hz = calc_flow(FLOW_MAX_HZ, speed_adc, dif_cold);

  OUT_COLD_enable = enable_flag;

  int dif_hot = dif_adc;

  timer1_hot_hz = calc_flow(FLOW_MAX_HZ, speed_adc, dif_hot);

  OUT_HOT_enable = enable_flag;
}

int calc_flow(int flow_max, int speed, int dif) {
  float calc1 = flow_max;  // load max freq for cold flow
  calc1 /= 1024;           // convert max freq to adc scale
  calc1 *= speed;          // convert pot position to freq

  int freq = calc1;  // set cold freq

  calc1 = dif;  // load pot position

  calc1 /= 1024;  // convert pot position to flow proportion

  calc1 *= freq;  // convert flow base to flow

  freq = calc1;  // set final freq_COLD value

  if (freq > 0) {
    calc1 = 1000;  // load 1s base

    calc1 /= freq;  // convert freq to ms

    enable_flag = true;

  } else {
    enable_flag = false;
  }

  return calc1;  // return cold freq proportional
}

void initTimerCounter1(void) {
  GTCCR = _BV(PSR1);    // Prescaler Reset Timer/Counter1
  TIMSK |= _BV(TOIE1);  // Add interrupt on Timer/Counter1 Overflow

  TCCR1 = _BV(CS13);  // Prescale: PCK/64 [16Mhz]
  // TCCR1 = _BV(CS12) | _BV(CS11) | _BV(CS10);  // Prescale: PCK/32 [8MHz]
  TCNT1 = TIMER1_RECALL;                      // Set the timer

  sei();
}

// 1ms
ISR(TIM1_OVF_vect) {
  TCNT1 = TIMER1_RECALL;  // Set the timer

  digitalWrite(LED_pin, LOW);  //turn the LED on
  delayMicroseconds(100);
  digitalWrite(LED_pin, LED_state);  //turn the LED off

  timer1_cold_cnt++;

  if (timer1_cold_cnt >= timer1_cold_hz) {
    timer1_cold_cnt = 0;

    digitalWrite(OUT_COLD_pin, OUT_COLD_enable);
    delayMicroseconds(100);
    digitalWrite(OUT_COLD_pin, LOW);
  }

  timer1_hot_cnt++;

  if (timer1_hot_cnt >= timer1_hot_hz) {
    timer1_hot_cnt = 0;

    digitalWrite(OUT_HOT_pin, OUT_HOT_enable);
    delayMicroseconds(100);
    digitalWrite(OUT_HOT_pin, LOW);
  }
}
