// файл дл€ подключени€ системного заголовочного файла windows.h
// позвол€ющий компилировать его с максимальным уровнем предупреждений
// без warning-ов
// (C) 2006 by Vitaly Bogomolov

#ifdef _MSC_VER
/*
 * Disable following warnings when including Windows headers
 * 
 * warning C4091: 'typedef ' : ignored on left of 'unsigned char ' when no variable is declared
 * warning C4114: same type qualifier used more than once
 * warning C4115: named type definition in parentheses
 * warning C4116: unnamed type definition in parentheses
 * warning C4201: nonstandard extension used : nameless struct/union
 * warning C4214: nonstandard extension used : bit field types other than int
 */
#pragma warning( disable : 4091 4114 4115 4116 4201 4214)
#endif

#include <windows.h>

#ifdef _MSC_VER
/*
 * Re-enable all
 */
#pragma warning( default : 4091 4114 4115 4116 4201 4214)
#endif
