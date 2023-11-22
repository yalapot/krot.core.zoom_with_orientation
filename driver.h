// модуль загрузки/выгрузки драйверов формата
// (C) 2005 by Vitaly Bogomolov

#include "krtapi.h"
#include "krtapi2.h"

#define FUNC_INFO      "krtDriverInfo"
#define FUNC_OPEN      "krtOpenTrace"
#define FUNC_CLOSE     "krtCloseTrace"
#define FUNC_INFOEXT   "krtDriverInfoExt"

// обобщенная структура описания драйвера
typedef struct {

 HINSTANCE           inst;              // дескриптор драйвера

 long apiVer;                           // версия API драйвера
 long isSpiral;                         // внешнетрубник?
 long verMax;                           // номер версии драйвера, старший разряд
 long verMin;                           // номер версии драйвера, младший разряд

 // указатели на функции драйвера

 drv_Register        krtRegister;       // регистрация записи
 drv_DriverInfo      krtDriverInfo;     // функция описания драйвера
 drv_OpenTrace       krtOpenTrace;      // открытие прогона
 drv_CloseTrace      krtCloseTrace;     // закрытие прогона

 // указатели на функции драйвера версии 1

 drv2_Online         krtOnline;         // установка обработчика получения новых данных
 drv2_DriverInfoExt  krtDriverInfoExt;  // описание драйвера версии 1
 drv2_Register       krtRegister2;      // регистрация записи

 char                sign[KRT_TEXT];    // сигнатура
 char                name[KRT_TEXT];    // наименование
 char                copyRt[KRT_TEXT];  // строка копирайта
 char                file[KRT_TEXT];    // файл драйвера

 long sens_type;

} T_DRIVER;

extern char drv_Error[];                // описание последней ошибки

// функция загружает файл драйвера, заданный путем drvFile
// и заполняет структуру T_DRIVER информацией о нем.
extern long LoadDriver(const char *drvFile, T_DRIVER *drv);

// функция выгружает драйвер, ранее загруженный функцией LoadDriver
extern long UnloadDriver(T_DRIVER *drv);
