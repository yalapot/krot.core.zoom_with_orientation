#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_CHANGE_PARAMETER_OF_FILTER_DOWN  0
#define FLAG_CHANGE_PARAMETER_OF_FILTER_UP    1
extern long flag_change_parameters_filter;

///////// ����������� �� krtapi.h ������ 1

// ��������� �������� ������������� �������
typedef struct {      
 long num;           // ����������
 long step;          // ������������ ��������� (��/������� - �� ������ ������)
 long min;           // ����������� �������� (����������� ���������� T_NODEGROUP)
 long max;           // ������������ �������� (����������� ���������� T_NODEGROUP)
} T_SENS;

///////// ����� ����������� �� krtapi.h ������ 1

#include "krtapi.h"
#include "krtapi2.h"
#include "dai.h"
#include "vbapi.h"
#include "pallete.h"
#include "driver.h"

// ��������� �������� �������� ����������� ��������� (����)
typedef struct {

 long            xStart;           // ������ ������� ���� �� X � ��
 short           yStart;           // ����� ������� ���� �� Y � ��������
 short           xSizeDat;         // �����. ������ ���� � ���������� ��������
 short           ySizeDat;         // ������. ������ ���� � ���������� ��������
 long            xSizePix;         // �����. ������ ���� � ��������
 long            ySizePix;         // ������. ������ ���� � ��������
 long            ySizePixPage;     // ������������ ������ �������� � ��������

 long            drawType;         // ��� ���������

 long            maxDat;           // ������ ������ ������ � KRTDATA.
 KRTDATA        *dat0Buffer;       // ��������� �� ����� ������������ ��������� �������� ��� ����
 long           *datOrnt;          // ��������� �� ������ ����������
 KRTROW         *datBuffer;        // ��������� �� ����� ����� ��������� �������� ��� ����

 void           *bmpBuffer;        // ��������� �� ����� �������
 long            maxBmp;           // ������ ������ ������� � ��������.
 HBITMAP         bmp;              // ������ ��� ����

 int             filterMode;       // ��� �������� krotPaintZoom

} T_ZOOM;

// ��������� �������� �������� ������� ����������� ���������
typedef struct {

 int             delta;            // ��������
 long            mode;             // �����
 KRTROW          value;            // �������� �������

} T_SENSOR;

// ��������� ������� ����
typedef struct {

 long            lastPos;          // ��������� ������ ������ �������
 long            bufSize;          // ������� ������ ������� �� X � ����������
 KRTDATA        *dat;              // ��������� �� ����� ������������ ������
 KRTROW         *row;              // ��������� �� ����� ����� ������

} T_WELDSCAN;

// ��������� �������� ����� �������� ����������� ���������
typedef struct {

 long            index;            // ������ ����� 
 long            needRedraw;       // ���� ������������� ������������ �������
 long            topSens;          // ������� ��������� (����� �������� �����.�������)
 T_WELDSCAN      scan;             // ��������� ������ ������� ����
 VB_PAINT_INFO   vbScreen;         // ��������� �������� ������ ����������� ��������� �������� ������
 VB_GRAPH_INFO   vbGraphs;         // ��������� �������� ������ ����������� ��������� ���������
 VB_FILTER_INFO  vbFilter;         // ��������� �������� �������� ��� ����������� ��������� �������� ������
 HWND            pic;              // ����� pictureBox ���������
 long            drawMode;         // ����� ������ (����/�������)
 T_PAL           pal;              // �������
 T_ZOOM          zoom;             // ����
 T_SENSOR       *sensor;           // ��������� �� ������ �������� ��������
 long            hide;             // ���-�� ����������� ��������
 void           *bmpBuff;          // ��������� �� ����� �������
 void           *bmpBuffScroll;    // ��������� �� ����� ������������� ���������� �������
 HBITMAP         bmp;              // ������ ��� ��������� �������� �������

 // ��������� ������ ������� ������ ���� ������ ����, ����� ��������� ������ ���
 // ������������� ������ ��� ���� ������� (��. ����������� � ������� changeScreenMode)
 KRTROW         *datScrlBuff;      // ��������� �� ����� ����� ������ ��� ������
 long           *datScrlOrnt;      // ��������� �� ������ ���������� ���������� ������
 KRTROW         *datPageBuff;      // ��������� �� ����� ����� ������ ��� ��������
 long           *datPageOrnt;      // ��������� �� ������ ���������� ����������� ������
 KRTDATA        *dat0ScrlBuff;     // ��������� �� ����� ������������ ������ ��� ������
 KRTDATA        *dat0PageBuff;     // ��������� �� ����� ������������ ������ ��� ��������


 // ��� ��������� ��� #KRT_APIVER_3
 T_DimensionPixel *bendingPlanePageBuff;  // ��������� �� ��������� ������ ����������� ������
 T_DimensionPixel *bendingPlaneScrlBuff;  // ��������� �� ��������� ������ ���������� ������
                                   // ������ � ��������� ������ // (bending plane)
                                   // ������ � ���������� ��������� ������ ����� ������������ ��� ������ ��������
                                   // typedef struct {   
                                   //     short sens;     // ������ ����� ������� �������� ��������� ������ �� ������ ���������
                                   //     short color;    // ���� ��������� (����� >500 D = ���, ����� <500 D = 120, ����� <250 D = 200) 
                                   // } T_DimensionPixel;

 // ��� ��������� ��� #KRT_APIVER_3
 T_DimensionPixel *odomSensPageBuff;  // ��������� �� ������ � ���������� �������� ����������� ������
 T_DimensionPixel *odomSensScrlBuff;  // ��������� �� ������ � ���������� �������� ���������� ������ 
                                   // ������ � ���������� �������� ����� ������������ ��� ������ ��������
                                   // typedef struct {   
                                   //     short sens;     // ������ �� �������� �������� ���������� �������� �� ������ ���������
                                   //     short color;    // ���� ��������� (0 = ���, 115 - ������ ����., 152 = ����. ����.) 
                                   // } T_DimensionPixel;

 long Amplification_dflt;          // �������� �������� �� trc-����� ��� ������� "�� ���������"

 long            oldPos;           // ���������� ��������� �� ������

 // ��������� ��� ���� ������ ������������ �������� �������, ����������� � �������
 // �������� ������ ������. ��� ���������� ��� �������� ������� � �����������������
 // ������� ������� � ������� ����������.
 long            maxDatPage;       // ������������ ������ �������� ������ �� X
 long            maxBmpSize;       // ����������� ������ ������ ������� � ��������

 // ��������� ���� ��������������� �������� changeScreenMode ������ �� ��������
 // �� ���������� vbScreen->hWnd (������ ���� �� X � ��������), vbScreen->xSize � �.�.
 long            pageDat;          // ������ ������������ �������� ������ �� X � ���������� ��������
 long            pixelX;           // ������ ���� ��������� �� X � ��������
 long            pixelY;           // ������ ���� ��������� �� Y � ��������

 // ��� �������� ������� ����� � ���� �����.�������� ����������
 // �� T_TRACEINFO ���� � openTrace
 long            step;             // ��� ������ �����. ��������
 long            sNum;             // ���-�� �����. ��������
 long            sType;            // ��� �������� ����� SENS_TYPE_* �� krtBase.h

#define ANALOG_SENS  1
#define ENCODER_SENS 2
#define MAX_CALIBR_SENS_NUM 128
#define MAX_CALIBR_LEVEL    128

 // ������������� ������ ���� ���������� �������
 char   calibrate_date[16];
 char   id_device[24];
 long   profil_sens_type;  // Encoder ��� Analog
 double length_sens;       // ������ ����������� ������� � �� (�� ��� �������� �� ��� ��������������� ������)
 double zero_angle_gradus; // ���� ����� ���� ������� � ������� ������� � ������� ������� ��������� � ��������
 long profil_sens_quantity;
 long profil_calibrate[MAX_CALIBR_SENS_NUM][MAX_CALIBR_LEVEL];
 long profil_row_inverse;  // �������� ������ ������� �������, ��� ����������� ��������������� ������ ����
                           // �������� ����� MAX_DATA_CODE �� �������� ��������� ������ ��� ��������, ����� 0
} T_CRZSENS;

// ��������� �������� �������
typedef struct {

 KRTHANDLE       vbHandle;         // ���������� �������
 T_DRIVER        drv;              // ��������� �������� �������� �������
 T_TRACEINFO     record;           // ��������� �������� ���������� �������
 T_CRZSENS      *crz;              // ��������� �� ������ ��������,
                                   // ����������� ����� �������� ����������� ���������
 long            lockNodeSeq;      // ���� ���������� ������� *FirstNode* ��� �������
 long            isOnLine;         // ���� ������ ������������ �������� ������
 long            scaleX;           // ������� �� X

 long            Orientation_OFF;  // �������� ����� ���������� � trc-�����
} T_TRACE;

// ������ �������� ��������
typedef struct T_TRACELIST {
 T_TRACE trace;
 struct T_TRACELIST *next;
 struct T_TRACELIST *prev;
} T_TRACELIST;

/* ���������� ���������� ���������� */
extern char lastError[];
extern long bytesPerPixel;
extern long bitsPerPixel;

/* ���������� ��������� ������� krotw32.c */
extern T_TRACE *TraceList (KRTHANDLE Handle);
extern const char *driverError(T_TRACE *trc);

extern void closeZoom(T_ZOOM *zoom);
extern void initZoom(T_ZOOM *zoom);

extern void krotStretchBlt (
  T_CRZSENS *crz,
  KRTDATA *data,
  long *ornt,
  long datX,
  long datY,
  void *bmpBuff,
  void *bmpBuffGrafh,
  HWND pic,
  HBITMAP bmp,
  long bmpX,
  long bmpY,
  long bmpXstart,
  long bmpXend,
  long top,
  T_PAL *pal, 
  int smooth
);

extern short readData(
 T_TRACE *trc,
 long crz,
 long dStart,
 long dLength,
 KRTDATA *buf,
 KRTROW *row,

 // ��� ��������� ��� #KRT_APIVER_3
 long *orient,                   // ��������� �� ������ ����������
 T_DimensionPixel *bendingPlane, // ��������� �� ������ � ��������� ������
 T_DimensionPixel *odomSens      // ��������� �� ������ � ���������� ��������
);

extern void degree2sens(
 long *ornt,          // ������ ����������
 long vLen,           // ����� �������
 long sensNum         // ����� ���-�� �������� 
 );


// use orientation for bufers 
// ��� ��������� ��� #KRT_APIVER_3
void use_orientation_for_bufers (
   KRTDATA *data,                    // ��������� �� ������ ������������ ������ �������� ���������
   KRTROW *rowData,                  // ��������� �� ����� ����� ������ ��� ��������
   long *dataOrnt,                   // ��������� �� ������ ���������� ����������� ������
   long length,                      // ������ ������� �� X
   long sens_num,                    // ������ ������� �� Y (���������� ��������)
   T_TRACE *trc,                     // ���������� �� �������� ������
   T_DimensionPixel *bendingPlane,   // ��������� �� ������ � ��������� ������
   T_DimensionPixel *odomSens        // ��������� �� ������ � ���������� ��������
);


short allocMem(long bufSize, void **bufAddr, const char *msg);
short makeDatBuff(T_CRZSENS *crz, long xSize, char *callStack);
short makeBmpBuff(T_CRZSENS *crz, long pixelX, long pixelY);
