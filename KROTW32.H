#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_CHANGE_PARAMETER_OF_FILTER_DOWN  0
#define FLAG_CHANGE_PARAMETER_OF_FILTER_UP    1
extern long flag_change_parameters_filter;

///////// определения из krtapi.h версии 1

// структура описания произвольного датчика
typedef struct {      
 long num;           // количество
 long step;          // дискретность измерения (мм/милисек - от метода опроса)
 long min;           // минимальное значение (размерность аналогично T_NODEGROUP)
 long max;           // максимальное значение (размерность аналогично T_NODEGROUP)
} T_SENS;

///////// конец определений из krtapi.h версии 1

#include "krtapi.h"
#include "krtapi2.h"
#include "dai.h"
#include "vbapi.h"
#include "pallete.h"
#include "driver.h"

// структура описания текущего выделенного фрагмента (лупа)
typedef struct {

 long            xStart;           // начало участка лупы по X в мм
 short           yStart;           // конец участка лупы по Y в датчиках
 short           xSizeDat;         // гориз. размер лупы в показаниях датчиков
 short           ySizeDat;         // вертик. размер лупы в показаниях датчиков
 long            xSizePix;         // гориз. размер лупы в пикселах
 long            ySizePix;         // вертик. размер лупы в пикселах
 long            ySizePixPage;     // вертикальный размер страницы в пикселах

 long            drawType;         // тип отрисовки

 long            maxDat;           // размер буфера данных в KRTDATA.
 KRTDATA        *dat0Buffer;       // указатель на буфер обработанных показаний датчиков для лупы
 long           *datOrnt;          // указатель на вектор ориентации
 KRTROW         *datBuffer;        // указатель на буфер сырых показаний датчиков для лупы

 void           *bmpBuffer;        // указатель на буфер битмапа
 long            maxBmp;           // размер буфера битмапа в пикселах.
 HBITMAP         bmp;              // битмап для лупы

 int             filterMode;       // код возврата krotPaintZoom

} T_ZOOM;

// структура описания настроек датчика корозионной развертки
typedef struct {

 int             delta;            // поправка
 long            mode;             // режим
 KRTROW          value;            // значение датчика

} T_SENSOR;

// структура сканера швов
typedef struct {

 long            lastPos;          // последнее начало данных сканера
 long            bufSize;          // текущий размер буферов по X в измерениях
 KRTDATA        *dat;              // указатель на буфер обработанных данных
 KRTROW         *row;              // указатель на буфер сырых данных

} T_WELDSCAN;

// структура описания пояса датчиков корозионной развертки
typedef struct {

 long            index;            // индекс пояса 
 long            needRedraw;       // флаг необходимости пересоздания битмапа
 long            topSens;          // поворот развертки (номер верхнего логич.датчика)
 T_WELDSCAN      scan;             // структура данных сканера швов
 VB_PAINT_INFO   vbScreen;         // структура описания режима отображения развертки цветовой картой
 VB_GRAPH_INFO   vbGraphs;         // структура описания режима отображения развертки графиками
 VB_FILTER_INFO  vbFilter;         // структура описания фильтров для отображения развертки цветовой картой
 HWND            pic;              // хендл pictureBox развертки
 long            drawMode;         // режим вывода (цвет/графики)
 T_PAL           pal;              // палитра
 T_ZOOM          zoom;             // лупа
 T_SENSOR       *sensor;           // указатель на вектор настроек датчиков
 long            hide;             // кол-во отключенных датчиков
 void           *bmpBuff;          // указатель на буфер битмапа
 void           *bmpBuffScroll;    // указатель на буфер вертикального скроллинга битмапа
 HBITMAP         bmp;              // битмап для отрисовки страницы целиком

 // следующие четыре массива должны быть одного типа, иначе возникнет ошибка при
 // распределении памяти для этих буферов (см. комментарии в функции changeScreenMode)
 KRTROW         *datScrlBuff;      // указатель на буфер сырых данных для сдвига
 long           *datScrlOrnt;      // указатель на вектор ориентации сдвигового буфера
 KRTROW         *datPageBuff;      // указатель на буфер сырых данных для страницы
 long           *datPageOrnt;      // указатель на вектор ориентации страничного буфера
 KRTDATA        *dat0ScrlBuff;     // указатель на буфер обработанных данных для сдвига
 KRTDATA        *dat0PageBuff;     // указатель на буфер обработанных данных для страницы


 // это добавлено для #KRT_APIVER_3
 T_DimensionPixel *bendingPlanePageBuff;  // указатель на плоскость изгиба страничного буфера
 T_DimensionPixel *bendingPlaneScrlBuff;  // указатель на плоскость изгиба сдвигового буфера
                                   // данные о плоскость изгиба // (bending plane)
                                   // данные о положениии плоскости изгиба будут передаваться как массив структур
                                   // typedef struct {   
                                   //     short sens;     // датчик через который проходит плоскость изгиба на данном измерении
                                   //     short color;    // цвет отрисовки (изгиб >500 D = нет, изгиб <500 D = 120, изгиб <250 D = 200) 
                                   // } T_DimensionPixel;

 // это добавлено для #KRT_APIVER_3
 T_DimensionPixel *odomSensPageBuff;  // указатель на данные о траектории одометра страничного буфера
 T_DimensionPixel *odomSensScrlBuff;  // указатель на данные о траектории одометра сдвигового буфера 
                                   // данные о траектории одометра будут передаваться как массив структур
                                   // typedef struct {   
                                   //     short sens;     // датчик по которому проходит траектория одометра на данном измерении
                                   //     short color;    // цвет отрисовки (0 = нет, 115 - первый одом., 152 = втор. одом.) 
                                   // } T_DimensionPixel;

 long Amplification_dflt;          // Усиление читаемое из trc-файла для фильтра "по умолчанию"

 long            oldPos;           // предыдущее положение на трассе

 // следующие два поля хранят максимальные значения величин, запрошенных в течение
 // текущего сеанса работы. они необходимы для принятия решения о перераспределении
 // размера буферов в сторону увеличения.
 long            maxDatPage;       // максимальный размер страницы данных по X
 long            maxBmpSize;       // масимальный размер буфера битмапа в пикселах

 // следующие поля устанавливаются функцией changeScreenMode исходя из значений
 // ее параметров vbScreen->hWnd (размер окна по X в пикселах), vbScreen->xSize и т.д.
 long            pageDat;          // размер отображаемой страницы данных по X в показаниях датчиков
 long            pixelX;           // размер окна развертки по X в пикселах
 long            pixelY;           // размер окна развертки по Y в пикселах

 // для удобства доступа даные о шаге короз.датчиков копируются
 // из T_TRACEINFO сюда в openTrace
 long            step;             // шаг опроса короз. датчиков
 long            sNum;             // кол-во короз. датчиков
 long            sType;            // тип датчиков пояса SENS_TYPE_* из krtBase.h

#define ANALOG_SENS  1
#define ENCODER_SENS 2
#define MAX_CALIBR_SENS_NUM 128
#define MAX_CALIBR_LEVEL    128

 // калибровочные данные если профильные датчики
 char   calibrate_date[16];
 char   id_device[24];
 long   profil_sens_type;  // Encoder или Analog
 double length_sens;       // длинна энкодерного датчика в мм (от оси энкодера до оси полиуританового колеса)
 double zero_angle_gradus; // угол между осью снаряда и рычагом датчика в крайнем верхнем положении в градусах
 long profil_sens_quantity;
 long profil_calibrate[MAX_CALIBR_SENS_NUM][MAX_CALIBR_LEVEL];
 long profil_row_inverse;  // инверсия сырого сигнала профиля, для корректного автоматического поиска швов
                           // значение равно MAX_DATA_CODE из которого вычитался сигнал для инверсии, иначе 0
} T_CRZSENS;

// структура описания прогона
typedef struct {

 KRTHANDLE       vbHandle;         // дескриптор прогона
 T_DRIVER        drv;              // структура описания драйвера прогона
 T_TRACEINFO     record;           // структура описания параметров прогона
 T_CRZSENS      *crz;              // указатель на массив структур,
                                   // описывающих пояса датчиков корозионной развертки
 long            lockNodeSeq;      // флаг блокировки вызовов *FirstNode* для прогона
 long            isOnLine;         // флаг работы динамической подкачки данных
 long            scaleX;           // масштаб по X

 long            Orientation_OFF;  // значение флага ориентации в trc-файле
} T_TRACE;

// список открытых прогонов
typedef struct T_TRACELIST {
 T_TRACE trace;
 struct T_TRACELIST *next;
 struct T_TRACELIST *prev;
} T_TRACELIST;

/* обьявления глобальных переменных */
extern char lastError[];
extern long bytesPerPixel;
extern long bitsPerPixel;

/* обьявления сервисных функций krotw32.c */
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

 // это добавлено для #KRT_APIVER_3
 long *orient,                   // указатель на вектор ориентации
 T_DimensionPixel *bendingPlane, // указатель на данные о плоскости изгиба
 T_DimensionPixel *odomSens      // указатель на данные о траектории одометра
);

extern void degree2sens(
 long *ornt,          // вектор ориентации
 long vLen,           // длина вектора
 long sensNum         // общее кол-во датчиков 
 );


// use orientation for bufers 
// это добавлено для #KRT_APIVER_3
void use_orientation_for_bufers (
   KRTDATA *data,                    // указатель на массив обработанных данных страницы развертки
   KRTROW *rowData,                  // указатель на буфер сырых данных для страницы
   long *dataOrnt,                   // указатель на вектор ориентации страничного буфера
   long length,                      // размер массива по X
   long sens_num,                    // размер массива по Y (количество датчиков)
   T_TRACE *trc,                     // информация об открытой трассе
   T_DimensionPixel *bendingPlane,   // указатель на данные о плоскости изгиба
   T_DimensionPixel *odomSens        // указатель на данные о траектории одометра
);


short allocMem(long bufSize, void **bufAddr, const char *msg);
short makeDatBuff(T_CRZSENS *crz, long xSize, char *callStack);
short makeBmpBuff(T_CRZSENS *crz, long pixelX, long pixelY);
