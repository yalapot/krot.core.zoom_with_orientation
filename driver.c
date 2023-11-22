#include <stdio.h>
#include <string.h>
#include "driver.h"

char drv_Error[KRT_TEXT]; // описание последней ошибки

///////////////////////////////////////////////////////
long UnloadDriver(T_DRIVER *drv) {
 if (drv->inst) FreeLibrary(drv->inst);
 drv->inst = NULL;
 return KRT_OK;
}

///////////////////////////////////////////////////////
long noFunc(T_DRIVER *drv, const char *func) {
 sprintf(drv_Error, "в драйвере %s отсутствует функция %s", drv->file, func);
 UnloadDriver(drv);
 return KRT_ERR;
}

///////////////////////////////////////////////////////
long LoadDriver(const char *drvFile, T_DRIVER *drv) {
T_DRVINFO driver1;
T_DRVINFO2 driver2;

 strncpy(drv->file, drvFile, KRT_TEXT-1);

 drv->inst = LoadLibrary(drv->file);
 if (drv->inst == NULL) {
  sprintf (
   drv_Error, 
   "Не могу загрузить драйвер %s\nGetLastError code: %d", 
   drv->file, 
   GetLastError()
          );
  return KRT_ERR;
 }

 // найти функцию krtDriverInfo
 drv->krtDriverInfo = (drv_DriverInfo) GetProcAddress(drv->inst, FUNC_INFO);
 if (!drv->krtDriverInfo) return noFunc(drv, FUNC_INFO);

 // найти функцию krtOpenTrace
 drv->krtOpenTrace = (drv_OpenTrace) GetProcAddress(drv->inst, FUNC_OPEN);
 if (!drv->krtOpenTrace) return noFunc(drv, FUNC_OPEN);

 // найти функцию krtCloseTrace
 drv->krtCloseTrace = (drv_CloseTrace) GetProcAddress(drv->inst, FUNC_CLOSE);
 if (!drv->krtCloseTrace) return noFunc(drv, FUNC_CLOSE);

 // вызов krtDriverInfo - запрос версии API драйвера
 if ((*(drv->krtDriverInfo)) (&driver1) == KRT_ERR) {
  sprintf (drv_Error, "Ошибка при вызове krtDriverInfo");
  UnloadDriver(drv);
  return KRT_ERR;
 }

 // проверить версию API драйвера
 switch ( driver1.apiVer ) {
  
  case KRT_APIVER:

   drv->apiVer           = KRT_APIVER;
   drv->isSpiral         = 0;
   drv->verMax           = 0; 
   drv->verMin           = 0; 

   drv->krtOnline        = NULL;         
   drv->krtDriverInfoExt = NULL;  
   drv->krtRegister2     = NULL;

   strncpy(drv->sign, "V0", KRT_TEXT); 
   strncpy(drv->name, "Driver for first krtapi version", KRT_TEXT); 
   strncpy(drv->copyRt, "(C) by Vitaly Bogomolov", KRT_TEXT); 

   drv->krtRegister = driver1.krtDrvRegister;

   break;
  
  case KRT_APIVER2:

   drv->krtDriverInfoExt = (drv2_DriverInfoExt) GetProcAddress(drv->inst, FUNC_INFOEXT);
   if (!drv->krtDriverInfoExt) return noFunc(drv, FUNC_INFOEXT);

   // вызов krtDriverInfoExt - запрос параметров драйвера согласно заявленной версии API
   if ((*(drv->krtDriverInfoExt)) (KRT_APIVER2, &driver2) == KRT_ERR) {
    sprintf(drv_Error, "Ошибка при вызове krtDriverInfoExt");
    UnloadDriver(drv);
    return KRT_ERR;
   }

   drv->apiVer       = KRT_APIVER2;
   drv->isSpiral     = driver2.pigType;
   drv->verMax       = driver2.drvVerMax;
   drv->verMin       = driver2.drvVerMin;

   drv->krtRegister  = driver1.krtDrvRegister;
   drv->krtOnline    = driver2.krtOnline;         
   drv->krtRegister2 = driver2.krtDrvRegister;

   strncpy(drv->sign,   driver2.drvSign,   KRT_TEXT); 
   strncpy(drv->name,   driver2.drvName,   KRT_TEXT); 
   strncpy(drv->copyRt, driver2.drvCopyRt, KRT_TEXT); 

   break;

  case KRT_APIVER_3:

   drv->apiVer           = KRT_APIVER_3;
   drv->isSpiral         = 0;
   drv->verMax           = 0; 
   drv->verMin           = 0; 

   drv->krtOnline        = NULL;         
   drv->krtDriverInfoExt = NULL;  
   drv->krtRegister2     = NULL;

   strncpy(drv->sign, "V3", KRT_TEXT); 
   strncpy(drv->name, "Driver for 3 krtapi version", KRT_TEXT); 
   strncpy(drv->copyRt, "(C) by Vitaly Bogomolov", KRT_TEXT); 

   drv->krtRegister = driver1.krtDrvRegister;
   drv->sens_type = driver1.sens_type;

   break;

  default :

   sprintf(drv_Error, "Неизвестная версия API (%ld) у драйвера %s", driver1.apiVer, drv->file);
   UnloadDriver(drv);
   return KRT_ERR;
 }

 return KRT_OK;
}
