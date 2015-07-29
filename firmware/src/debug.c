#include <asf.h>
#include "debug.h"


#ifdef __DEBUG__


// When a Black Magic Probe is attached, this function prints
// a string to the standard output of GDB (on the host).
void print(const char *str) {
  int data[3] = {0x02, (int) str, 0};
  while(*(str++)) ++(data[2]);  // count length of string
  __asm__ volatile ("mov r0, $0x05; mov r1, %0; bkpt $0xAB" : : "r" (data) : "r0", "r1");
}

void dbgputc(void *stream, char data){
  if(data=='\r')
    data='!';
  print(&data);
}
#else
  void print(const char *str){
    //noop
  }
void dbgputc(void *stream, char data){
  //noop
}
#endif

