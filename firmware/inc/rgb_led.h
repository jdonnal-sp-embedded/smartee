#ifndef __RGB_LED_H__
#define __RGB_LED_H__

#define RGB_LED_PIN    (PIO_PA17_IDX)
#define RGB_SET_HIGH   (PIOA->PIO_SODR = 1<<17)
#define RGB_SET_LOW    (PIOA->PIO_CODR = 1<<17)

#define LED_MIN_BLINK_RATE 10    //10 Hz
#define LED_MAX_BLINK_RATE 10000 //every 10 seconds

#define LED_PWM_CHANNEL 1

#define LED_BLUE     0,0,255
#define LED_LT_BLUE  0,30,200
#define LED_GREEN    0,255,0

#define LED_DK_GREEN 242,222,68
#define LED_LT_GREEN 0,125,30

#define LED_RED      255,0,0
#define LED_ORANGE   255,255,0
#define LED_PURPLE   255,0,255
#define LED_OFF      0,0,0
#define LED_GRAY     128,128,128

struct color{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} led_color;

void rgb_led_init(void);
//used by other modules to control the LED
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b, uint32_t blink);
//used internally and by the blink interrupt (PWM) in monitor.c
void rgb_led_write(uint8_t r, uint8_t g, uint8_t b);
#endif
