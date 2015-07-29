#include <asf.h>
#include "rgb_led.h"

static void inline write_one(void);
static void inline write_zero(void);

extern const uint8_t gamma_correct[];

//These assembly routines bit bang the 
//WS2812 protocol. Timings must be precise
//but we don't care about speed so this is 
//fine. 
static void inline write_one(void)
{

  RGB_SET_HIGH;
  asm volatile //this routine waits 800 nano seconds
    (
     "mov r3, 32  \n\t" //
     "loop1: \n\t"
     "subs r3, #1 \n\t"
     "bne loop1 \n\t"
     : //empty output list
     : //empty input list
     : "r3", "cc" //clobber list
     );
  RGB_SET_LOW;
  asm volatile //this routine waits 450 nano seconds
    (
     "mov r3, 16  \n\t" //
     "loop2: \n\t"
     "subs r3, #1 \n\t"
     "bne loop2 \n\t"
     : //empty output list
     : //empty input list
     : "r3", "cc" //clobber list
     );
}

static void inline write_zero(void)
{

  RGB_SET_HIGH;
  asm volatile //this routine waits 400 nano seconds
    (
     "mov r3, 15  \n\t" //
     "loop3: \n\t"
     "subs r3, #1 \n\t"
     "bne loop3 \n\t"
     : //empty output list
     : //empty input list
     : "r3", "cc" //clobber list
     );
  RGB_SET_LOW;
  asm volatile //this routine waits 850 nano seconds
    (
     "mov r3, 26  \n\t" //
     "loop4: \n\t"
     "subs r3, #1 \n\t"
     "bne loop4 \n\t"
     "nop \n\t"
     "nop \n\t"
     : //empty output list
     : //empty input list
     : "r3", "cc" //clobber list
     );
}

//End bit bang sequence
////////////////////////

void rgb_led_init(void){
  gpio_configure_pin(RGB_LED_PIN, PIO_OUTPUT_0);
  //set up blink interrupt with PWM
  //PWM module configured by monitor init
  pwm_channel_t channel = {
    .channel = LED_PWM_CHANNEL,
    .ul_duty = 0,
    .ul_period = 1000, //blink every second
    .ul_prescaler = PWM_CMR_CPRE_CLKA,
    .polarity = PWM_HIGH,
  };
  pwm_channel_init(PWM, &channel);
  //                enable interrupts on overflow
  pwm_channel_disable_interrupt(PWM,LED_PWM_CHANNEL,LED_PWM_CHANNEL);
  pwm_channel_disable(PWM,LED_PWM_CHANNEL);

  rgb_led_set(LED_OFF,0); 
};

void rgb_led_set(uint8_t r, uint8_t g, uint8_t b, uint32_t blink){
  led_color.red = r;
  led_color.green = g;
  led_color.blue = b;
  //set the LED
  rgb_led_write(r,g,b);
  //only allow blinking at 10Hz - every ten seconds
  if(blink!=0 && (blink<LED_MIN_BLINK_RATE || blink > LED_MAX_BLINK_RATE)){
    printf("invalid blink rate, ignoring");
    blink = 0;
  }
  //disable the blink timer
  pwm_channel_disable(PWM,LED_PWM_CHANNEL);
  pwm_channel_disable_interrupt(PWM,LED_PWM_CHANNEL,LED_PWM_CHANNEL);
  //set blink timer for the specified rate (ms)
  if(blink!=0){ 
    pwm_channel_t channel = {
      .channel = LED_PWM_CHANNEL,
      .ul_duty = 0,
      .ul_period = blink,
      .ul_prescaler = PWM_CMR_CPRE_CLKA,
      .polarity = PWM_HIGH,
    };
    pwm_channel_init(PWM, &channel);
    pwm_channel_enable_interrupt(PWM,LED_PWM_CHANNEL,LED_PWM_CHANNEL);
    pwm_channel_enable(PWM,LED_PWM_CHANNEL);
  }
}

void rgb_led_write(uint8_t r_orig, uint8_t g_orig, uint8_t b_orig){
  uint8_t r,b,g;
  //gamma correction
  r = gamma_correct[r_orig];
  g = gamma_correct[g_orig];
  b = gamma_correct[b_orig];
  //make a 24 bit array so we can quickly step through it
  uint8_t bits [24];
  int i;
  //green bits
  for(i=0;i<8;i++){
    if((b&(1<<i))==(1<<i))
      bits[i]=1;
    else
      bits[i]=0;
  }
  //blue bits
  for(i=0;i<8;i++){
    if((r&(1<<i))==(1<<i))
      bits[i+8]=1;
    else
      bits[i+8]=0;
  }
  //red bits
  for(i=0;i<8;i++){
    if((g&(1<<i))==(1<<i))
      bits[i+16]=1;
    else
      bits[i+16]=0;
  }
  delay_us(100);
  //now write everything
  for(i=23;i>=0;i--){
    if(bits[i])
      write_one();
    else
      write_zero();
  }
}

//Gamma correction from 
//   https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
const uint8_t gamma_correct[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
