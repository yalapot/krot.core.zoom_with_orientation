#include <stdio.h>
#include "debuglog.h"

#pragma warning(disable : 4996) // for VC 8.00

char LogString[1024];          // строка для записи в лог
char *LogFile = LOG_DFLT_FILE; // имя лог-файла

// Функция записи в лог
// use:
// sprintf(
//  LogString, 
//  "checkVmode()"
// );
// Log();
void Log(void) {
FILE *l;

 l = fopen(LogFile, "at");
 fprintf(l, "%s\n", LogString);
// fflush(l);
 fclose(l);
}
