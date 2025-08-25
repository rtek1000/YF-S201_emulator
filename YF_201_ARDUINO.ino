#define USE_TIMER_1 true // It is necessary to declare before #include "TimerInterrupt.h"

#include "TimerInterrupt.h" // https://github.com/rtek1000/TimerInterrupt

#define TIMER1_INTERVAL_US 1000

#define POT_DIF_pin A0
#define POT_SPEED_pin A1

#define OUT_COLD_pin 4
#define OUT_HOT_pin 5

#define LED_pin 13

// #define TIMER1_OUT_pin 10 // debug only

#define FLOW_MAX_HZ 65  // Hertz (8l/min: 65.5Hz)

volatile int dif_adc = 0;
int speed_adc = 0;

int freq_COLD = 0;
int freq_HOT = 0;

bool LED_state = false;

uint32_t LED_millis = 0;

int timer1_cold_cnt = 0;
volatile int timer1_cold_hz = 0;

int timer1_hot_cnt = 0;
volatile int timer1_hot_hz = 0;

volatile bool OUT_COLD_enable = false;
volatile bool OUT_HOT_enable = false;

bool enable_flag = false;

void TimerHandler1(void);

void setup() {
  // put your setup code here, to run once:

  pinMode(POT_DIF_pin, INPUT);
  pinMode(POT_SPEED_pin, INPUT);

  pinMode(OUT_COLD_pin, OUTPUT);
  pinMode(OUT_HOT_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);
  // pinMode(TIMER1_OUT_pin, OUTPUT); // debug only

  ITimer1.init();
  ITimer1.attachInterruptInterval_us(TIMER1_INTERVAL_US, TimerHandler1); // us
}

void loop() {
  // put your main code here, to run repeatedly:

  if ((millis() - LED_millis) > 500) {
    LED_millis = millis();

    LED_state = !LED_state;

    digitalWrite(LED_pin, LED_state);
  }

  dif_adc = analogRead(POT_DIF_pin);
  speed_adc = analogRead(POT_SPEED_pin);

  int dif_cold = 1023 - dif_adc;

  timer1_cold_hz = calc_flow(FLOW_MAX_HZ, speed_adc, dif_cold);

  OUT_COLD_enable = enable_flag;

  int dif_hot = dif_adc;

  timer1_hot_hz = calc_flow(FLOW_MAX_HZ, speed_adc, dif_hot);

  OUT_HOT_enable = enable_flag;
}

int calc_flow(int flow_max, int speed, int dif) {
  float calc1 = flow_max;  // load max freq for cold flow
  calc1 /= 1024;              // convert max freq to adc scale
  calc1 *= speed;         // convert pot position to freq

  int freq = calc1;  // set cold freq

  calc1 = dif; // load pot position

  calc1 /= 1024; // convert pot position to flow proportion

  calc1 *= freq; // convert flow base to flow

  freq = calc1; // set final freq_COLD value

  if (freq > 0) {
    calc1 = 1000;  // load 1s base

    calc1 /= freq;  // convert freq to ms

    enable_flag = true;

  } else {
    enable_flag = false;
  }

  return calc1;  // return cold freq proportional
}

void TimerHandler1(void) {
  // digitalWrite(TIMER1_OUT_pin, HIGH); // debug only
  // delayMicroseconds(5);
  // digitalWrite(TIMER1_OUT_pin, LOW);

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
