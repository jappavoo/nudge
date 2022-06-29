#ifndef __MISC_H__
#define __MISC_H__

#ifdef VERBOSE
#define VPRINTLN(...) Serial.println(__VA_ARGS__)
#define VPRINT(...) Serial.print(__VA_ARGS__)
#else
#define VPRINTLN(...) 
#define VPRINT(...) 
#endif

void
misc_setup() {
#ifdef VERBOSE
 Serial.begin(115200);
#endif
}

#endif
