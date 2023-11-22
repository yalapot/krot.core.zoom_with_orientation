/*
krotw32.c (C) Vitaly Bogomolov 2001-2004.
DLL ������������ ���������� ��������� ��� �������������� � ����������
�������. �������������� �������������� ��� ������ ������� ��������������
������ ������� �������. (��.������ "������� �������")
*/

#include "krotw32.h"
#include "scan.h"

#include "debuglog.h"

T_TRACELIST *traceList = NULL; // ������ �������� ��������
char lastError[KRT_TEXT];      // �������� ��������� ������

long flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_DOWN;


// ������� ��������� ������
short allocMem(
 long bufSize, 
 void **bufAddr, 
 const char *msg
) {

if (*bufAddr) { free(*bufAddr); }
*bufAddr = malloc(bufSize);
if (*bufAddr == NULL) {
 sprintf(lastError, "%s. �� ���������� ������ (%ld ����)", msg, bufSize);
 return 0;
}
return 1;
}

/*
������� ���� � ������ �������� �������� ������ � ������������ Handle
���������� ��������� �� ��������� ������. ���� ������ � �������� 
������������ �� ������, ���������� NULL.
*/
T_TRACE *TraceList (KRTHANDLE Handle) {
T_TRACELIST *trc;

 trc = traceList;
 while (trc) {
  if (trc->trace.vbHandle == Handle)
   {return &(trc->trace);}
   else
   {trc = trc->next;}
 }
 sprintf(lastError, "�� ���� ����� ���� �������� ������� ��� ����������� %d", Handle);
 return NULL;
}

/*
������� ���������� ��������� �������� ��������� ������, ��������� ��� 
��������� � �������� �������, ��������� ���������� trc.
���� ������� ����� ������� �� ������������ ����������� �������� ������, 
������������ ������ ������.
*/
const char *driverError(T_TRACE *trc) {
static char drvError[KRT_TEXT];

 if (trc->record.krtDrvError == NULL) {
  return "";
 } else {
  sprintf (
   drvError, 
   "\n%s", 
   (*(trc->record.krtDrvError)) ()
          );
  return drvError;
 }
}

void closeScan(T_WELDSCAN *scn) {
 scn->lastPos = -1;
 scn->bufSize = 0;
 if (scn->dat) free(scn->dat);
 if (scn->row) free(scn->row);
 scn->dat = NULL;
 scn->row = NULL;
}

void initScan(T_WELDSCAN *scn) {
 scn->lastPos = -1;
 scn->bufSize = 0;
 scn->dat = NULL;
 scn->row = NULL;
}

long *tmp_orient = NULL;

/* ����� ���� ���� 2013 */
short call2013 (
 KRTHANDLE Handle, 
 VB_TUBE_SCAN_IN *inpData, 
 VB_TUBE_SCAN_OUT *outData
) {

short ret, i;
T_TRACE *trc;
T_SCAN_2013_OUTPUT out;
long bufSize, bufStart;
T_CRZSENS *crz;
T_WELDSCAN *scn;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 bufStart = inpData->scanStart / trc->record.stepSize;
 bufSize  = inpData->lenMax / trc->record.stepSize;

 for (i=0; i<trc->record.sensGroups; i++) {

  crz = &(trc->crz[i]);
  scn = &(crz->scan);

  // �������� ������ ��� �������, ���� ����������
  if (bufSize > scn->bufSize) {

    if (!(
      allocMem(bufSize * crz->sNum * sizeof(KRTROW),  &(scn->row), "����� ����� ������ ������� ���� 2013") &
      allocMem(bufSize * crz->sNum * sizeof(KRTDATA), &(scn->dat), "����� ������������ ������ ������� ���� 2013") &
      allocMem(bufSize * sizeof(long), (void**) &(tmp_orient), "����� ������� ���������� ������� ���� 2013")
      )) {
      return KRT_ERR;
    }
    scn->bufSize = bufSize;
    scn->lastPos = -1;

  }

  // ����� ����� �������� ���������� ���������, � �� ������������ ����� ������� ������
  if (bufStart != scn->lastPos) {
    if (readData(trc, i, bufStart, bufSize, scn->dat, scn->row, tmp_orient, NULL, NULL)) {
      return KRT_ERR;
    }
    scn->lastPos = bufStart;
  }

 }

 ret = AnalyseData(trc, inpData, &out);
 if (ret) {
  sprintf(lastError, "������ ProccessPage ���: %d", ret);
  return KRT_ERR;
 }

 if (out.wldIndex < 0) {
  outData->wldDst = -1;
  outData->wldTyp = -1;
  outData->wld1   = -1;
  outData->wld2   = -1;
  outData->crzIndex = -1;
 } else {
  outData->wldDst = out.wldIndex * trc->record.stepSize + inpData->scanStart;
  outData->wldTyp = out.wldTyp;
  outData->wld1   = out.slit1; 
  outData->wld2   = out.slit2; 
  outData->crzIndex = out.crzIndex; 
 }

 return KRT_OK;
}

/**************************************************************************/
/*                           ������� �������                              */

short EXPORT KRTAPI krotScanWeld (
 KRTHANDLE Handle, 
 long crzIndx, 
 VB_TUBE_SCAN_IN *inpData, 
 VB_TUBE_SCAN_OUT *outData,
 long is2013
) {

T_TRACE *trc;
T_CRZSENS *crz;
T_WELDSCAN *scn;
T_SCAN_INPUT inp;
T_SCAN_OUTPUT out;
long bufSize, bufStart;
short ret;

 if (is2013) {
   return call2013(Handle, inpData, outData);
 }

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);
 scn = &(crz->scan);

 bufStart = inpData->scanStart / crz->step;
 bufSize  = inpData->lenMax    / crz->step;

 // �������� ������ ��� �������, ���� ����������
 if (bufSize > scn->bufSize) {

  if (!(
   allocMem(bufSize * crz->sNum * sizeof(KRTROW),  &(scn->row), "����� ����� ������ ������� ����") &
   allocMem(bufSize * crz->sNum * sizeof(KRTDATA), &(scn->dat), "����� ������������ ������ ������� ����") &
   allocMem(bufSize * sizeof(long), (void**) &(tmp_orient), "����� ������� ���������� ������� ���� 2013")
   )) {
   return KRT_ERR;
  }
  scn->bufSize = bufSize;
  scn->lastPos = -1;

 }

 // ����� ����� �������� ���������� ���������, � �� ������������ ����� ������� ������
 if (bufStart != scn->lastPos) {
  if (readData(trc, crzIndx, bufStart, bufSize, scn->dat, scn->row, tmp_orient, NULL, NULL)) {
   return KRT_ERR;
  }
  scn->lastPos = bufStart;
 }

 inp.x        = scn->bufSize;
 inp.y        = crz->sNum;
 inp.dat      = scn->dat;
 inp.row      = scn->row;

 inp.minTubeLen   = inpData->lenMin / crz->step;
 inp.maskSize     = inpData->maskSize;
 inp.listSize     = inpData->listSize / crz->step;
 inp.signalLevel  = inpData->signalLevel;
 inp.slitNum      = inpData->slitNum;
 inp.weldSensitiv = inpData->weldSensitiv;
 inp.slitSensitiv = inpData->slitSensitiv;
 inp.spirSensitiv = inpData->spirSensitiv;

 inp.noDRC    = inpData->noDRC;  
 inp.noSPR    = inpData->noSPR;  
 inp.noWTO    = inpData->noWTO;  

 out.wldIndex = -1;
 out.wldTyp   = -1;
 out.slit1    = -1;
 out.slit2    = -1;

 ret = ProccessPage(&inp, &out);
 if (ret) {
  sprintf(lastError, "������ ProccessPage ���: %d", ret);
  return KRT_ERR;
 }

 if (out.wldIndex < 0) {
  outData->wldDst = -1;
  outData->wldTyp = -1;
  outData->wld1   = -1;
  outData->wld2   = -1;
 } else {
  outData->wldDst = out.wldIndex * crz->step + inpData->scanStart;
  outData->wldTyp = out.wldTyp;
  outData->wld1   = out.slit1; 
  outData->wld2   = out.slit2; 
 }

 return KRT_OK;
}

/* *************************************************************************
������� ��������� ��������� T_SENSGROUP ����������� � ����� ��������
����������� ���������. ����� ����� �������� ���������� crzIndex.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotExtCorozInfo (
 KRTHANDLE Handle, 
 long crzIndx, 
 T_SENSGROUP *inf
) {

T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 if (crzIndx >= trc->record.sensGroups) {
  sprintf(lastError, "�������� ������ ����� �������� %d", crzIndx);
  return KRT_ERR;
 }

 *inf = trc->record.group[crzIndx];
 return KRT_OK;
}

/* *************************************************************************
������� ���������� ��� ������� ����� ���������������� ������� ��������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotExtSensorInfo (
 KRTHANDLE Handle,       // ���������� �������
 long index,             // ������ ���������������� �������
 T_SENS *sens,           // ��������� �������
 char *lpReturnedString  // �����, ���� ���������� ������ �������� �������
) {

T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 if (trc->record.extSensors == 0) {
  sprintf(lastError, "� �������� ��� ��������������� ��������");
  return KRT_ERR;
 }

 if (trc->record.extSensors <= index) {
  sprintf(lastError, "�������� ������ �������.\n������������: %d", 
          trc->record.extSensors - 1 
         );
  return KRT_ERR;
 }

 //*sens =  trc->record.extSens[index].sens;
 sens->min = trc->record.extSens[index].minValue;
 sens->max = trc->record.extSens[index].maxValue;
 sens->num = 1;
 sens->step = -1;

 strncpy(lpReturnedString, trc->record.extSens[index].name, EXT_SENS_NAME_LENGTH);
 return KRT_OK;
}

/* *************************************************************************
������� ��������� ������ outString ��������� ��������� ������ ������������
��� ������ � �������� ��������. ���������� ����� ������ ��������� �� ������.
*/
long EXPORT KRTAPI krotError (LPSTR outString) {
 strcpy(outString, lastError);
 return strlen(lastError);
}

/**************************************************************************
������� ���������, �������� �� ���� dll �������� ���������� driverFileName
���������� ��������� �������
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotCheckDriver (
 LPSTR driverFileName, 
 LPSTR outDrvSign,     // ��������� ��������
 LPSTR outDrvName,     // �������� ��������
 LPSTR outDrvCopyRt,   // ������ ���������
 LPSTR outFullDllPath, // ������ ���� �� ����������� dll ��������
 VB_DRV_INFO *drvInfo
) {

T_DRIVER drv;

 if (LoadDriver(driverFileName, &drv) == KRT_ERR) {
  sprintf (lastError, "������ ��� �������� ��������:\n%s", drv_Error);
  return KRT_ERR;
 }

 drvInfo->apiVer     = drv.apiVer;
 drvInfo->isSpiral   = drv.isSpiral;
 drvInfo->drvVerMax  = drv.verMax; 
 drvInfo->drvVerMin  = drv.verMin; 

 strncpy(outDrvSign, drv.sign, KRT_TEXT); 
 strncpy(outDrvName, drv.name, KRT_TEXT); 
 strncpy(outDrvCopyRt, drv.copyRt, KRT_TEXT); 

 GetModuleFileName(drv.inst, outFullDllPath, KRT_TEXT);

 UnloadDriver(&drv);
 return KRT_OK;
}

short byteOnBits(long bits) {
short i;

 i = (short) (bits / 8);
 return (short) (((bits % 8) > 0) ? (i+1) : i); 
}

/*
long maxValOnBits(long bits) {
long i,j;

 if (bits == 0) return 0;

 i=1;
 for (j=1; j<bits; j++) {
  i = i << 1;
  i = i | 1;
 }
 return i;
}
*/

void freeTrace(T_TRACELIST *tl) {
T_TRACE *trc;
T_CRZSENS *crz;
long i;

 trc = &(tl->trace);

 for (i=0; i < trc->record.sensGroups; i++) {
  crz = &(trc->crz[i]);

  if (crz->sensor) free(crz->sensor);
  crz->hide = 0;
  crz->sensor = NULL;

//if (crz->datPageOrnt) free(crz->datPageOrnt);
  crz->datPageOrnt = NULL;
//if (crz->datScrlOrnt) free(crz->datScrlOrnt);
  crz->datScrlOrnt = NULL;

  if (crz->datPageBuff) free(crz->datPageBuff);
  crz->datPageBuff = NULL;
  if (crz->datScrlBuff) free(crz->datScrlBuff);
  crz->datScrlBuff = NULL;

  if (crz->dat0PageBuff) free(crz->dat0PageBuff);
  crz->dat0PageBuff = NULL;
  if (crz->dat0ScrlBuff) free(crz->dat0ScrlBuff);
  crz->dat0ScrlBuff = NULL;

  if (crz->bmpBuffScroll) { free(crz->bmpBuffScroll); }
  crz->bmpBuffScroll = NULL;
  if (crz->bmpBuff) free(crz->bmpBuff);
  crz->bmpBuff = NULL;
  crz->maxBmpSize = 0;

  if (crz->bmp) DeleteObject(crz->bmp);
  crz->bmp = NULL;

  palClose(&(crz->pal));
  closeZoom(&(crz->zoom));
  closeScan(&(crz->scan));

 }

 if (trc->record.extSens) free(trc->record.extSens);
 trc->record.extSens = NULL;
 if (trc->crz) free(trc->crz);
 trc->crz = NULL;
 if (trc->record.group) free(trc->record.group);
 trc->record.group = NULL;

 UnloadDriver(&(trc->drv));

 if (tl) free(tl);

}

void checkSensDiapazon(T_SENS *sens) {
 if ((sens->num > 0) && (sens->min == sens->max)) sens->max++;
}


long read_data_profil_ini(T_CRZSENS *crz, char *prfFile)
{
   char key_name[1024];
   char key_value[1024];

   long i, j, k;

   char seps[]   = " ,\t\n";
   char *token;
   long next_token; 


   // ��������� ������������� ������� �� ������ ������
   for (i=0; i<  MAX_CALIBR_SENS_NUM; i++) {
       k=1000;
       for (j=0; j < MAX_CALIBR_LEVEL; j++) {
           crz->profil_calibrate[i][j]=k;
           k += 100;
       }
   }

   // ������ �� trc-����� ���������� ���������� ��������
   for (i=0; i < crz->profil_sens_quantity; i++) {
      sprintf(key_name, "%s%i", "Profil", i);
      if (GetPrivateProfileString("MagnetSystem", key_name, "1000", key_value, sizeof(key_value), prfFile) != 0)
      {
         next_token=0;
         token = strtok(key_value, seps);
         while (token!=NULL) {
            crz->profil_calibrate[i][next_token]=atoi(token);

            next_token++;
            token = strtok(NULL, seps);
         }

         for (j=next_token; j < MAX_CALIBR_LEVEL; j++) {
            crz->profil_calibrate[i][j] = 
                    crz->profil_calibrate[i][j-1] - 100;
         }
      } else {
          crz->profil_sens_quantity = i;
          break;
      }
   }
   // ��������� �� trc-����� ���������� ���������� ��������

   return KRT_OK;
} // long read_data_profil_ini(T_CRZSENS *crz, char *trcFile)


/**************************************************************************
������� ��������� ������ ������� ���������� � ����� fileName ��� ������
�������� driverFileName, ��������� �������� ������ � ������������ Handle
� ��������� ����������� �� �������� ������� ��������� inf.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotOpenTrace (
 KRTHANDLE Handle, 
 LPSTR fileName, 
 LPSTR indxFolder, 
 LPSTR driverFileName, 
 long (KRTAPI *newDataRegistered) (T_ArrivedData *newData),
 VB_TRACE_INFO *inf) {

T_TRACELIST *tl;
T_TRACE *trc;
T_CRZSENS *crz;
char *arr; 
long i, j, k, datSize;

 (void) indxFolder;

 // ��������� ������������ �����������
 trc = TraceList(Handle);
 if (trc) {
  sprintf(lastError, "���������� ��� ������������: %d", Handle);
  return KRT_ERR;
 }

 // �������� ������ ��� ���� �������� �������
 tl = (T_TRACELIST *) malloc(sizeof(T_TRACELIST));
 if (tl== NULL) {
  sprintf (
   lastError, 
   "�� ���� �������� %ld ���� ������ ��� ����� �������� �������", 
   sizeof(T_TRACELIST)
          );
  return KRT_ERR;
 }

 // ������������������� ��� ��������� ������
 arr = (char *) tl;
 for (i=0; i<sizeof(T_TRACELIST); i++) arr[i]=0;
 trc = &(tl->trace);

 // ��������� �������
 if (LoadDriver(driverFileName, &(trc->drv)) == KRT_ERR) {
  sprintf (lastError, "������ ��� �������� ��������:\n%s", drv_Error);
  return KRT_ERR;
 }

 // ������ ����� krtOpenTrace - ������ ���-�� ������
 // �������� ���������, ���-�� ��������������� ��������
 if ((*(trc->drv.krtOpenTrace)) (fileName, Handle, &(trc->record), 1) == KRT_ERR) { //indxFolder
  sprintf (
   lastError, 
   "������ �������� ������ %s :\n%s", 
   fileName, driverError(trc)
          );
  freeTrace(tl);
  return KRT_ERR;
 }

 /* ��������������� ������ ��� ���������� ���-�� ������ �������� ��������� */
 i = trc->record.sensGroups;

 if (i > 0) {

  j = sizeof(T_SENSGROUP) * i;
  trc->record.group = (T_SENSGROUP *) malloc(j);
  if (trc->record.group == NULL) {
   sprintf (
    lastError, 
    "�� ���� �������� %d ���� ������ ��� �������� %d ������ �������� ���������", 
    j, i
           );
   freeTrace(tl);
   return KRT_ERR;
  }
  /* ������������������� ��������� �������� �������� ��������� ������ */
  arr = (char *) trc->record.group;
  for (k=0; k<j; k++) arr[k]=0;

  j = sizeof(T_CRZSENS) * i;
  trc->crz = (T_CRZSENS *) malloc(j);
  if (trc->crz == NULL) {
   sprintf (
    lastError, 
    "�� ���� �������� %d ���� ������ ��� ���������� �������� %d ������ �������� ���������", 
    j, i
           );
   freeTrace(tl);
   return KRT_ERR;
  }
  /* ������������������� ��������� �������� �������� ��������� ������ */
  arr = (char *) trc->crz;
  for (k=0; k<j; k++) arr[k]=0;

 } else {

  trc->record.group = NULL; /* �������� ��������� ��� */ 
  trc->crz = NULL;

 }

 /* ��������������� ������ ��� ���������� ���-�� ��������������� �������� */
 i = trc->record.extSensors;
 j = sizeof(T_EXTSENS) * i;
 if (i > 0) {
  trc->record.extSens = (T_EXTSENS *) malloc(j);
  if (trc->record.extSens == NULL) {
   sprintf (
    lastError, 
    "�� ���� �������� %d ���� ������ ��� �������� %d ��������������� ��������", 
    j, i
           );
   freeTrace(tl);
   return KRT_ERR;
  }
  /* ������������������� ��������� �������� ��������������� �������� ������ */
  arr = (char *) trc->record.extSens;
  for (i=0; i<j; i++) arr[i]=0;
 } else {
  trc->record.extSens = NULL; /* ��������������� �������� ��� */ 
 }

 /* ������ ����� krtOpenTrace - ������ ������� �������� ������ */
 if ((*(trc->drv.krtOpenTrace)) (fileName, Handle, &(trc->record), 0) == KRT_ERR) { //indxFolder, 
  sprintf (
   lastError, 
   "������ ��� ������ krtOpenTrace %s", 
   driverError(trc)
          );
  freeTrace(tl);
  return KRT_ERR;
 }

 for (i=0; i < trc->record.sensGroups; i++) {
  crz = &(trc->crz[i]);
  crz->index = i;
  crz->hide = 0;
  j = trc->record.group[i].num;
  crz->sensor = (T_SENSOR *) malloc(j * sizeof(T_SENSOR));
  if (crz->sensor == NULL) {
   sprintf (
    lastError, 
    "�� ���� �������� %d ���� ������ ��� �������� �������� ����� %d", 
    j * sizeof(KRTDATA), i
           );
   freeTrace(tl);
   return KRT_ERR;
  }
  palInit(&(crz->pal)); // ���������������� �������
  initZoom(&(crz->zoom)); // ���������������� �����
  initScan(&(crz->scan)); // ���������������� ������ �������

  /* �������������� ��� � ���-�� �������� ������� ����� ��������� � ��������� T_CRZSENS */
  crz->sNum = trc->record.group[i].num;
  crz->step = trc->record.stepSize;
  crz->sType = trc->record.group[i].type; // ��� �������� ����� SENS_TYPE_* �� krtBase.h

  if (crz->sType == SENS_TYPE_PROFIL)
  { // ��������� �� trc ����� ���������� � ������ ��������� �������
      char key_name[1024];
      char key_value[1024];

      sprintf(key_name, "%s", "calibrate_date");
      GetPrivateProfileString("MagnetSystem", key_name, "", key_value, sizeof(key_value), fileName);
      strncpy(crz->calibrate_date, key_value, 16);

      sprintf(key_name, "%s", "id_device");
      GetPrivateProfileString("MagnetSystem", key_name, "", key_value, sizeof(key_value), fileName);
      strncpy(crz->id_device, key_value, 16);

      sprintf(key_name, "%s", "profil_sens_type");
      GetPrivateProfileString("MagnetSystem", key_name, "", key_value, sizeof(key_value), fileName);
      toupper(key_value[0]);
      if ( key_value[0] == 'A') crz->profil_sens_type = ANALOG_SENS;
        else crz->profil_sens_type = ENCODER_SENS;

      sprintf(key_name, "%s", "length_sens");
      GetPrivateProfileString("MagnetSystem", key_name, "0", key_value, sizeof(key_value), fileName);
      crz->length_sens = atof(key_value);

      sprintf(key_name, "%s", "zero_angle_gradus");
      GetPrivateProfileString("MagnetSystem", key_name, "0", key_value, sizeof(key_value), fileName);
      crz->zero_angle_gradus = atof(key_value);

      sprintf(key_name, "%s", "profil_sens_quantity");
      GetPrivateProfileString("MagnetSystem", key_name, "0", key_value, sizeof(key_value), fileName);
      crz->profil_sens_quantity = atoi(key_value);
	  
      if (crz->profil_sens_quantity <= 0) crz->profil_sens_quantity = crz->sNum;

      // ��������� ����������
      read_data_profil_ini(crz, fileName);

      sprintf(key_name, "%s", "PROFIL_ROW_INVERSE");
      GetPrivateProfileString("MagnetSystem", key_name, "0", key_value, sizeof(key_value), fileName);
      crz->profil_row_inverse = atoi(key_value);

  } // if (crz->sType == SENS_TYPE_PROFIL)

  /* ���������������� ������ ������ � ������� �������� ���������, ����� �� ��������� 
     � ����������������� ������ � ���������� */ 
  datSize = 10000000 / crz->sNum;
  if (datSize > 10000) { datSize = 10000; }

//  datSize = 100;

  if ( makeDatBuff(crz, datSize, "krotOpenTrace") || makeBmpBuff(crz, 1000, crz->sNum * 4) ) {
   freeTrace(tl);
   return KRT_ERR;
  }
  /* ������� �� ����� ���������� � ��������� ������ */
  crz->pageDat = 0;
  
 }
 trc->scaleX = 0;

 { // �������� �� trc-����� �������� ����� ����������
     char key_value[16];
 
     GetPrivateProfileString("DriverData", "Orientation_OFF", "0", key_value, sizeof(key_value), fileName);
     trc->Orientation_OFF = atoi(key_value);
 } // ��������� �� trc-����� �������� ����� ����������


 { // �������� �� trc-����� �������� ��� ������� "�� ���������"
     char key_value[16];

    // �� ������ ������ ��������� ���� �� ��������
    if (GetPrivateProfileString("DriverData", "Amplification_Group0", "10", key_value, sizeof(key_value), fileName)!=0)
    {
       trc->crz[0].Amplification_dflt = atoi(key_value);
    }; 

    // �� ������ ������ ��������� ���� �� ��������
    if (GetPrivateProfileString("DriverData", "Amplification_Group1", "10", key_value, sizeof(key_value), fileName)!=0)
    {
       trc->crz[1].Amplification_dflt = atoi(key_value);
    }; 

 } // ��������� �� trc-����� �������� ��� ������� "�� ���������"


 /* �������� ���������� �� VB ���������� */
 trc->vbHandle = Handle;

 /* �������� ������������������ ���� �������� ������� � ������ */
 tl->prev = NULL;
 if (traceList) traceList->prev = tl;
 tl->next = traceList;
 traceList = tl;

 if (trc->drv.krtOnline) {
  trc->isOnLine = (*(trc->drv.krtOnline)) (Handle, newDataRegistered);
 }

 /* �������� � VB ������ �� �������� ������� */
 inf->onLine      = trc->isOnLine;
 inf->crzZoneNum  = trc->record.sensGroups;
 inf->extSensors  = trc->record.extSensors;
 inf->VOG         = trc->record.vog;
 inf->evnt        = 0; //trc->record.krtDrvEvent ? 1 : 0;

 inf->odom.num    = trc->record.odoNum;
 inf->odom.step   = trc->record.stepSize; //trc->record.odoStep;
 inf->odom.min    = 0;
 inf->odom.max    = trc->record.length;

 inf->timer.num   = trc->record.timerNum;
 inf->timer.step  = trc->record.timerStep;
 inf->timer.min   = 0;
 inf->timer.max   = trc->record.time; //?

 inf->shake.num   = trc->record.shakeNum;
 inf->shake.step  = trc->record.shakeStep;
 inf->shake.min   = trc->record.shakeMin;
 inf->shake.max   = trc->record.shakeMax; 

 inf->press.num   = trc->record.pressNum;
 inf->press.step  = trc->record.pressStep;
 inf->press.min   = trc->record.pressMin;
 inf->press.max   = trc->record.pressMax; 

 inf->temp.num    = trc->record.tempNum;
 inf->temp.step   = trc->record.tempStep;
 inf->temp.min    = trc->record.tempMin;
 inf->temp.max    = trc->record.tempMax; 

 inf->angle.num   = trc->record.angleNum;
 inf->angle.step  = trc->record.angleStep;
 inf->angle.min   = trc->record.angleMin;
 inf->angle.max   = trc->record.angleMax; 

 inf->wall.num    = trc->record.wallThickNum;
 inf->wall.step   = trc->record.stepSize;
 inf->wall.min    = trc->record.wallThickMin;
 inf->wall.max    = trc->record.wallThickMax; 

 inf->orient.num  = trc->record.orientNum;
 inf->orient.step = trc->record.orientStep;
 inf->orient.min  = 0;
 inf->orient.max  = trc->record.group ? trc->record.group[0].num : 0; 

 inf->speed.num   = ((inf->timer.num > 0) && (inf->odom.num > 0)) ? 1 : 0;
 inf->speed.step  = trc->record.tempStep;
 inf->speed.min   = trc->record.speedMin;
 inf->speed.max   = trc->record.speedMax; 

// �������� ���������� ��������� ��������
// ���� ������ ������ == �������� �������, ������������� ������� ������, ������ ������� + 1.
// ��� ���� ����� ������ �������� ����� ��������� ������
 checkSensDiapazon(&(inf->shake));
 checkSensDiapazon(&(inf->press));
 checkSensDiapazon(&(inf->temp));
 checkSensDiapazon(&(inf->angle));
 checkSensDiapazon(&(inf->wall));
 checkSensDiapazon(&(inf->speed));

 return KRT_OK;
}

/**************************************************************************
������� ��������� ������ ������� ���������� � ������������ Handle
� ����������� �������, ������� ��� ������ � ���� ��������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotCloseTrace (KRTHANDLE Handle) {
T_TRACE *trc;
T_TRACELIST *tl;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR; 

 /* ����������� ������������ �������� ������, ���� ��� �������� */
 if (trc->isOnLine) {
  (*(trc->drv.krtOnline)) (trc->vbHandle, NULL);
  trc->isOnLine = 0;
 }

 /* ������� ������ �������� krtCloseTrace �������� */
 if ((*(trc->drv.krtCloseTrace)) (Handle) == KRT_ERR) {
  sprintf (
   lastError, 
   "������ ��� ������ krtCloseTrace %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }

 tl = traceList;
 while (tl->trace.vbHandle != Handle) tl = tl->next;

 /* ������� ���� �������� ������� �� ������ */
 if (tl->prev == NULL) {
  traceList = tl->next;
  if (traceList) traceList->prev = NULL;
 } else {
  tl->prev->next = tl->next;
 }
 /* ���������� ������ */
 freeTrace(tl);

 return KRT_OK;
}

short EXPORT KRTAPI krotGetFirstNode (
 KRTHANDLE Handle,      // ���������� �������
 T_NODE *node,          // ��������� �� ��������� T_NODE
 long start,            // ������� �� ������
 long sensType,         // ��� �������
 long sensIndex,        // ������ ������� (-1L - �������)
 long length,           // ����� ������� ��� ��������� ������� krotGetNextNode
 long controlSize       // ������ �������� �������� � ��������.
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 // ��������������� ������, ���� ���������� ����� ����������
 if (trc->lockNodeSeq) return KRT_OK;

 // ������ KRT_SENS_ANGLE ��������� �� ������ ���������� ��� ��������� �������
 if (sensType == KRT_SENS_ANGLE) {
   sensType = KRT_SENS_ORIENT;
 } 
 /* ��������� � �������� ��������� ������� */
 if (
     (*(trc->record.krtDrvGetFirstNode)) 
     (Handle, node, start, sensType, sensIndex, length, controlSize) == KRT_ERR
    ) {
  sprintf (
   lastError, 
   "������ ��� ������ krtDrvGetFirstNode: %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }
 return KRT_OK;
}

short EXPORT KRTAPI krotGetNextNode (
 KRTHANDLE Handle,      // ���������� �������
 T_NODE *node           // ��������� �� ��������� T_NODE
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 /* ��������� � �������� ��������� ������� */
 if ((*(trc->record.krtDrvGetNextNode)) (Handle, node) == KRT_ERR) {
  sprintf (
   lastError, 
   "������ ��� ������ krtDrvGetNextNode: %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }

 return KRT_OK;
}

short EXPORT KRTAPI krotGetFirstNodeGroup (
 KRTHANDLE Handle,      // ���������� �������
 T_NODEGROUP *node,     // ��������� �� ��������� T_NODEGROUP
 long start,            // ������� �� ������
 long sensGroup,        // ����� ������ ��������
 long length,           // ����� ������� ��� ����������� ������� GetNextNodeGroup
 long controlSize       // ������ �������� � ��������.
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 // ��������������� ������, ���� ���������� ����� ����������
 if (trc->lockNodeSeq) return KRT_OK;

 // ��� ������� KRT_SENS_ANGLE �������� � ������ ���������� ��� ��������� �������
 if (sensGroup & KRT_SENS_ANGLE) {
   sensGroup = sensGroup | KRT_SENS_ORIENT;
 } 
 /* ��������� � �������� ��������� ������� */
 if (
     (*(trc->record.krtDrvGetFirstNodeGroup)) 
     (Handle, node, start, sensGroup, length, controlSize) == KRT_ERR
    ) {
  sprintf (
   lastError, 
   "������ ��� ������ krtDrvGetFirstNodeGroup: %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }
 return KRT_OK;
}

short EXPORT KRTAPI krotGetNextNodeGroup (
  KRTHANDLE Handle,      // ���������� �������
  T_NODEGROUP *node      // ��������� �� ��������� T_NODE
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 /* ��������� � �������� ��������� ������� */
 if ((*(trc->record.krtDrvGetNextNodeGroup)) (Handle, node) == KRT_ERR) {
  sprintf (
   lastError, 
   "������ ��� ������ krtDrvGetNextNode: %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }

 return KRT_OK;
}

drv2_informUser realCallBack;

long KRTAPI oldCallBack (short percentDone) {
 return (* realCallBack) (percentDone, " ");
}

/**************************************************************************
������� ����������� �������. ��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotRegisterTrace (
 LPSTR driverFileName,                   // ��� ����� ��������, ������� ������ ��������������
 LPSTR primFile,                         // ��� ����� � ��������� �����������
 LPSTR indxFolder,                       // ������� ��� ��������� ������
 LPSTR trcFile,                          // ��� trc-����� 
 long (KRTAPI *informUser) (short percentDone, const char *msg) // ��������� �� callback ������� ������ ���������
) {                                      // ���������� � ���������� ������ �������������
T_DRIVER drv;
char *errText;
short ret;

 // ��������� �������
 if (LoadDriver(driverFileName, &drv) == KRT_ERR) {
  sprintf (lastError, "������ ��� �������� ��������:\n%s", drv_Error);
  return KRT_ERR;
 }

 if (drv.krtRegister2) {
  // ���� ������� ����������� ������� �����������, ������������ ��
  ret = (short) (*(drv.krtRegister2)) (primFile, trcFile, indxFolder, &errText, informUser);
 } else if (drv.krtRegister) {
     // � ��������� ������ ������������ ������ ������� �����������
     realCallBack = informUser;
     ret = (short) (*(drv.krtRegister)) (primFile, trcFile, &errText, oldCallBack);

     { // ������� ����������� ������ ���������� �������, ���� ��� �������!
         char calibrate_file_name[1024];
         FILE *calibrate_file;
         long calibrate_file_exist_flag=0;

         char path_data[1024];
         char key_value[1024];
         char key_name[1024];
         long sens_quantity;
         long i;


         if (drv.sens_type == SENS_TYPE_PROFIL) {
             /* =============== ������ ����� ���������� �������� ������� ========================
             ;����������� ��� ������� 1022 encoder 22.07.19

             [Profile_calidrate]

             calibrate_date = 22.07.19

             profil_sens_type = Encoder
             ; ������������ "= Analog"

             profil_sens_quantity = 32

             Profil0  = 621
             Profil1  = 630
             Profil2  = 627
             Profil3  = 650

             --//--//--//--

             Profil30 = 637
             Profil31 = 630
             =============== ������ ����� ���������� �������� ������� ========================*/

             // �������� ������ ���� �� trc-����� (��� ����� ������ �����)
             strcpy(key_value, trcFile);
             while((strlen(key_value)>0) && (key_value[strlen(key_value)-1]!='\\')) {
               key_value[strlen(key_value)-1]=0;
             };
             strcpy(path_data, key_value);

             // �������� ��� ���� ���������� ����
             calibrate_file_exist_flag = 0;
             sprintf(calibrate_file_name, "%s%s", path_data, "calibrat.prf");
             calibrate_file = fopen(calibrate_file_name, "rb");
             if (calibrate_file != NULL) calibrate_file_exist_flag = 1;
             fclose(calibrate_file);

             if (calibrate_file_exist_flag != 0){

                  sprintf(key_name, "%s", "calibrate_date");
                  GetPrivateProfileString("Profile_calidrate", key_name, "", key_value,
                                                sizeof(key_value), calibrate_file_name);
                  WritePrivateProfileString("MagnetSystem", key_name, key_value, trcFile);

                  sprintf(key_name, "%s", "id_device");
                  GetPrivateProfileString("Profile_calidrate", key_name, "", key_value,
                                                sizeof(key_value), calibrate_file_name);
                  WritePrivateProfileString("MagnetSystem", key_name, key_value, trcFile);

                  sprintf(key_name, "%s", "profil_sens_quantity");
                  GetPrivateProfileString("Profile_calidrate", key_name, "", key_value,
                                                sizeof(key_value), calibrate_file_name);
                  sens_quantity = atoi(key_value);
                  WritePrivateProfileString("MagnetSystem", key_name, key_value, trcFile);

                 // �������� � trc-���� ���������� ���������� ��������
                 if (sens_quantity <= 0) sens_quantity = 500;
                 for (i=0; i < sens_quantity; i++) {
                     sprintf(key_name, "%s%i", "Profil", i);
                     if (GetPrivateProfileString("Profile_calidrate", key_name, "", key_value,
                                                 sizeof(key_value), calibrate_file_name) != 0)
                     {
                         WritePrivateProfileString("MagnetSystem", key_name, key_value, trcFile);
                     } else {
                         break;
                     }
                 }
                 // ����������� � trc-���� ���������� ���������� ��������
             }

         } // if (drv.sens_type == SENS_TYPE_PROFIL) {
     } // �������� ����������� ������ ���������� �������, ���� ��� �������!

 } else {

  errText =
   "������ ������� �� ����� ���������� ������� ����������� �������."
   "\n�������������� ����������� ���������� �����������.";
  ret = KRT_ERR;

 }

 if (ret == KRT_ERR) {
  sprintf (lastError, "������ �������� ��� ����������� ������ %s :\n%s", primFile, errText);
 }

 UnloadDriver(&drv);
 return ret;
}

/**************************************************************************
������� ������������� ����� ������������� ������� ������� krotFirstNode ��� ������� � ������������
Handle ��� ��������� lockActive == 1. ������� ���� ����� ��� lockActive == 0. ��� ������  ���������� KRT_OK, 
KRT_ERR ��� ������. ����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotLockNodeSequence (
 KRTHANDLE Handle, 
 long lockActive
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 if (lockActive && trc->lockNodeSeq) {
  sprintf (
   lastError, 
   "����� ���������� NodeSequence ��� ���������� ��� ������� %d.", 
   Handle
          );
  return KRT_ERR;
 }

 trc->lockNodeSeq = lockActive;
 return KRT_OK;
}

short EXPORT KRTAPI krotDai (
 KRTHANDLE Handle, 
 long crzIndx, 
 T_USERDAI *udai, 
 VB_DAI_INFO *daiInfo, 
 LPSTR outString, 
 long (DAIAPI *userBreak) (short percentDone)
) {
T_TRACE *trc;
T_CRZSENS *crz;
HINSTANCE hinstDllDAI;
long (DAIAPI *daiQuest) (T_DAI *quest, long (DAIAPI *informUser) (short percentDone), char *explainText);
T_DAI quest;
long daiReturn;

 trc = TraceList(Handle);
 if (trc == NULL) return DAI_QUEST_ERROR;
 crz = &(trc->crz[crzIndx]);

 /* ��������� ������� */
 hinstDllDAI = LoadLibrary("dai.dll");
 if (hinstDllDAI == NULL) /* load failed */
 {
  sprintf (
   lastError, 
   "�� ���� ��������� ������ dai.dll\nGetLastError code: %d", 
   GetLastError()
          );
  return DAI_QUEST_ERROR;
 }

 /* ����� ������� daiQuest */
 daiQuest = (long (DAIAPI *) (T_DAI *, long (DAIAPI *) (short), char *)) GetProcAddress(hinstDllDAI, "daiQuest");
 if (daiQuest == NULL) {
  sprintf (
   lastError, 
   "�� ���� ����� ������� daiQuest � ������ dai.dll"
          );
  FreeLibrary(hinstDllDAI);
  return DAI_QUEST_ERROR;
 }

 quest.user.internal         = udai->internal;
 quest.user.wallThickness    = udai->wallThickness;
 quest.user.x1               = udai->x1;
 quest.user.y1               = udai->y1;
 quest.user.x2               = udai->x2;
 quest.user.y2               = udai->y2;

 quest.xSize                 = crz->zoom.xSizeDat;
 quest.ySize                 = crz->zoom.ySizeDat;
 quest.data                  = crz->zoom.datBuffer;
 quest.orntStart             = daiInfo->orntStart;
 quest.orntLen               = daiInfo->orntLen;
 quest.itemX                 = daiInfo->itemX;
 quest.itemY                 = daiInfo->itemY;

 /* ����� daiQuest */
 daiReturn = (*daiQuest) (&quest, userBreak, outString);
 if ( daiReturn == DAI_QUEST_ERROR) {
  sprintf (
   lastError, 
   "������ ��� ������ daiQuest:\n%s", outString 
          );
  FreeLibrary(hinstDllDAI);
  return DAI_QUEST_ERROR;
 }

 FreeLibrary(hinstDllDAI);
 return (short) daiReturn;
}

short EXPORT KRTAPI krotEvent (
 KRTHANDLE Handle,
 T_EVENT *event
) {
T_TRACE *trc;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 (void) event;
 sprintf (lastError, "���������� � �������� ������� �� ��������������");
 return KRT_ERR;
 
 /* ���� �� � �������� ������� �������? */
 /*
 if (trc->record.krtDrvEvent == NULL) {
  sprintf (lastError, "������� �� ������������ ���������� � �������� �������");
  return KRT_ERR;
 }
*/

 /* ��������� � �������� ��������� ������� */
 /*
 if ((*(trc->record.krtDrvEvent)) (Handle, event) == KRT_ERR) {
  sprintf (
   lastError, 
   "������ ��� ������ krtDrvEvent: %s", 
   driverError(trc)
          );
  return KRT_ERR;
 }

 return KRT_OK;
 */
}