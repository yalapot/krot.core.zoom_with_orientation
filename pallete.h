// Файл описания структур и функций работы с палитрой. 
// Требует файлов vbapi.h, krtapi.h, windows.h


// структура для данных палитры из kpl файла
typedef struct {
 VB_PAL       head;             // заголовок
 VB_PAL_ITEM *Item;             // массив элементов
 long         maxItemNum;       // размер буфера Item (кол-во элементов VB_PAL_ITEM)
 long         changed;          // флаг обновления данных
} T_PAL_CORE;

// структура описания окна отображения палитры
typedef struct {
 long         X;                // размер окна отрисовки палитры в пикселах по X
 long         Y;                // размер окна отрисовки палитры в пикселах по Y
 long         maxX;             // размер буфера dataVect (кол-во пикселов)
 void        *dataVect;         // буфер битмапа палитры для данного размера окна
 HBITMAP      bmp;              // битмап палитры
} T_PAL_WIN;

// структура описания диапазона значений данных со значениями цветов на границах диапазона
typedef struct {
 VB_PAL_ITEM  min;              // левая граница
 VB_PAL_ITEM  max;              // правая граница
} T_PAL_DIAP;

// структура описания палитры целиком
typedef struct {
 T_PAL_CORE   core;             // данные палитры
 T_PAL_WIN    pic;              // окно отрисовки палитры
 T_PAL_DIAP   border;           // верхняя и нижняя границы и цвета отображаемых значений
 T_PAL_DIAP  *color;            // указатель на массив диапазонов данных с граничными цветами
 long         usrMin;           // нижняя граница цвета, установленная пользователем
 long         usrMax;           // верхняя граница цвета, установленная пользователем
} T_PAL;


// декларации переменных обслуживания операций с палитрой
extern long bytesPerPixel;      // кол-во байт на пиксел в зависимости от видеорежима
extern long bitsPerPixel;       // кол-во бит на пиксел в зависимости от видеорежима

// декларации функций обслуживания операций с палитрой
extern INLINE void putPixel(void *buff, long index, long val, T_PAL *pal);
extern void palClose(T_PAL *pal);
extern short palInit(T_PAL *pal);
