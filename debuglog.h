#ifndef _DEBUGLOG_H_
#define _DEBUGLOG_H_

#define LOG_DFLT_FILE "c:\\krot.log"  // имя лог-файла по умолчанию

extern char *LogFile;                 // имя лог-файла
extern char LogString[];              // строка для записи в лог
extern void Log(void);                // Функция записи в лог

#endif
