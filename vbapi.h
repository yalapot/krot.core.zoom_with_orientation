#ifndef _VBAPI_H_
#define _VBAPI_H_
/*
Файл описания интерфейса, предоставляемого ядром вторичной программы krotw32.dll 
(C) Vitaly Bogomolov 2001-2007.

Данная библиотека представляет собой набор функций (в дальнейшем ядро), при помощи которых можно запрашивать данные
у драйверов формата, поддерживающих интерфейс krtapi версий 1 и 2, и отображать эти данные в окнах win32 GUI.
Кроме этого в библиотеке имеется набор вспомогательных сервисных функций.

Все функции ядра возвращают одно из двух значений: 
KRT_OK в случае успешного завершения работы и KRT_ERR в случае, если произошла ошибка.
Обе эти константы определены в файле krtapi.h.
Подробное описание последней возникшей ошибки можно получить, в строке outString длиной KRT_TEXT (krtapi.h) 
вызвав функцию 

long EXPORT KRTAPI krotError (LPSTR outString);

До начала работы с записью можно воспользоваться двумя сервисными функциями:

krotCheckDriver - проверяет, является ли dll, указанная в аргументе driverFileName корректным драйвером krtapi и
заполняет информацией об этом драйвере остальные аргументы.

krotRegisterTrace - проводит процесс регистрации первичных данных записи primFile при помощи драйвера driverFileName.
В процессе регистрации создается trc файл, который в дальнейшем можно использовать для работы.

Обычный процесс работы с записью можно описать следующим образом:

Получить от пользователя имя trc файла fileName, из которого будут запрашиваться данные.
Сформировать в своей программе уникальный дескриптор сеанса Handle. 
Это может быть, например, число 1.
Получить из ini-файла fileName (секция [Trace], ключ Index) 
имя каталога indxFolder с индексными файлами записи.

Выделить три строковых буфера (outDrvSign, outDrvName, outDrvCopyRt) длиной KRT_TEXT (krtapi.h)
Создать экземпляр структуры VB_TRACE_INFO (vbapi.h)
Вызвать функцию 

short EXPORT KRTAPI krotOpenTrace (
 KRTHANDLE Handle, 
 LPSTR fileName, 
 LPSTR indxFolder, 
 LPSTR driverFileName, 
 LPSTR outDrvSign,     // сигнатура драйвера
 LPSTR outDrvName,     // описание драйвера
 LPSTR outDrvCopyRt,   // строка копирайта
 VB_TRACE_INFO *inf);

После успешного вызова krotOpenTrace поле inf->crzZoneNum содержит количество поясов датчиков
корозионной развертки. Первый пояс (индекс 0) является основным поясом, остальные вспомогательными.
Необходимо выбрать номер пояса crzIndex (от 0 до inf->crzZoneNum - 1), 
для которого будут запрашиваться данные.
Создать экземпляр структуры T_SENSGROUP (krtapi.h) и вызвать функцию krotExtCorozInfo.
После успешного вызова krotExtCorozInfo поле cinf->crz.num содержит количество датчиков в поясе, 
а поле cinf->crz.step содержит размер шага опроса по X в мм.

Если в записи имеются доп.датчики, то после вызова krotOpenTrace поле inf->extSensors будет содержать 
их количество. Получить информацию о них можно при помощи функции krotExtSensorInfo, передавая ей 
нужный номер доп.датчика.

Ядро может отрисовывать массив данных коррозионной развертки в окнах win32 GUI, используя механизм палитр.
Функции работы с палитрами более подробно описаны ниже в этом файле. Логика работы механизма палитр описана 
в соответствующем разделе Руководства пользователя программы 'Крот для Win32'. 

Прежде чем пытаться отобразить данные кор.развертки в окне, нужно убедиться, что текущий видеорежим системы
поддерживается ядром. Это можно сделать при помощи функции checkVmode.

Существует набор функций, которые управляют режимом отображения данных кор.развертки. 
Первый аргумент этих функций задает дескриптор открытой записи, второй - номер пояса кор.развертки (начиная с 0).
Эти функции включают в себя:

krotTopSens - задает величину вертикального циклического сдвига пояса развертки.

krotChangePic - задает вертикальный и горизонтальный размер окна отображения кор.данных по хендлу соответствующего окна.

krotPageScale - задает горизонтальный масштаб сжатия/растяжения данных при отображении.

krotScreenMode - задает доп.параметры отображения в режиме цветовой карты (положение базовой линии) и 
режиме графиков (прореживание, усиление, цвета графиков).

Собственно вывод кор.данных с дистанции start в окно осуществляется функцией krotPaint.
Отрисовка данных в окне линзы с соблюдением геометрических пропорций изображения выполняется при помощи
функции krotPaintZoom.

Следующая группа функций используется для запроса данных заданных датчиков из записи:

krotGetFirstNode и krotGetNextNode для получения данных одного датчика.
krotGetFirstNodeGroup и krotGetNextNodeGroup  для получения показаний группы датчиков.
Функция krotLockNodeSequence используется для переключения режима блокировки вызовов krotGetFirstNode* 
при длительных цепочках вызовов krotGetNextNode*.
krotGetSingleSens для получения массива показаний кор.датчика с текущей отображаемой страницы данных.
krotGetVectSens для получения массива показаний кор.датчиков на заданной дистанции (срез датчиков).

Следующая группа функций используется для анализа данных и взаимодействия с внешними программами:

krotSetClipboard - помещает содержимое заданного окна в виде растра в системный буфер обмена.
krotCorozDump - записывает в текстовый файл показания датчиков кор.развертки, отображаемые в окне в текущий момент.
krotScanWeld - ищет продольные и поперечные швы на заданном учатске данных.
krotDai - передает во внешнюю dai.dll фрагмент данных для поиска и анализа дефектов.
krotCorozData - помещает в указанный буфер заданный фрагмент кор.данных.

По завершении сеанса работы запись необходимо закрыть функцией krotCloseTrace.

*/

// режимы датчиков
#define SMODE_NORMAL      0
#define SMODE_FILTER      1
#define SMODE_DELETE      2

// типы изображения в линзе
#define ZOOM_NONE  -1
#define ZOOM_COMMON 0
#define ZOOM_SMOOTH 1
#define ZOOM_LINES  2
#define ZOOM_3D     3

// структура описания параметров окна отображения развертки в режиме цветовой карты
typedef struct {
 long orntOff;         // флаг отключения ориентации
 long baseLine;        // положение базовой линии для выравнивания
 long amplif;          // коэф-нт усиления
 long intwin;          // длина окна интегрирования
} VB_PAINT_INFO;
/*
Public Type VB_PAINT_INFO
 orntOff As Long       'флаг отключения ориентации
 baseLine As Long      'положение базовой линии для выравнивания
 amplif As Long        'коэф-нт усиления
 intwin As Long        'длина окна интегрирования
End Type
*/

// структура описания параметров окна отображения развертки в режиме цветовой карты
typedef struct {
 long active;          // тип текущего фильтра (0 - фильтр отключен)
 long unweldParam;     // параметр фильтра бесшовных труб           
 long rolledParam1;    // параметр1 фильтра цельнотянутых труб
 long rolledParam2;    // параметр2 фильтра цельнотянутых труб
 long rolledAmplifer;  // параметр усиления фильтра цельнотянутых труб
 long tfiParam1;       // параметр1 фильтра продольных трещин
 long tfiParam2;       // параметр2 фильтра продольных трещин
 long tfiBase;         // применять ли "базовую линию" в фильтре TFI (0/1)
} VB_FILTER_INFO;
/*
Public Type VB_FILTER_INFO
 active As Long        'тип текущего фильтра (0 - фильтр отключен)
 unweldParam As Long   'параметр фильтра бесшовных труб
 rolledParam1 As Long  'параметр1 фильтра цельнотянутых труб
 rolledParam2 As Long  'параметр2 фильтра цельнотянутых труб
 rolledAmplifer As Long  'параметр усиления фильтра цельнотянутых труб
 tfiParam1 As Long     'параметр1 фильтра продольных трещин
 tfiParam2 As Long     'параметр2 фильтра продольных трещин
 tfiBase As Long       'применять ли "базовую линию" в фильтре TFI (0/1)
End Type
*/

// структура описания параметров окна отображения развертки в режиме графиков
typedef struct {
 long amplif;          // коэф-нт усиления
 long gap;             // величина прореживания
 long clrBackGround;   // цвет фона
 long clrOdd;          // цвет нечетных графиков
 long clrEven;         // цвет четных графиков
} VB_GRAPH_INFO;
/*
Public Type VB_GRAPH_INFO
 amplif As Long        ' коэф-нт усиления
 gap As Long           ' величина прореживания
 clrBackGround As Long ' цвет фона
 clrOdd As Long        ' цвет нечетных графиков
 clrEven As Long       ' цвет четных графиков
End Type
*/

// структура для передачи из VB информации о характеристиках лупы для DAI
typedef struct {
 long orntStart;       // положение первой строки массива данных по окружности трубы в градусах (1-360)
 long orntLen;         // размер сектора по окружности трубы в градусах (1-360) занимаемый анализируемым фрагментом
 long itemX;           // протяженность элемента массива данных вдоль трубы в мм. (шаг опроса снаряда)
 long itemY;           // протяженность элемента массива данных по окружности трубы в мм. (расстояние между датчиками)
} VB_DAI_INFO;
/* 
Public Type VB_DAI_INFO
 orntStart As Long     'положение первой строки массива данных по окружности трубы в градусах (1-360)               
 orntLen As Long       'размер сектора по окружности трубы в градусах (1-360) занимаемый анализируемым фрагментом   
 itemX As Long         'протяженность элемента массива данных вдоль трубы в мм. (шаг опроса снаряда)                
 itemY As Long         'протяженность элемента массива данных по окружности трубы в мм. (расстояние между датчиками)
End Type
*/

// структура описания драйвера
typedef struct {
 long apiVer;          // версия API драйвера
 long isSpiral;        // внешнетрубник?
 long drvVerMax;       // номер версии драйвера, старший разряд
 long drvVerMin;       // номер версии драйвера, младший разряд
} VB_DRV_INFO;

/*
Public Type DRV_INFO 
 apiVer As Long        ' версия API драйвера
 isSpiral As Long      ' внешнетрубник?
 ' номер версии драйвера
 drvVerMax As Long     ' старший разряд
 drvVerMin As Long     ' младший разряд
End Type
*/

// структура для передачи в VB информации об открытом прогоне
typedef struct {

 long onLine;          // флаг функционирования динамического обновления данных

 long crzZoneNum;      // количество поясов короз.датчиков
 long extSensors;      // кол-во вспомогательных датчиков
 long VOG;             // функция VOG
 long evnt;            // поддержка событий

 T_SENS odom;          // одометры (разница .max - .min это длина трассы)
 T_SENS timer;         // таймеры (разница .max - .min это длительность прогона)

 T_SENS shake;         // ускорение
 T_SENS press;         // давление
 T_SENS temp;          // температура
 T_SENS angle;         // угловые скорости
 T_SENS wall;          // толщина стенки трубы

 T_SENS orient;        // ориентация
 T_SENS speed;         // скорость в см/сек

} VB_TRACE_INFO;
/*
Public Type VB_TRACE_INFO

 onLine As Long        ' флаг возможности динамического обновления данных

 crzZoneNum As Long    ' количество поясов короз.датчиков
 extSensors As Long    ' кол-во вспомогательных датчиков
 VOG As Long           ' функция VOG
 evnt As Long          ' поддержка событий

 odom As T_SENS        ' одометры (разница .max - .min это длина трассы)
 tmr As T_SENS         ' таймеры (разница .max - .min это длительность прогона)

 shake As T_SENS       ' ускорение
 press As T_SENS       ' давление
 temp As T_SENS        ' температура
 angle As T_SENS       ' угловые скорости
 wall As T_SENS        ' толщина стенки трубы

 orient As T_SENS      ' ориентация
 speed As T_SENS       ' скорость в см/сек

End Type
*/

/**************************************************************************/
/* обьявления внешних функций krotw32.dll используемых из VB              */
/**************************************************************************/

short EXPORT KRTAPI krotTopSens (
 KRTHANDLE Handle,
 long crzIndx,
 long topSens
);
/*
Public Declare Function krotTopSens Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal topSens As Long _
) As Integer
*/

short EXPORT KRTAPI CS_krotChangePic (
 KRTHANDLE Handle,
 long crzIndx,

 long pixelX,
 long pixelY
);


short EXPORT KRTAPI krotChangePic (
 KRTHANDLE Handle,
 long crzIndx,
 HWND hWnd
);
/*
Public Declare Function krotChangePic Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal hwnd As Long _
) As Integer
*/

short EXPORT KRTAPI krotPageScale (
 KRTHANDLE Handle,
 long scale
);
/*
Public Declare Function krotPageScale Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal scale As Long _
) As Integer
*/

short EXPORT KRTAPI krotScreenFilter (
 KRTHANDLE Handle, 
 long crzIndx, 
 VB_FILTER_INFO *vbFilter
);
/*
Public Declare Function krotScreenFilter Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByRef t_filter As VB_FILTER_INFO _
) As Integer
*/

short EXPORT KRTAPI krotScreenMode (
 KRTHANDLE Handle, 
 long crzIndx, 
 long *sens,         
 VB_PAINT_INFO *vbScreen,
 VB_GRAPH_INFO *vbGraphs
);
/*
Public Declare Function krotScreenMode Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByRef buf As Long, _
 ByRef t_paint As VB_PAINT_INFO, _
 ByRef t_grphs As VB_GRAPH_INFO _
) As Integer
*/

short EXPORT KRTAPI CS_krotPaint (
 KRTHANDLE Handle,
 long crzIndx,
 long start,

 long pixelX,
 long pixelY,
// LPSTR fileName,
 char * bmp_buf

);

short EXPORT KRTAPI krotPaint (
 KRTHANDLE Handle, 
 long crzIndx, 
 HWND hWnd,
 long start, 
 long forceReadData,
 long drawMode
);
/*
Public Declare Function krotPaint Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal hwnd As Long, _
 ByVal start As Long, _
 ByVal forceReadData As Long, _
 ByVal drawMode As Long _
) As Integer
*/

long EXPORT KRTAPI krotError (LPSTR outString);
/*
Public Declare Function krotError Lib "krotw32.dll" ( _
 ByVal lpReturnedString As String _
) As Long
*/

short EXPORT KRTAPI krotOpenTrace (
 KRTHANDLE Handle, 
 LPSTR fileName, 
 LPSTR indxFolder, 
 LPSTR driverFileName, 
 long (KRTAPI *newDataRegistered) (T_ArrivedData *newData),
 VB_TRACE_INFO *inf);
/*
Public Declare Function krotOpenTrace Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal FileName As String, _
 ByVal indxFolder As String, _
 ByVal driverFileName As String, _
 ByVal newDataRegistered As Long, _
 ByRef t_info As VB_TRACE_INFO _
) As Integer
*/

short EXPORT KRTAPI krotCloseTrace (KRTHANDLE Handle);
/*
Public Declare Function krotCloseTrace Lib "krotw32.dll" ( _
 ByVal handle As Long _
) As Integer
*/

short EXPORT KRTAPI checkVmode (void);
/*
Public Declare Function checkVmode Lib "krotw32.dll" () As Integer
*/

short EXPORT KRTAPI krotExtSensorInfo (
 KRTHANDLE Handle, 
 long index, 
 T_SENS *sens, 
 char *lpReturnedString
);
/*
Public Declare Function krotExtSensorInfo Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal index As Long, _
 ByRef sens As T_SENS, _
 ByVal lpReturnedString As String _
) As Integer
*/

short EXPORT KRTAPI krotExtCorozInfo (
 KRTHANDLE Handle, 
 long crzIndex, 
 T_SENSGROUP *inf
);
/*
Public Declare Function krotExtCorozInfo Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndex As Long, _
 ByRef c_info As T_SENSGROUP _
) As Integer
*/

short EXPORT KRTAPI krotGetFirstNode (
 KRTHANDLE handle,      // дескриптор прогона
 T_NODE *node,          // указатель на структуру T_NODE
 long start,            // позиция на трассе
 long sensType,         // тип датчика
 long sensIndex,        // индекс датчика (-1L - базовый)
 long length,           // длина участка для следующих вызовов krotGetNextNode
 long controlSize       // размер контрола отрисови в пикселах.
);
/*
Public Declare Function krotGetFirstNode Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByRef t_node As T_NODEDATA, _
 ByVal pos As Long, _
 ByVal sensType As Long, _
 ByVal sensIndx As Long, _
 ByVal seqLength As Long, _
 ByVal controlPixelsNum As Long _
) As Integer
*/

short EXPORT KRTAPI krotGetNextNode (
 KRTHANDLE handle,      // дескриптор прогона
 T_NODE *node           // указатель на структуру T_NODE
);
/*
Public Declare Function krotGetNextNode Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByRef t_node As T_NODEDATA _
) As Integer
*/

short EXPORT KRTAPI krotGetFirstNodeGroup (
 KRTHANDLE handle,      // дескриптор прогона
 T_NODEGROUP *node,     // указатель на структуру T_NODEGROUP
 long start,            // позиция на трассе
 long sensGroup,        // маска группы датчиков
 long length,           // длина участка для последующих вызовов GetNextNodeGroup
 long controlSize       // размер контрола в пикселах.
);
/*
Public Declare Function krotGetFirstNodeGroup Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByRef t_node As T_NODEDATAGROUP, _
 ByVal pos As Long, _
 ByVal sensGroup As Long, _
 ByVal seqLength As Long, _
 ByVal controlPixelsNum As Long _
) As Integer
*/

short EXPORT KRTAPI krotGetNextNodeGroup (
 KRTHANDLE handle,      // дескриптор прогона
 T_NODEGROUP *node      // указатель на структуру T_NODE
);
/*
Public Declare Function krotGetNextNodeGroup Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByRef t_node As T_NODEDATAGROUP _
) As Integer
*/

short EXPORT KRTAPI krotGetSingleSens (
 KRTHANDLE Handle,     // дескриптор прогона
 long      crzIndx, 
 long      sens,       // номер датчика
 KRTDATA  *bufDat,     // указатель на буфер
 KRTROW   *bufRow      // указатель на буфер сырых данных
);
/*
Public Declare Function krotGetSingleSens Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal sens As Long, _
 ByRef buf As Byte, _
 ByRef bufRow As Long _
) As Integer
*/

short EXPORT KRTAPI krotPaintZoom (
 KRTHANDLE Handle, 
 long crzIndx, 
 HWND hwnd,
 long xStart,
 short xDataSize,
 short yStart,
 short yDataSize, 
 short *dataBuff, 
 KRTROW  *rowBuff, 
 long drawType
);
/*
Public Declare Function krotPaintZoom Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal hWnd As Long, _
 ByVal xStart As Long, _
 ByVal xDataSize As Integer, _
 ByVal yStart As Integer, _
 ByVal yDataSize As Integer, _
 ByRef dllDat As Any, _
 ByRef dllRow As Any, _
 ByVal drawType As Long _
) As Integer
*/

short EXPORT KRTAPI krotGetVectSens (
 KRTHANDLE Handle,      // дескриптор прогона
 long crzIndx, 
 long      pos,         // позиция в мм.
 KRTDATA  *buf,         // указатель на буфер обработанных данных
 KRTROW   *row          // указатель на буфер сырых данных
);
/*
Public Declare Function krotGetVectSens Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal pos As Long, _
 ByRef buf As Byte, _
 ByRef bufRow As Long _
) As Integer
*/

short EXPORT KRTAPI krotSetClipboard (
  HWND      src          // hwnd источника данных
);
/*
Public Declare Function krotSetClipboard Lib "krotw32.dll" ( _
  ByVal hwnd As Long _
) As Integer
*/

short EXPORT KRTAPI krotRegisterTrace (
  LPSTR driverFileName, 
  LPSTR primFile, 
  LPSTR indxFolder,           
  LPSTR trcFile, 
  long (KRTAPI *informUser) (short percentDone, const char *msg)
);
/*
Public Declare Function krotRegisterTrace Lib "krotw32.dll" ( _
  ByVal driverFileName As String, _
  ByVal primFile As String, _
  ByVal indxFolder As String, _
  ByVal trcFile As String, _
  ByVal informUser As Long _
) As Integer
*/

short EXPORT KRTAPI krotCheckDriver (
 LPSTR driverFileName, 
 LPSTR outDrvSign,     // сигнатура драйвера
 LPSTR outDrvName,     // описание драйвера
 LPSTR outDrvCopyRt,   // строка копирайта
 LPSTR outFullDllPath, // полный путь на загруженную dll драйвера
 VB_DRV_INFO *drvInfo
);
/*
Public Declare Function krotCheckDriver Lib "krotw32.dll" ( _
 ByVal driverFileName As String, _
 ByVal outDrvSign As String, _
 ByVal outDrvName As String, _
 ByVal outDrvCopyRt As String, _
 ByRef d_info As VB_DRV_INFO _
) As Integer
*/

short EXPORT KRTAPI krotLockNodeSequence (
 KRTHANDLE Handle, 
 long lockActive
);
/*
Public Declare Function krotLockNodeSequence Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal lockActive As Long _
) As Integer
*/

short EXPORT KRTAPI krotEvent (
 KRTHANDLE Handle,
 T_EVENT *event
);
/*
Public Declare Function krotEvent Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByRef evnt As T_EVENT _
) As Integer
*/

short EXPORT KRTAPI krotCorozDump (
 KRTHANDLE Handle, 
 long crzIndx, 
 LPSTR outFileName, 
 long isRow
);
/*
Public Declare Function krotCorozDump Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal FileName As String, _
 ByVal isRow As Integer _
) As Integer
*/

short EXPORT KRTAPI krotZoomDump (
 KRTHANDLE Handle, 
 long crzIndx, 
 LPSTR outFileName, 
 long isRow
);
/*
Public Declare Function krotZoomDump Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal FileName As String, _
 ByVal isRow As Long _
) As Integer
*/

// структура описания параметров поиска шва
typedef struct {
 long scanStart;      // дистанция предыдущего шва
 long lenMin;         // минимальная длина трубы
 long lenMax;         // максимальная длина трубы

 long maskSize;       // размер маски в показаниях датчиков
 long listSize;       // ширина прокатного листа в показаниях датчиков
 long signalLevel;    // уровень сигнала
 short slitNum;       // ожидаемое количество продольных швов

 short weldSensitiv;  // уровень чувствительности поперечных швов (от 1 до 100)
 short slitSensitiv;  // уровень чувствительности продольных швов (от 1 до 100) 
                      // если 0, то продольные швы определять не надо.
 short spirSensitiv;  // то же для спиральных швов

 short noDRC;         // не может быть прямошовной
 short noSPR;         // не может быть спиралешовной
 short noWTO;         // не может быть бесшовной

 // параметры для алгоритма 2013

 short Weld1_width;   // 1 - 30
 float Weld1_sens;    // 1 - 10
 float Weld1_bord;    // 1 - 100
 short Weld1_otst;    // 10 - 30

 short Weld2_width;   // 1 - 30
 float Weld2_sens;    // 1 - 10
 float Weld2_bord;    // 1 - 100
 short Weld2_second;  // 1 - 15

 short Spiral_width;  // 1 - 30
 float Spiral_sens;   // 1 - 10
 float Spiral_bord;   // 1 - 100
 short Spiral_area;   // 30 - .lenMin

} VB_TUBE_SCAN_IN;
/*
Public Type VB_TUBE_SCAN_IN
 scanStart As Long    'дистанция предыдущего шва
 lenMin As Long       'минимальная длина трубы
 lenMax As Long       'максимальная длина трубы

 maskSize As Long         'размер маски в показаниях датчиков
 listSize As Long         'ширина прокатного листа в показаниях датчиков
 signalLevel As Long      'уровень сигнала
 slitNum As Integer       'ожидаемое количество продольных швов

 weldSensitiv As Integer  'уровень чувствительности поперечных швов (от 1 до 100)
 slitSensitiv As Integer  'уровень чувствительности продольных швов (от 1 до 100) 
                          'если 0, то продольные швы определять не надо.
 spirSensitiv As Integer  'то же для спиральных швов

 noDRC As Integer     'не может быть прямошовной
 noSPR As Integer     'не может быть спиралешовной
 noWTO As Integer     'не может быть бесшовной

 'параметры для алгоритма 2013
 Weld1_width As Integer
 Weld1_sens As Single
 Weld1_bord As Single
 Weld1_otst As Integer

 Weld2_width As Integer
 Weld2_sens As Single
 Weld2_bord As Single
 Weld2_second As Integer

 Spiral_width As Integer
 Spiral_sens As Single
 Spiral_bord As Single
 Spiral_area As Integer
End Type
*/

// структура описания параметров найденного шва
typedef struct {
 long wldDst;         // дистанция найденного шва
 short wldTyp;        // тип трубы (0-прямошовная, 1-спираль, 2-бесшовка)
 short wld1;          // номер датчика 1 продольного шва (-1 если нет)
 short wld2;          // номер датчика 2 продольного шва (-1 если нет)
 short crzIndex;      // номер пояса по которому определены продольные швы
} VB_TUBE_SCAN_OUT;
/*
Public Type VB_TUBE_SCAN_OUT
 wldDst As Long       'дистанция найденного шва
 wldTyp As Integer    'тип трубы (0-прямошовная, 1-спираль, 2-бесшовка)
 wld1 As Integer      'номер датчика 1 продольного шва (-1 если нет)
 wld2 As Integer      'номер датчика 2 продольного шва (-1 если нет)
 crzIndx As Integer   'пояс, по которому определены продольные швы
End Type
*/

short EXPORT KRTAPI krotScanWeld (
 KRTHANDLE Handle, 
 long crzIndx, 
 VB_TUBE_SCAN_IN *inpData, 
 VB_TUBE_SCAN_OUT *outData, 
 long is2013
);
/*
Public Declare Function krotScanWeld Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByRef prmIn As VB_TUBE_SCAN_IN, _
 ByRef prmOut As VB_TUBE_SCAN_OUT, _
 ByVal is2013 As Integer _
) As Integer
*/

short EXPORT KRTAPI krotDai (
 KRTHANDLE Handle, 
 long crzIndx, 
 T_USERDAI *udai, 
 VB_DAI_INFO *daiInfo, 
 LPSTR outString, 
 long (KRTAPI *informUser) (short percentDone)
);

/*
Public Const DAI_EXPLAIN_TEXT_MAX_LENGTH = 4096
Public Const DAI_QUEST_ERROR = -1

Public Type DAI_INFO
 internal As Long      ' 0 - наружный дефект или 1 - внутренний
 wallThickness As Long ' толщина стенки трубы в мм
 x1 As Long            ' номер колонки левого верхнего угла области
 y1 As Long            ' номер строки левого верхнего угла области
 x2 As Long            ' номер колонки правого нижнего угла области
 y2 As Long            ' номер строки правого нижнего угла области
End Type

Public Declare Function krotDai Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByRef udai As DAI_INFO, _
 ByRef daiInfo As VB_DAI_INFO, _
 ByVal lpReturnedString As String, _
 ByVal informUser As Long _
) As Integer
*/


/* 
Функция предназначена для внешних приложений, которым необходимо получить доступ 
к показаниям корозионных датчиков записи. 

Порядок работы следующий:

Подключить заголовочные файлы vbapi.h и krtapi.h, в которых определены необходимые
конствнты и структуры данных.
Загрузить krotw32.dll Все функции вызываются из этой библиотеки.
Получить от пользователя имя trc файла fileName, из которого будут запрашиваться данные.
Сформировать в своей программе уникальный дескриптор сеанса Handle. 
Это может быть, например, число 1.
Получить из ini-файла fileName (секция [Trace], ключ Index) 
имя каталога indxFolder с индексными файлами записи.

Выделить три строковых буфера (outDrvSign, outDrvName, outDrvCopyRt) длиной KRT_TEXT (krtapi.h)
Создать экземпляр структуры VB_TRACE_INFO (vbapi.h)
Вызвать функцию 

short EXPORT KRTAPI krotOpenTrace (
 KRTHANDLE Handle, 
 LPSTR fileName, 
 LPSTR indxFolder, 
 LPSTR driverFileName, 
 LPSTR outDrvSign,     // сигнатура драйвера
 LPSTR outDrvName,     // описание драйвера
 LPSTR outDrvCopyRt,   // строка копирайта
 VB_TRACE_INFO *inf);

В случае успешного вызова она вернет значение KRT_OK. (krtapi.h) 
Если вернет KRT_ERR (krtapi.h), то запись открыть не удалось. 

Описание ошибки можно получить в строке outString длиной KRT_TEXT (krtapi.h) вызвав функцию 

long EXPORT KRTAPI krotError (LPSTR outString);

После успешного вызова krotOpenTrace поле inf->crzZoneNum содержит количество поясов датчиков
корозионной развертки. Первый пояс (индекс 0) является основным поясом, остальные вспомогательными.
Необходимо выбрать номер пояса crzIndex (от 0 до inf->crzZoneNum - 1), 
для которого будут запрашиваться данные.
Создать экземпляр структуры T_SENSGROUP (krtapi.h) и вызвать функцию 

short EXPORT KRTAPI krotExtCorozInfo (
 KRTHANDLE Handle, 
 long crzIndx, 
 T_SENSGROUP *cinf
);

В случае успешного вызова она вернет значение KRT_OK. (krtapi.h) Обработку ошибки см. выше
После успешного вызова krotExtCorozInfo поле cinf->crz.num содержит количество датчиков в поясе, 
а поле cinf->crz.step содержит размер шага опроса по X в мм.

Вычислить размер буфера, необходимого для приема данных о фрагменте развертки длиной xLength мм.
cinf->crz.num * (xLength / cinf->crz.step) * sizeof(long)
Выделить буфер datBuff такого размера и вызвать функцию 

short EXPORT KRTAPI krotCorozData (
 KRTHANDLE Handle, 
 long crzIndx, 
 long xStart,     // Начало фрагмента, мм
 long xLength,    // Длина фрагмента, мм
 KRTDATA *datBuff,
 KRTROW *rowBuff
);

В случае успешного вызова она вернет значение KRT_OK. (krtapi.h) Обработку ошибки см. выше
После успешного вызова krotCorozData в datBuff находиться массив 32-битных показаний датчиков.

При завершении сеанса необходимо вызвать функцию закрытия записи 

short EXPORT KRTAPI krotCloseTrace (KRTHANDLE Handle);

*/
short EXPORT KRTAPI krotCorozData (
 KRTHANDLE Handle, 
 long crzIndx, 
 long xStart,     // мм
 long xLength,    // мм
 KRTDATA *datBuff,
 KRTROW *rowBuff
);

/*********************************************************
 Функции поддержки операций с палитрой
**********************************************************/

// составляющие полного кода цвета
#define RGB_RED      0  // красная 
#define RGB_GREEN    1  // зеленая
#define RGB_BLUE     2  // синяя

/*
 Функция возвращает одну из трех составляющих цвета colorCode.
 Какую составляющую вернуть задает аргумент rgbItem, который может
 принимать одно из трех значений RGB_* (см. выше)
*/
short EXPORT KRTAPI rgbColor (long colorCode, short rgbItem);
/*
'составляющие RGB цвета (возможные значения rgbItem)
Public Const RGB_RED = 0
Public Const RGB_GREEN = 1
Public Const RGB_BLUE = 2

Public Declare Function rgbColor Lib "krotw32.dll" ( _
 ByVal colorCode As Long, _
 ByVal rgbItem As Integer _
) As Integer
*/

/*
 Структура элемента палитры
*/
typedef struct {
 long iPos;                  // положение на векторе элементов
 long iVal;                     // значение полного кода цвета
} VB_PAL_ITEM;

/*
 Структура палитры пользователя
*/
typedef struct {
 long         itemNum;          // кол-во элементов в массиве item[]
 VB_PAL_ITEM  minItem;          // первый элемент палитры
 VB_PAL_ITEM  maxItem;          // последний элемент палитры
} VB_PAL;

// Функция инициализирует палитру для редактора палитр.
short EXPORT KRTAPI krtPalInit (void);
// Функция освобождает ресурсы из палитры для редактора палитр.
void EXPORT KRTAPI krtPalClose (void);

/*
 Функция устанавливает данные палитры для развертки crzIndx прогона Handle.
 Если Handle == -1, то данные устанавливаются для редактора палитр.
 аргумент item это указатель на массив элементов палитры между первым и последним.
 должны быть отсортированы по item->iPos по возрастанию.
 Возвращает KRT_OK при успехе, KRT_ERR при ошибке.
*/
short EXPORT KRTAPI krtPalSet (
 KRTHANDLE Handle, 
 long crzIndx, 
 long brdMin,
 long brdMax,
 long usrMin,
 long usrMax,
 VB_PAL *pal, 
 VB_PAL_ITEM *item
);

/*
 Функция рисует палитру развертки crzIndx прогона Handle в окне hwnd. 
 Если Handle == -1, то рисуется палитра редактора палитр.
 Возвращает KRT_OK при успехе, KRT_ERR при ошибке.
*/
short EXPORT KRTAPI krtPalDraw (
 KRTHANDLE Handle, 
 long crzIndx, 
 HWND    hwnd
);

/*
'Структура элемента палитры
Public Type VB_PAL_ITEM
 iPos As Long                   ' положение на векторе элементов
 iVal As Long                   ' значение полного кода цвета   
End Type

'Структура палитры пользователя
Public Type VB_PAL
 itemNum As Long                ' кол-во элементов в массиве по адресу item
 minItem As VB_PAL_ITEM         ' первый элемент палитры                                         
 maxItem As VB_PAL_ITEM         ' последний элемент палитры                                      
End Type

' Функция инициализирует палитру для редактора палитр.
Public Declare Sub krtPalInit Lib "krotw32.dll" ()
' Функция освобождает ресурсы из палитры для редактора палитр.
Public Declare Sub krtPalClose Lib "krotw32.dll" ()

' Функция устанавливает данные палитры для развертки crzIndx прогона Handle.
' Если Handle == -1, то данные устанавливаются для редактора палитр.
' аргумент item это указатель на массив элементов палитры между первым и последним.
' должны быть отсортированы по item->iPos по возрастанию.
' Возвращает KRT_OK при успехе, KRT_ERR при ошибке.
Public Declare Function krtPalSet Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal brdMin As Long, _
 ByVal brdMax As Long, _
 ByVal usrMin As Long, _
 ByVal usrMax As Long, _
 ByRef pal As VB_PAL, _
 ByRef palItem As Long _
) As Integer

' Функция рисует палитру для развертки crzIndx прогона Handle в окне hwnd. 
' Если Handle == -1, то рисуется палитра редактора палитр.
' Возвращает KRT_OK при успехе, KRT_ERR при ошибке.
Public Declare Function krtPalDraw Lib "krotw32.dll" ( _
 ByVal handle As Long, _
 ByVal crzIndx As Long, _
 ByVal hwnd As Long _
) As Integer

*/

/*
' Функция задает тип прореживания точек при изменении масштаба изображения
' i = 0 - выбирается максимальное значение
' i = 1 - выбирается минимальное значение
' i = 2 - выбирается среднее значение
' i = 3 - выбирается первое попавшееся значение
Public Declare Sub SetScaleMode Lib "krotw32.dll" (ByVal i As Integer)

*/
void EXPORT KRTAPI SetScaleMode (short i);

/*
функция возвращает тип используемого масштабирования
*/
short EXPORT KRTAPI GetScaleMode ();

#endif
