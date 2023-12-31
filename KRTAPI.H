#ifndef _KRTAPI_H_
#define _KRTAPI_H_


#pragma warning(disable : 4996) // for VC 8.00

/*
krtapi.h (C) Vitaly Bogomolov 2000-2003

������ ���� ������������ ��� ������������, ���������� ��������� 
��� ������������� ������ �������-������������ (������� �������) � ��������� 
��������� � �������.
��������� ��������� �� ����� ������� ������� � ������ ����������� �������, 
� ��������������� � ��������� ����������� �������, ������� ���������� ������ 
��������������� ������ � ������������� ���������� ���������� ��������� ��������� 
�� ����������� � ���� ����� ��������. ����� ����� ������ ��������� �������� 
������������� ������ ������������� ������� ��� ������� ������� �������� 
����� �������.
��������� ���������� �� �������-������������ ���������� �� ������ ���������� 
������.
������� ����� ���� ������� �� ����� ����� ����������������, ���������� �������� 
����� ��������� ���������� win32 dll. 
��� ��������� �������� �� ����� ����������������, �������� �� C/C++, 
������������ ���������� ���������� ���������� ������������ �������� ������, 
��������, ���������� ������� � ������ krtapi.h. 
����� ����� ���������� ���������� ���������� ��� ������������ �������� ������, 
������������ � krtapi.h (������������ �� 1 ����).
*/
/*
 History:

 AW 23.01.01 
 + ��������� compiler.h
 + ���������� ��������
 + krtOK, krtERR
 Vit 28.01.01
 + ���������� ���������� ������� krtDrvError (char -> char *)
 Vit 03.02.01
 + ������� ��� ������ ����������� ������� (unsigned int -> short)
 + ������� �� ���������� ���� ����������, � ���������� ���������� ������.
   �������������� �������� ���������� ������� �������� �������, ���������
   ������� ��������� ��� ������� � ������������ ����� � ������ �����.
 Vit 08.04.03
 + ��������� ���������� � ������������ �������� ���������� �� 
   krtDriverInfo � krtOpenTrace.
 Vit 11.04.03
 + ���������� �������� ������� ����������� ������: ��������� �� ������ � 
   call-back ������� ��� ��������� �������� ����������� ������ � ����������
   ������ �������������.
*/
/***********************************************************
������ �������������� �������� � ���������� ��������� ������

��������� ������:
1. ������� ����������� ��������� ����������� �� ���������� ����� ���������������
� ����������. �.�. ���� ������ ����� ������ �������, �� ������ ������ ��������
������ � 90 �������� � ������� ����������� ���� �� ������.

������� ��������������� � ���������� ��������� ��� ������ �������������� 
�������. ��������� ��������� �������� �������������� ��������� ������� �� �����. 
��� ���� ����� �������� �������� ���� �������, ������������ ���������� 
������������� (�������� ������������� C++) �������������� ������� ������� ���� 
���������� ��� extern "C". ��������������� ���������� ������� � ����� krtapi.h.

������� ������ ������������� ��������� �������: krtDriverInfo, krtOpenTrace, 
krtCloseTrace. ���� ������� �� ������������ ����� �� �� ���� ���� �������, 
�� ��� �������� �������� ���������� ��������� ����� ������ ��������� �� ������.

������� krtDriverInfo  ���������� ������. ��� ������ ������ ������� ��������� 
��������� � ��������� T_DRVINFO ���������� ���������� � ������ API, ������������
��������� � � ������� � �������� ������� ����������� �������.

������� ����������� ������� ������������ ��� �������� ����� �������� �������, 
������� ��������� ��� ������� �������� ������� krtOpenTrace. 
��������������� ���������� ������������ �������� ����������� � ���� �������� 
�������, ��� �������� ���������� �� ��� ��������. ���� �������� ������� �������� 
��������� ������, ������� ��������� ini-����� Windows. ��������� ��������� 
���������� � ���� ���� �������� � ���� ��������, ������� ���� �������� ���� 
������ � ��������� �������� �������. ������� ����������� ������� � �������� 
������ ���������� ����������� ��� �������� ���������� � ������ DRIVER_DATA. 
������ ��������� ���������� � ����� krtapi.h. 

���� ������� �� ������������ ������� ����������� �������, �� ��������������� 
���������� � ���� �������� ������� ������ �������� ������� ������� ����������� 
�������, ���� ��� ������ ������ ���� ������� ���� �������.

����� ������ ������� krtDriverInfo � ��������� ���������� � �������� ��������� 
��������� �������� ������� krtOpenTrace ��� ������ ������ � ���������� ������� 
�������. 

� �������� ��������� ������� krtOpenTrace ���������� ����� ��������� �������� 
������, ��� ����� �������� ������� � ���������� ������� (����� ��������������� 
�����) �� �������� ��� ���������� ������ ��������� ��������� ����� ��������� 
�� ���� ������. ����� ������� krtOpenTrace ���������� � ��� ����� (��. ��������
�������).

� ��������� �������� ������ ������� �������� ���������� � ��������������� 
�������, �������������� ��������� � ������ �������, ����������� ��������� ���� 
�������������. � ���������� ��������� ���������, ����������� �� ���� ����������, 
�������� ��������������� ������� �������� ��� ������ ����� ������ �������. 
��� ��� �������������� �������� � ���������� ��������� ��� ������ � ������ 
������� �������������� ����� ��������� T_DRVINFO ������ � ���� ��������� ������ 
����� ���������� ������������ � ��������� ��������� � � ��������. 
������ �� ����� ���������� ������� ������ ���� ������������� � ������ 
"������������ �������� �� 8 ���". ��������������� ��������� #PRAGMA ��� 
��������� ������������ �������� � ������ ����� krtapi.h.

������������ ������������ ������� �������������� ���������� ���������.
���������� ������� ���������� ���� �������� �������� ��� ������ �� ���������� 
������, ��� ��� ��������� ��������� ����� ������� � ����� ������ ��������� 
�������� ��� ������ ������ ��������. 
������� ������ ������������ ������������� � ��������������� ������ ��� ������� 
�������, ����������� ������������.
���� �������� ������� ������ ���� �������������� ������ ��� ������ 
������� ����������� ������� �� ��������, ���� ������� ���������� ����������� 
�������. 

������� krtCloseTrace ������������ ��� �������� �������, ����������� ���������� 
������������ � ������������ ��������, ��������������� ��� �������� �������.

*/

#ifdef _MSC_VER
/*
 * Disable following warnings when including Windows headers
 *
 * warning C4115: named type definition in parentheses
 * warning C4116: unnamed type definition in parentheses
 * warning C4201: nonstandard extension used : nameless struct/union
 * warning C4214: nonstandard extension used : bit field types other than int
 */
#pragma warning( disable : 4115 4116 4201 4214)
#endif

#include <windows.h>

#ifdef _MSC_VER
/*
 * Re-enable all
 */
#pragma warning( default : 4115 4116 4201 4214)
#endif

#include "krtBase.h"

#define KRT_APIVER      1     // ��� ��������� ������ 1 API

#define KRT_APIVER_3    3     // ��� ��������� ������ 3 API
                              // ���� ��������� API 3 � ���, ��� ����������� ���������� ������ ����������
                              // � ���� ����� (krotw32.dll) ��� ����������, �� �������� � ������ ��� ������ ����������


#define BYTE unsigned char    // �������� ��� ������
#define KRTDATA BYTE //long          // ��� ������������ ������ ���������
#define KRT_PALLETE_SIZE 200 //00  // ������ �������

// ��� ��������� ��� #KRT_APIVER_3
  // �������� ������ ������������ ��� ������
#pragma pack (1)
// �������������� �������������� ������ �� �������� � ���������
typedef struct {
    short sens;
    short color;
} T_DimensionPixel;
// ������������ � ������������ �� ���������
#pragma pack ()

// ��������� �������� ����� �������� ���������
typedef struct {      
 long num;      // ���-�� �������� � �����
 long type;     // ��� �������� (SENS_TYPE_*)
 long minValue; // ����������� �������� ��������� ������� (������� ���)
 long maxValue; // ������������ �������� ��������� ������� (������� ���)
 long isRow;     // 1/0 ���� 0 ����� ������ �� ��������������
 long maxRowVal; // ������������ �������� ����� ������ 
 long minRowVal; // ������������ �������� ����� ������ 
} T_SENSGROUP;

// ��������� �������� ���������������� �������
typedef struct {      
 long minValue; // ����������� �������� ��������� ������� (������� ���)
 long maxValue; // ������������ �������� ��������� ������� (������� ���)
 char name[EXT_SENS_NAME_LENGTH]; // �������� ������� 
} T_EXTSENS;

// ��������� �� ������� ������ ��������� �������� ���������.
// ���� NULL, ���������� � ��������� �� ��������������.
// ���������� KRT_OK ��� ������, KRT_ERR ��� ������.
typedef long (KRTAPI *drvFillData) (

  KRTHANDLE handle,      // ���������� �������

  long group,            // ������ ����� �������� ���������, �� �������� ����
                         // ������� ������.

  long start,            // ��������� ������� ������������ ��������� ���������
                         // �� ������.
                         // ������� �������� � �������� �������� ��� �������
                         // � ����������� �� ������ ������ ��������, ���������
                         // ��� ��������������

  long length,           // ����� ������������ ������� ���������
                         // �������� � �������� �������� ��� �������
                         // � ����������� �� ������ ������ ��������, ���������
                         // ��� ��������������

  KRTDATA *screen,       // ��������� �� ������, ������� ���������� ��������� 
                         // ����������� � �������� ���������. 
                         // ������� ������� ����� ���� BYTE, ������� �����
                         // ��������� �������� �� 0 �� KRT_PALLETE_SIZE - 1
                         // ������� ������ ������������� ��������� �������� 
                         // ������� � ��������� �������� �������� ����� 
                         // ���������� ������ � ���������� �������� 
                         // ������������� ���������� ���� � � ����, ��� 
                         // �������� ��������� ������ �������� ������������� 
                         // ��������� ���������� ����, ���������� ��������. 
                         // ���������� ��� ��������, ��� ��� ������ ������ 
                         // ��������� � ��������� ��������� �� ������ ���������
                         // ��������, ��������� � ���� ������ ������ ����������
                         // ���������� ���������. ��� ����� ��������� ������ 
                         // �� ������ ����������� ������ �������� ����������. 
                         // ���������� ����� ������� ����� ��������
                         // ���� num ��������� T_SENSGROUP, �����������
                         // ��������������� ���� �������� ���������
                         // ���������� �������� ������� ����� ����� ������� 
                         // ��������� (�������� length) ��������� �� ��������
                         // ���� stepSize ��������� T_SENSGROUP
                         // ������ ��������� ������ ����������� ��������� �
                         // ������ ����������� �������� ������� �� ������ �������

  long *rowData         // ��������� �� ������, ������� ���������� ��������� 
                         // ����������� � �������� ��������� � ���� ����� ������. 
                         // ���� ������ ���������� ����������� ������� screen, 
                         // �� ����������� ����������� �������� �������. 
                         // ������ rowData �������� �������� �������� (sizeof long)
//
//  long *orntData         // ��������� �� ������ ���������� ����������� ������,
//                         // ����������� ����������� � ������ ������� ������������
//                         // ������ � ������ ��������� �� ���� ������ ���������
);

//////////////////////////////////////////////////////////////////////////
// ����� �������� ����� ������ callback - �������
//////////////////////////////////////////////////////////////////////////

// ��������� �������� �������, ������������ ��� ������.
typedef struct {

 long sensGroups;    // ���-�� ������ �������� ���������
 long extSensors;    // ���-�� ��������������� ��������
 long askMetod;      // ����� ������ �������� (SENS_ASK_*)
 long stepSize;      // ������������ ������ �������� (��/�������)

 T_SENSGROUP *group; // ��������� �� ������ �������� �������� ������ 
                     // �������� ���������
                     // ������ ������� ����� ����� sensGroups

 T_EXTSENS *extSens; // ��������� �� ������ �������� �������� 
                     // ��������������� ��������
                     // ������ ������� ����� ����� extSensors

 // VOG �������
 long vog;           // 1 ��� 0

 // ������ ������� ������ �����
 long wallThickNum;  // ���������� �������
 long wallThickMin;  // ����������� �������� ������ 
 long wallThickMax;  // ������������ �������� ������ 

 // ������� (����������� �������)
 long timerNum;      // ����������   
 long timerStep;     // ������������ ��������� (��������)

 // �������� (��������� ���������)
 long odoNum;        // ����������   
 long odoStep;       // ������������ ��������� (��)

 // ������� ���������� (���� ������� ����.������� ��� �� 0 �� 360)
 long orientNum;     // ����������
 long orientStep;    // ������������ ��������� (��/������� - �� ������ ������)

 // ������� ��������� (������� ���)
 long shakeNum;      // ����������
 long shakeStep;     // ������������ ��������� (��/������� - �� ������ ������)

 // ������� �������� (���???)
 long pressNum;      // ����������                                            
 long pressStep;     // ������������ ��������� (��/������� - �� ������ ������)
                     
 // ������� ����������� (������� �������)
 long tempNum;       // ����������                                            
 long tempStep;      // ������������ ��������� (��/������� - �� ������ ������)

 // ������� ������� ��������� (������� ���)
 long angleNum;      // ����������                                            
 long angleStep;     // ������������ ��������� (��/������� - �� ������ ������)

 // ��������!!! ����, ������������ ������������ ��������� ���.�������� (timerStep, odoStep � �.�.) 
 // ������������ ��� �������, ����� ����� ���� ������� ������������� �������� �� �������� ���������.
 // ��� ���� �� ������ �� ���-�� ������� ������� krtDrvGetNextNode. ������ ������� ���������� ��
 // ��� ���, ���� �������� ���� Pos ��������� T_NODE �� �������� �� ��������� ������������� 
 // ���������.

 // Callback - �������
 drvErrorFunc           krtDrvError;
 drvGetFirstNode        krtDrvGetFirstNode;
 drvGetNextNode         krtDrvGetNextNode;
 drvGetFirstNodeGroup   krtDrvGetFirstNodeGroup;
 drvGetNextNodeGroup    krtDrvGetNextNodeGroup;
 drvFillData            krtDrvFillData;

 // ��������� ������

 long length;            // ����� ������ � ��
 long time;              // ����������������� ������� � ��������

 long speedMin;          // ����������� �������� ������� �� ������ � ��/���
 long speedMax;          // ������������ �������� ������� �� ������ � ��/���

 long shakeMin;          // ����������� �������� �������� ���������
 long shakeMax;          // ������������ �������� �������� ���������

 long pressMin;          // ����������� �������� �������� ��������
 long pressMax;          // ������������ �������� �������� ��������

 long tempMin;            // ����������� �������� �������� �����������
 long tempMax;            // ������������ �������� �������� �����������

 long angleMin;           // ����������� �������� �������� ������� ���������
 long angleMax;           // ����������� �������� �������� ������� ���������

} T_TRACEINFO;

//////////////////////////////////////////////////////////////////////////
// ���� ������ ���������� �������, �������������� ���������
//////////////////////////////////////////////////////////////////////////

// ������� ��������� ������ ��� ������ � ��������� ����������� � ��� ��������� inf.
// ���������� KRT_OK ��� ������, KRT_ERR ��� ������.
typedef long (KRTAPI *drv_OpenTrace)  (

// ������ ������� ���������� � ��� �����. ��� ������ ������
// � ��������� draft ���������� �������� 1 � ��� ���� ��������� T_TRACEINFO
// ���������������� ������.
// ��� ���� ������ ������ ������� ������ ��������� ��������� ����
// ��������� T_TRACEINFO:

// � ���� sensGroups ������� ���������� ���������� � ���������� ������ �������� ���������. 

// � ���� extSensors ������� ���������� ���������� � ���������� ���������������
// �������� �� ������ ���� �������.

// � ���� krtDrvError ������� ���������� ����� �������, ������������ ��������� 
// ������ �������� ��������� ������ ��� ������ � ��������, �������� ������������.
// ��� ����������, ����� ������� ��� ������� ������� ������, ��������� ��� 
// �������� �������.

// ����� ����� ������� ������ ��������� ���������, ����������� �� ������������
// ��������� ����� sensGroups � extSensors, �������� ����������� �� �������
// ������� ������, ����� ���������� �������� ���� ������ �������� ��������� �
// ���� ��������������� �������� � ���������� ��������� �� ��� ������� ������
// � ���� group � extSens ��������������.

// ����� ����� ��������� ��������� ������ ������ ����� ������� krtOpenTrace.
// �������� draft ��� ���� ����� 0, � � ��������� T_TRACEINFO ������� ���������
// ������������������ ��������� �� �������, ������� ����� ����� ������ ������
// � ������ ���������. �� ����� ����� ������� ������ ������� ��������� ���
// ��������� ������� � �������������� ��������� �������.

 const char * traceFile, // ��� trc-�����, ���������� ��� ����������� 
                         // �������. � ������ DRIVER_DATA ������� �����
                         // ������ ����������� ��� ����������� ����������
                         // ��� ������ � �������� (�������������� �����/��
                         // ������, ���������� �� ������� � �.�.)

 KRTHANDLE handle,       // ���������� �������. � ������ ��������������
                         // ����������� ����� ������� KRT_ERR

 T_TRACEINFO *inf,       // ��������� �� ��������� T_TRACEINFO, ������� 
                         // ���������� ��������� ����������� � �������.

 long draft              // ������� �������/������� ������ (1/0)

);

// ������� ��������� ������ � ����������� �������, ������� ��� ��� ��������.
// ���������� KRT_OK ��� ������, KRT_ERR ��� ������.
typedef long (KRTAPI *drv_CloseTrace) (

  KRTHANDLE handle       // ���������� �������

);

//////////////////////////////////////////////////////////////////////////
// ������� �������������� ���������

long EXPORT KRTAPI krtOpenTrace  (
 const char * traceFile,  
 KRTHANDLE handle,       
 T_TRACEINFO *inf,        
 long draft              
);

long EXPORT KRTAPI krtCloseTrace (
  KRTHANDLE handle
);

#endif