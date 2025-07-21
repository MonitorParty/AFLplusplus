#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// AFL++ Persistent Mode Header
#include "../include/afl-fuzz.h"

// Globale Variablen für den State
static char buf[100] = {0};

__AFL_FUZZ_INIT();

int main() {
    // Persistent Mode Loop
    while (__AFL_LOOP(1000)) {  // 1000 Iterationen pro Prozess
        
        // Input lesen (direkt aus AFL)
        unsigned char *input = __AFL_FUZZ_TESTCASE_BUF;
        size_t len = __AFL_FUZZ_TESTCASE_LEN;
        
        // Input kopieren (mit Längencheck)
        memset(buf, 0, sizeof(buf));
        if (len > sizeof(buf)) len = sizeof(buf);
        memcpy(buf, input, len);

        // Fehler 1: Buffer Overflow
        if (buf[0] == 'A' && buf[1] == 'B' && buf[2] == 'C') {
            char crash[4];
            memcpy(crash, buf + 97, 10); // absichtlicher Buffer Overflow
        }

        // Fehler 2: Division durch Null
        if (memcmp(buf, "CRASH!", 6) == 0) {
            int zero = 0;
            printf("%d\n", 1 / zero);
        }

        // Fehler 3: Use-After-Free
        if (buf[10] == 'X' && buf[11] == 'Y') {
            char *p = malloc(10);
            free(p);
            p[0] = 'Z'; // Use-After-Free
        }

        // Fehler 4: Integer Overflow
        if ((unsigned char)buf[20] + (unsigned char)buf[21] < buf[20]) {
            int big = 0x7fffffff;
            big += buf[22];
            if (big < 0) abort();
        }

        // Fehler 5: Segfault durch NULL-Pointer
        if (buf[30] == 'S' && buf[31] == 'E' && buf[32] == 'G') {
            char *p = NULL;
            p[0] = '!';
        }

	//error 6: hang indef loop 
	if(memcmp(buf, "HANG!", 5) == 0){
		printf("Entering infinitive loop... :D");
		while(1){}
	}

	//hang 7: long sleep 
	if(memcmp(buf, "SLEEP!", 6) == 0){
		printf("Sleeping for 10 secs...");
		sleep(10);
	}
	//hang 8: cpu-inten 
	if(memcmp(buf, "CPUHANG!", 8) == 0){
		printf("CPU-intense loop");
		volatile unsigned long long i = 0;
		while(1){i++;}
	}

	//hang 8: deadlock 
	if(memcmp(buf, "DEADLOCK!", 9) == 0){
		printf("deadlock...");
		while(1){
			sleep(1);
		}
	}


        // Edge Detection
        if (buf[40] == 'F' && buf[41] == 'U' && buf[42] == 'Z' && buf[43] == 'Z') {
            printf("Fuzzing rocks!\n");
        }

        // Pfadauswahl
        if (buf[50] == 'A') {
            printf("Path A\n");
        } else if (buf[50] == 'B') {
            printf("Path B\n");
        } else {
            printf("Path C\n");
        }
    }

    return 0;
}
