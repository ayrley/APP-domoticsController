#ifndef __CONTROL_H_
#define __CONTROL_H_

#define DEBUG

int main(void);
int loadSettings();

#ifdef DEBUG
	#define DIR_ETC 		""
	#define DIR_SHARED		""
#else
	#define DIR_ETC			"/etc/"
	#define DIR_SHARED		"/mnt/data/"
#endif

#endif
