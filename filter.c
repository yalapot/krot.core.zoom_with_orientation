#include "krotw32.h"
#include "filter.h"

#include <stdio.h>
#include <math.h>
#include "debuglog.h"


// эти функции взять из scan2013.c
extern double CalculateMean(double value[], long array_size);
extern double CalculateVariane(double value[], long array_size);
extern double GetStandardDeviation(double value[], long array_size);

// ***************************************************************
// функция одной итерации фильтра "Експоненциальное среднее"

#define EXP_AVRG_KOEFF 0.2
long Exponent_smooth (long value)
{
   static double Exponent_average;

   Exponent_average = value * EXP_AVRG_KOEFF + Exponent_average * (1-EXP_AVRG_KOEFF);

   return (long) Exponent_average;
}


long Exponent_average (
long value,                       // очередное значение в фильтруемом ряду
long ExpAvrgWaightingCoefficient, // весовой коэффициэнт фильтра (значения от 0 до 100)
long ReinitExponent_average       // проинициализировать среднее очередным значением (TRUE/FALSE)
)
{
   static double Exponent_average;
   double ExpAvrgWC_double = ExpAvrgWaightingCoefficient;
   double result;


   if (ReinitExponent_average)
   {
       result = Exponent_average = value;
   } else {
       ExpAvrgWC_double /= 100;
       result = value;
       result = result * ExpAvrgWC_double + Exponent_average * (1.0 - ExpAvrgWC_double);
       Exponent_average = result;
   }

   return (long) result;
}


int fltExponent (
KRTDATA* data,                    // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,                  // указатель на буфер сырых данных для страницы
long *dataOrnt,                   // указатель на вектор ориентации страничного буфера
long length,                      // размер массива по X
long sens_num,                    // размер массива по Y (количество датчиков)

//T_SENSOR *sensor       // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta

long ExpAvrgWaightingCoefficient, // весовой коэффициэнт фильтра (значения от 0 до 100)
long Amplifer                     // параметр усиления фильтра (значения от 1 до 100)
) {

long sens_counter;
long length_counter;
long tmp_long;
//long flag_viravnivanie = 0;

  (void) dataOrnt;

  { // проверка на изменение параметров фильтра
      static long length_pred = 0;
      static long sens_num_pred = 0;
      static long ExpAvrgWaightingCoefficient_pred = 0;
      static long Amplifer_pred = 1;

      if ( ( length != length_pred ) ||
           ( sens_num != sens_num_pred ) ||
           ( ExpAvrgWaightingCoefficient != ExpAvrgWaightingCoefficient_pred ) ||
           ( Amplifer != Amplifer_pred )
         ) flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

      length_pred = length;
      sens_num_pred = sens_num;
      ExpAvrgWaightingCoefficient_pred = ExpAvrgWaightingCoefficient;
      Amplifer_pred = Amplifer;
  }

  // отработка флага "выравнивание"
//  for (sens_counter = 0; sens_counter < sens_num; sens_counter ++) {
//     if (sensor[sens_counter].value != 0) {
//        flag_viravnivanie = 1;
//        break;
//     }
//  }

  for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
  {
     tmp_long = rowData[length * sens_counter + 0] - 
                 Exponent_average(rowData[length * sens_counter + 0],
                                  ExpAvrgWaightingCoefficient, TRUE);

     for (length_counter = 1; length_counter < length; length_counter++)
     { 
//         if (flag_viravnivanie != 0) // отработка флага "выравнивание"
//         {
//             tmp_long = rowData[length * sens_counter + length_counter] - sensor[sens_counter].value;
//         } else {
             tmp_long = rowData[length * sens_counter + length_counter] - 
                        Exponent_average(rowData[length * sens_counter + length_counter],
                                         ExpAvrgWaightingCoefficient, FALSE);
//         }

         tmp_long *= Amplifer;

         tmp_long = KRT_PALLETE_SIZE/2 + (tmp_long * KRT_PALLETE_SIZE) / 3943;

         if (tmp_long >= KRT_PALLETE_SIZE) tmp_long = KRT_PALLETE_SIZE-1;
         if (tmp_long < 0) tmp_long = 0;

         data[length * sens_counter + length_counter] =  (KRTDATA) tmp_long;
     }
  }

 return 0;
} // int fltExponent (


#define MAX_DATA_VALUE 4096
#define MAX_MEDIAN_FILTER_LENGTH 500

long Median_average (
long value,                     // очередное значение в фильтруемом ряду
long Win_Length,                // длинна окна
long ReinitMedian_average     // проинициализировать среднее очередным значением (TRUE/FALSE)
)
{
   static long src_data_bufer [MAX_MEDIAN_FILTER_LENGTH];
   static long cur_src_data_buf_len = 0;
   static long cur_pos_src_data_buf = 0;
   static long first_pos_src_data_buf = 0;


   static long gistograma [MAX_DATA_VALUE];

   long gist_counter;
   long max_gisttogram;
   long max_gist_pos = 0;

   if (Win_Length > MAX_MEDIAN_FILTER_LENGTH) Win_Length = MAX_MEDIAN_FILTER_LENGTH;

   if (ReinitMedian_average)
   {
       cur_src_data_buf_len = 0;  
       cur_pos_src_data_buf = 0;  
       first_pos_src_data_buf = 0;
       memset(gistograma, 0, MAX_DATA_VALUE * sizeof(gistograma [0]));
       memset(src_data_bufer, 0, MAX_MEDIAN_FILTER_LENGTH * sizeof(src_data_bufer [0]));
       max_gisttogram = 0;
   } else {
       if (cur_src_data_buf_len >= Win_Length)
       {
           gistograma[src_data_bufer[first_pos_src_data_buf]] --;
           src_data_bufer[first_pos_src_data_buf] = 0;
           first_pos_src_data_buf++;
           if (first_pos_src_data_buf >= Win_Length) first_pos_src_data_buf = 0;
           cur_src_data_buf_len--;
       }

       gistograma[value] ++;
       src_data_bufer[cur_pos_src_data_buf] = value;
       cur_pos_src_data_buf ++;
       if (cur_pos_src_data_buf >= Win_Length) cur_pos_src_data_buf = 0;
       cur_src_data_buf_len ++;

       max_gisttogram = 0;
       max_gist_pos = 0;
       for (gist_counter = 0; gist_counter < MAX_DATA_VALUE; gist_counter++)
       {
           if (max_gisttogram <= gistograma[gist_counter])
           {
               max_gisttogram = gistograma[gist_counter];
               max_gist_pos = gist_counter;
           }
       }
   }

   return max_gist_pos;
}

// ***************************************************************
// функция одной итерации фильтра "наблюдатель"

#define PI 3.141592653589793
#define Tp 0.2


long nablyudatel(double X, long period)
{
   static double W;
   static double Xf=0;
   static double A=0;
   static double B=0;
   static int i = 1;

   W = 2 * PI / period;


   Xf +=  Tp * ( X - Xf - A*sin(W*i) - B*cos(W*i) ) ;
   A  +=  Tp * sin(W*i)* (X - Xf - A*sin(W*i) - B*cos(W*i));
   B  +=  Tp * cos(W*i)* (X - Xf - A*sin(W*i) - B*cos(W*i));

   i += 1;
   return (long) Xf;
}

#define Fs 200.0


void Wavelet(long* src, long* dst, int length_src, long filterParam1, long filterParam2)
{
   double * RRR;
   double * WWW;
   double SSS, aaa, aaa0, aaa1, ttt;
   double Fs_sqrt2_pi;
   long ppp, iii, jjj, kkk, length_WWW, zzz;

   RRR = malloc(length_src * sizeof(RRR[0]));
   for (zzz = 0; zzz < length_src; zzz++) RRR[zzz] = 0;

   Fs_sqrt2_pi = 2.0 / (Fs * sqrt(2.0*PI));

   for (jjj = filterParam1; jjj <= filterParam2; jjj++)
   {
       ppp = 1;
       ttt = 0;

       for (iii = 1; iii <= jjj; iii++) ppp = ppp * 2;

       SSS = 2.0 * ppp / Fs;

       length_WWW =  2 * ppp + 1;
       WWW = malloc (sizeof(WWW[0]) * length_WWW);
       for (zzz = 0; zzz < length_WWW; zzz++) WWW[zzz] = 0;

       for (iii = 0; iii <= 2*ppp; iii++)
       {
            WWW[iii] = PI / 2.0 / SSS * sin(2.0 * PI * ttt / SSS);
            ttt = ttt + 1.0/Fs;
       }

       for (iii = 0; iii <= (length_src - length_WWW - 1); iii++)
       {
            aaa  = 0;
            aaa0 = 0;
            aaa1 = 0;

            for (kkk = 0; kkk <= (length_WWW -1); kkk++)
            {
                if ( kkk > 0)
                {
                    aaa1 = WWW[kkk] * src[iii + kkk];
                    aaa  = aaa + 0.5 * (aaa0 + aaa1)/Fs;
                    aaa0 = aaa1;
                }

            }

            for (kkk = 0; kkk <= (length_WWW -1); kkk++)
            {
                RRR[iii + kkk] = RRR[iii + kkk] + aaa * WWW[kkk] * Fs_sqrt2_pi;
            }
       }

       free(WWW);

   } // for (jjj = 2; jjj <= 5; jjj++)

   for (iii = 0; iii <= (length_src - 1); iii++)
   {
       dst[iii] = (long)(src[iii] - RRR[iii]);           // Отфильтрованный сигнал
   }

   free(RRR);
} // void Wavelet(int* src, int length_src)



// ***************************************************************
// функция фильтрации бесшовной трубы
int fltUnweld (
KRTDATA *data,         // указатель на массив обработанных данных страницы развертки
KRTROW *dataRow,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long x,                // размер массива по X
long y,                // размер массива по Y(количество датчиков)
long filterParam       // параметр фильтра
) {

    long pos_in_data;

    long *tmp_array;

    { // проверка на изменение параметров фильтра
       static long filterParam_pred = 0;

       if ( filterParam != filterParam_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       filterParam_pred = filterParam;
    }

    (void) dataRow;
    (void) dataOrnt;
    (void) data;

    tmp_array = malloc(x * y * sizeof(tmp_array[0]));

    for (pos_in_data=0; pos_in_data < (y * x); pos_in_data++ )
    {
         tmp_array[pos_in_data] = nablyudatel(dataRow[pos_in_data], filterParam);
    }

    //  скользящее среднее
    {
    long sens_counter;
    long length_counter;
    long length = x;
    long sens_num = y;
    long WinLen = 300;
    long startWin;
    long local_sum;
    long local_average;
    long result;
    long Amlification = 3;


    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {

        for (startWin = 0;
             startWin < length-WinLen;
             startWin++) 
        {
            local_sum = 0;
            for (length_counter = startWin;
                 length_counter < startWin + WinLen;
                 length_counter++) 
            {
                local_sum += tmp_array[length * sens_counter + length_counter];
            };

            local_average = local_sum / WinLen;


            // вычтем постоянную составляющюю
            length_counter = startWin + WinLen / 2;
            result = tmp_array[length * sens_counter + length_counter] - 
                     local_average;

            result *= Amlification;

            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;

            data[length * sens_counter + length_counter] = (KRTDATA) result;
        };
    };
    }
    // посчитали скользящее среднее

     free(tmp_array);

     return 0;
}

// ***************************************************************
// функция фильтрации цельнотянутой трубы
int fltRolled (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long filterParam1,     // параметр1 фильтра
long filterParam2,     // параметр2 фильтра
long rolledAmplifer    // параметр усиления фильтра цельнотянутых труб
) {

long *tmp_array;
long sens_counter, length_counter, tmp_long;


    { // проверка на изменение параметров фильтра
       static long filterParam1_pred   = 0;
       static long filterParam2_pred   = 0;
       static long rolledAmplifer_pred = 0;
       
       if ( ( filterParam1_pred != filterParam1 ) ||
            ( filterParam2_pred != filterParam2 ) ||
            ( rolledAmplifer_pred != rolledAmplifer )
          )
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

       filterParam1_pred = filterParam1;
       filterParam2_pred = filterParam2;
       rolledAmplifer_pred = rolledAmplifer;
    }


 (void) dataOrnt;

  tmp_array = malloc(length * sizeof(tmp_array[0]));

  for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
  {
     Wavelet ( &(rowData[length * sens_counter]), tmp_array, length, filterParam1, filterParam2);

     tmp_long = tmp_array[0] - Exponent_average(tmp_array[0], 20,TRUE );

     for (length_counter = 0; length_counter < length; length_counter++)
     {
         tmp_long = tmp_array[length_counter] - 
                    Exponent_average(tmp_array[length_counter], 20, FALSE);
         tmp_long *= rolledAmplifer;

         tmp_long = KRT_PALLETE_SIZE/2 + (tmp_long * KRT_PALLETE_SIZE) / 4096;

         if (tmp_long >= KRT_PALLETE_SIZE) tmp_long = KRT_PALLETE_SIZE-1;
         if (tmp_long < 0) tmp_long = 0;

         data[length * sens_counter + length_counter] =  (KRTDATA) tmp_long;
     }
  }

 free(tmp_array);

 return 0;
}

// ***************************************************************
// функция фильтрации пригрузов
// расчет условной нормальной составляющей            
int fltPrigruz (
KRTDATA *data,         // указатель на массив обработанных данных страницы развертки
KRTROW *dataRow,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long x,                // размер массива по X
long y                 // размер массива по Y(количество датчиков)
) {

 int i,j;

 (void) dataRow;
 (void) dataOrnt;

 for (i=0; i < x; i++) {
     for (j=0; j < y*x; j+=x) {
         if ( i == (x-1) ) {
             data[j+i] = (KRTDATA) (MIDLE_LEVEL);
         } else {
             data[j+i] = (KRTDATA) (data[j+i+1] - data[j+i] + MIDLE_LEVEL);
         }
     }
 }

 return 0;
}

// ***************************************************************
// функция расчета условной нормальной составляющей            
// (производной по X, убирает пригруза,патроны и прочую л...)
int fltDerivation (
KRTDATA* data,        // указатель на входной массив
int x,                // размер массива по X
int y,                // размер массива по Y(количество датчиков)
KRTDATA* out1,        // указатель на выходной массив (с погашением темных участков)
KRTDATA* out2         // указатель на выходной массив
) {
int i,j,n=0;

 for (i=0; i < x; i++) {
  for (j=0; j < y*x; j+=x) {
   if ( i == (x-1) ) {
    out1[n] = (KRTDATA) (MIDLE_LEVEL);
    out2[n] = (KRTDATA) (MIDLE_LEVEL);
   } else {
    out2[n] = (KRTDATA) (data[j+i+1] - data[j+i] + MIDLE_LEVEL);
    out1[n] = (KRTDATA) ((out2[n] < MIDLE_LEVEL) ? MIDLE_LEVEL : out2[n]);
   }
   n++;
  }
 }
 return n;
}

// ***************************************************************
// производная по Y, убирает сбои датчиков
void fltDerivationY (
KRTDATA* data,        // указатель на входной массив
int x,                // размер массива по X
int y,                // размер массива по Y(количество датчиков)
KRTDATA* out          // указатель на выходной массив (с погашением темных участков)
) {
int i,j,n=0;

 for (i=0; i < x; i++) {
  for (j=0; j < y*x; j+=x) {
   if(j == (y*x - x)) {
    out[n] = (KRTDATA) (MIDLE_LEVEL);
   } else {
    out[n] = (KRTDATA) (data[j+x+i] - data[j+i] + MIDLE_LEVEL);
   }
   n++;
  }
 }
}

// ***************************************************************
//фильтр Собела (Out=|Y|-|X|)
//использует окно (3*3)
/*
|-x4|-2x5|-x6|
--------------
|   | x* |   |
--------------
|+x1|+2x2|+x3|
x*=x1+2x2+x3-x4-2x5-x6
-------------------
| +y1|   |-y4 |
---------------
|+2y2| y*|-2y5|
---------------
| +y3|   |-y6 |
y*=y1+2y2+y3-y4-2y5-y6
___________________
Out=|y|-|x|
*/
void fltSobel(KRTDATA* In, int sizeX, int sizeY, KRTDATA* Out) {
int i,j,x,y,n,k,r;

 k=sizeY+1;
 n=sizeY+1;

 for(i=0; i <= sizeY * sizeX; i++) 
  Out[i]=0;

 for(i=0; i < sizeX * sizeY - sizeY * 2; i += sizeY) {
  for(j=sizeY + 1; j < 2 * sizeY - 2; j++) {
   x = In[i+j-sizeY+1]+2*In[i+j+1]+In[i+j+sizeY+1]-In[i+j-sizeY-1]-2*In[i+j-1]-In[i+j+sizeY-1];
   y = In[i+j-sizeY-1]+2*In[i+j-sizeY]+In[i+j-sizeY+1]-In[i+j+sizeY-1]-2*In[i+j+sizeY]-In[i+j+sizeY+1];
   r = abs(y) - abs(x);
   if ( r < 0 ) { r=0; }
   Out[n] = (KRTDATA) r;
   n++;
  }
  n+=3;
 }
}

long Base_line_delta[2048];

int Base_line_init(
 KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
 long length,           // размер массива по X
 long sens_num,         // размер массива по Y(количество датчиков)
 long shift_srez
)
{
    long sens_counter;

    Base_line_delta[0] = 0;
    for (sens_counter = 1; sens_counter < sens_num; sens_counter++)
    {
        Base_line_delta[sens_counter] = data[length * sens_counter + shift_srez] -
                                        data[length * 0            + shift_srez];
    }
    return 0;
};

int Base_line_calkulate(
 KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
 long length,           // размер массива по X
 long sens_num         // размер массива по Y(количество датчиков)
)
{
    long sens_counter;
    long length_counter;

    for (length_counter = 0; length_counter < length; length_counter++)
    {
        for ( sens_counter = 1; sens_counter < sens_num; sens_counter++)
        {
            data[length * sens_counter + length_counter] = data[length * sens_counter + length_counter]
                                                           + (KRTDATA) Base_line_delta[sens_counter];
        };
    };

    return 0;
};

// ***************************************************************
// функция просмотра без математики
int fltNo_math (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor       // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
)
{
    long sens_counter;
    long length_counter;
    long result;

    (void) dataOrnt;


    for (length_counter = 0; length_counter < length; length_counter++)
    { 
         for (sens_counter = 0; sens_counter < sens_num; sens_counter ++) 
         {
             result =  rowData[length * sens_counter + length_counter];

             result = (result * KRT_PALLETE_SIZE) / 4096;

             // поправка на базовую линию
             result += sensor[sens_counter].delta;

             if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
             if (result < 0 ) result = 0;

             data[length * sens_counter + length_counter] = (KRTDATA) result;

         }; // for sens_counter

    }; // for (length_counter = 0; length_counter < length; length_counter++)

    return 0;

}; // int fltNo_math

// ***************************************************************
// функция контурной фильтрации
int fltKontur (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long FilterParam_X,
long FilterParam_Y
)
{
    long * Dif_X_bufer; //[length] [sens_num];

    long sens_counter;
    long length_counter;

    long result;

    (void) dataOrnt;


    Dif_X_bufer = malloc(length * sens_num * sizeof(Dif_X_bufer[0]));


    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        Dif_X_bufer[length * sens_counter + 0] = 0;

        for (length_counter = 1;
             length_counter < length;
             length_counter++) 
        {

            Dif_X_bufer[length * sens_counter + length_counter] = 
                      rowData[length * sens_counter + length_counter] -
                      rowData[length * sens_counter + length_counter-1];

            if ( abs (Dif_X_bufer[length * sens_counter + length_counter]) > FilterParam_X)
            {
                Dif_X_bufer[length * sens_counter + length_counter] =
                     FilterParam_X *
                     ( Dif_X_bufer[length * sens_counter + length_counter] / 
                       abs (Dif_X_bufer[length * sens_counter + length_counter])
                     );
            };
        };
    };

    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {

        data[length * 0 + length_counter] = 0;

        for (sens_counter = 1; sens_counter < sens_num; sens_counter ++)
        {
            result =
                Dif_X_bufer[length * sens_counter + length_counter] - 
                Dif_X_bufer[length * (sens_counter-1) + length_counter];

            if ( result > FilterParam_Y)
            {
                result = FilterParam_Y *
                     (result / abs(result));
            }
            result *= 10;
            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;

            data[length * sens_counter + length_counter] = (KRTDATA) result;
        };
    };


    free(Dif_X_bufer);

    return 0;

}; // int fltKontur 

// ***************************************************************
// функция средняя линиия по найденной минимальной дисперсии
int fltSrednLin (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long ExpAvrgWaightingCoefficient,
long Amlification
)
{
    long * tmp_bufer; //[length] ;

    long sens_counter;
    long length_counter;

    long result;

    (void) dataOrnt;
    (void) ExpAvrgWaightingCoefficient;

    tmp_bufer = malloc(sens_num * length * sizeof(tmp_bufer[0]));
    if (tmp_bufer == NULL) return 0;

/*
    // поиск минимальной дисперсии по срезам
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        Dispers_Y_bufer[length_counter] = 0;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            Dispers_Y_bufer[length_counter] += 
                 abs( rowData[length * sens_counter + length_counter]
                      -
                      rowData[length * 0 + length_counter]
                    );
        };
    };

    min_dispers_pos = 0;
    min_dispers_value = Dispers_Y_bufer[0];
    for (length_counter = 1;
         length_counter < length;
         length_counter++) 
    {
        if ( min_dispers_value < Dispers_Y_bufer[length_counter])
        {
            min_dispers_value = Dispers_Y_bufer[length_counter];
            min_dispers_pos = length_counter;
        };
    };
*/


   // медиана на весь экран
   {
        long gistograma [MAX_DATA_VALUE];

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos = 0;

        long tube_size = 10;
        long tube_sum [MAX_DATA_VALUE];
        long tube_counter;

        long median_amlification;


        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            memset(gistograma, 0, MAX_DATA_VALUE * sizeof(gistograma [0]));
            memset(tube_sum, 0, MAX_DATA_VALUE * sizeof(tube_sum [0]));

            for (length_counter = 0;
                 length_counter < length;
                 length_counter++) 
            {
//                if ( rowData[length * sens_counter + length_counter] <2500)
                    gistograma[ rowData[length * sens_counter + length_counter] ] ++;
            }


            for ( gist_counter = 0; gist_counter < MAX_DATA_VALUE - tube_size; gist_counter++)
            {
                 for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                 {

                    tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                 }
            }

            max_gisttogram = 0;
            max_gist_pos = 0;
            for (gist_counter = 0; gist_counter < MAX_DATA_VALUE; gist_counter++)
            {
                if (max_gisttogram < tube_sum[gist_counter])
                {
                    max_gisttogram = tube_sum[gist_counter];
                    max_gist_pos = gist_counter;
                }
            }

            median_amlification = Amlification / 3;
            if (median_amlification < 1) median_amlification = 1;

            for (length_counter = 0;
                 length_counter < length;
                 length_counter++) 
            {
                result = rowData[length * sens_counter + length_counter] - max_gist_pos;

                result *= median_amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;

//                tmp_bufer[length * sens_counter + length_counter] = result;
            }
        }
   }
   // закончилась медиана на весь экран

/*
    // медианное усреднение
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        result = rowData[length * 0 + length_counter] - 
                    Median_average(rowData[length * 0 + length_counter],
                                     100, TRUE);

        for (length_counter = 0;
             length_counter < length;
             length_counter++) 
        {

            result = rowData[length * sens_counter + length_counter] - 
                       Median_average(rowData[length * sens_counter + length_counter],
                                        100, FALSE);

            result *= Amlification;

            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
            if (result < 0) result = 0;

            data[length * sens_counter + length_counter] =  (KRTDATA) result;

//            tmp_bufer[length * sens_counter + length_counter] = result;

        };
    };
    // закончилось медианное усреднение
*/

/*
    //  скользящее среднее
    {
    long WinLen = 100;
    long startWin;
    long local_sum;
    long local_average;


    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {

        for (startWin = 0;
             startWin < length-WinLen;
             startWin++) 
        {
            local_sum = 0;
            for (length_counter = startWin;
                 length_counter < startWin + WinLen;
                 length_counter++) 
            {
                local_sum += rowData[length * sens_counter + length_counter];
            };

            local_average = local_sum / WinLen;


            // вычтем постоянную составляющюю
            length_counter = startWin + WinLen / 2;
            result = rowData[length * sens_counter + length_counter] - 
                     local_average;

//            result *= Amlification;
//
//            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;
//
//            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
//            if (result < 0 ) result = 0;
//
//            data[length * sens_counter + length_counter] = (KRTDATA) result;

            tmp_bufer[length * sens_counter + length_counter] = result;
        };
    };
    }
    // посчитали скользящее среднее
*/

/*
    // среднее по всей длинне датчика
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {


            sum_dat = 0;

            for (length_counter = 0;
                 length_counter < length;
                 length_counter++) 
            {
                sum_dat += rowData[length * sens_counter + length_counter];
            }

            average = sum_dat/length;

            for (length_counter = 0; length_counter < length; length_counter++) 
            {
                 tmp_bufer[length * sens_counter + length_counter] = 
                       rowData[length * sens_counter + length_counter] - average;
            }
        }
*/

/*
    // фильтр поперек
    for (length_counter = 0; length_counter < length; length_counter++) 
    {
        result = tmp_bufer[length * 0 + length_counter] - 
                    Exponent_average(tmp_bufer[length * 0 + length_counter],
//        result = rowData[length * 0 + length_counter] - 
//                    Exponent_average(rowData[length * 0 + length_counter],
                                     ExpAvrgWaightingCoefficient, TRUE);

        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {

            result = tmp_bufer[length * sens_counter + length_counter] - 
                       Exponent_average(tmp_bufer[length * sens_counter + length_counter],
//            result = rowData[length * sens_counter + length_counter] - 
//                       Exponent_average(rowData[length * sens_counter + length_counter],
                                        ExpAvrgWaightingCoefficient, FALSE);
            result *= Amlification;

            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
            if (result < 0) result = 0;

            data[length * sens_counter + length_counter] =  (KRTDATA) result;

//            tmp_bufer[length * sens_counter + length_counter] = result;
        };
    };
*/

/*
    // среднее по всей длинне датчика
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {

            sum_dat = 0;

            for (length_counter = 0; length_counter < length; length_counter++) 
            {
                sum_dat += tmp_bufer[length * sens_counter + length_counter];
            }

            average = sum_dat/length;

            for (length_counter = 0; length_counter < length; length_counter++) 
            {
                 result = tmp_bufer[length * sens_counter + length_counter];// - average;

                 result *= Amlification;

//                 result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;
//
//                 if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
//                 if (result < 0) result = 0;
//
//                 data[length * sens_counter + length_counter] =  (KRTDATA) result;

                 tmp_bufer[length * sens_counter + length_counter] = result;
            }
        }
*/

/*
    // приведение всех датчиков к средней линии
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = tmp_bufer[length * sens_counter + length_counter] - 
                     tmp_bufer[length * sens_counter];
//                     rowData[length * sens_counter + min_dispers_pos];
//                     rowData[length * sens_counter + length/2];

            result *= Amlification;

            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
            if (result < 0) result = 0;

            data[length * sens_counter + length_counter] =  (KRTDATA) result;
        };
    };
*/


    free(tmp_bufer);

    return 0;

}; // int fltSrednLin 


// ***************************************************************
// функция скользящего среднего
int fltSkolzyashSredn (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long WinLen,
long Amlification
)
{
    long sens_counter;
    long length_counter;

    long startWin;
    long local_sum;
    long local_average;

    long result;

    (void) dataOrnt;

    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {

        for (startWin = 0;
             startWin < length-WinLen;
             startWin++) 
        {
            local_sum = 0;
            for (length_counter = startWin;
                 length_counter < startWin + WinLen;
                 length_counter++) 
            {
                local_sum += rowData[length * sens_counter + length_counter];
            };

            local_average = local_sum / WinLen;


            // вычтем постоянную составляющюю
            length_counter = startWin + WinLen / 2;
            result = rowData[length * sens_counter + length_counter] - 
                     local_average;

            result *= Amlification;

            result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;

            data[length * sens_counter + length_counter] = (KRTDATA) result;

        };
    };

    return 0;

}; // int fltSkolzyashSredn

// ***************************************************************
// функция контурной фильтрации
int fltTreshini (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long FilterLim,
long Amlification
)
{
    double* Double_bufer;  // length * sens_num

    long sens_counter;
    long length_counter;

    double xm;
    double xstd;

    double x0;
    long k;

    long result;

    (void) dataOrnt;

    Double_bufer = malloc(length * sens_num * sizeof(Double_bufer[0]));


    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            Double_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter];
        };
    };

    // Удаление среднего по каждому датчику
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
       xm = CalculateMean(&Double_bufer[length * sens_counter], length);
       xstd  =  GetStandardDeviation(&Double_bufer[length * sens_counter], length);
       x0 = 0;
       k = 0;

       for (length_counter = 0;
            length_counter < length;
            length_counter++) 
       {
           if ( (Double_bufer[length * sens_counter + length_counter] > xm - xstd) ||
                (Double_bufer[length * sens_counter + length_counter] < xm + xstd) )
           {
               x0 = x0 + Double_bufer[length * sens_counter + length_counter];
               k = k + 1;
           };
       };

       x0 = x0 / k;

       for (length_counter = 0;
            length_counter < length;
            length_counter++) 
       {
          Double_bufer[length * sens_counter + length_counter] =
              Double_bufer[length * sens_counter + length_counter] - x0;
       };
    };
    // Фильтрация сигналов датчиков без Н0 пороговым фильтром

    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
       for (length_counter = 0;
            length_counter < length;
            length_counter++) 
       {
          if (Double_bufer[length * sens_counter + length_counter] < FilterLim)
          {
              result = (long) FilterLim;
          } else {
              result = (long) Double_bufer[length * sens_counter + length_counter];
          };
          result = KRT_PALLETE_SIZE/2 + (result * Amlification * KRT_PALLETE_SIZE) / 4096;

          if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
          if (result < 0 ) result = 0;

          data[length * sens_counter + length_counter] = (KRTDATA) result;
       };
    };



    free(Double_bufer);

    return 0;

}; // int fltTreshini 


// ***************************************************************
// функция полигональной фильтрации фильтрации
int fltPoligon (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long FilterLength,     // ширина окна фильтра (1-500 в отсчетах одометра, default = 30)
long PoligonAmplifer,  // параметр усиления фильтра (1-30, default =2)
long Srez_ON_OF        // включение/выключение выравнивания
)
{
    long *src_data_bufer; //[FilterLength] [sens_num];
    long cur_src_data_buf_len = 0;
    long cur_pos_src_data_buf = 0;

    long pos_in_buf_for_average;

    long sens_counter;

    long length_counter;

    long pos_screen;

    int bufer_shift_counter;

    long average_value;
    long average_sum = 0;

    long result;

    { // проверка на изменение параметров фильтра
       static long FilterLength_pred    = 0;
       static long PoligonAmplifer_pred = 0;
       static long Srez_ON_OF_pred      = 0;
       
       if ( ( FilterLength_pred != FilterLength ) ||
            ( PoligonAmplifer_pred != PoligonAmplifer ) ||
            ( Srez_ON_OF_pred != Srez_ON_OF )
          )
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

       FilterLength_pred = FilterLength;
       PoligonAmplifer_pred = PoligonAmplifer;
       Srez_ON_OF_pred = Srez_ON_OF;
    }

    (void) dataOrnt;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
fltSkolzyashSredn (
data,
rowData,
dataOrnt,
length,
sens_num,
FilterLength,
PoligonAmplifer
);
*/


/*
// последний рабочий
fltTreshini (
data,
rowData,
dataOrnt,
length,
sens_num,
FilterLength,
PoligonAmplifer
);
*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    src_data_bufer = malloc(FilterLength * sens_num * sizeof(src_data_bufer[0]));

    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        for (bufer_shift_counter = 0;
             bufer_shift_counter < FilterLength;
             bufer_shift_counter++) 
        {
            src_data_bufer [bufer_shift_counter + sens_counter * FilterLength] = 0;
        };
    };

    for (length_counter = 0; length_counter < length; length_counter++)
    { 
         if (cur_src_data_buf_len >= FilterLength)
         {
             // вычеркнем устаревшее значение из буфера исходных данных
             cur_src_data_buf_len--;
             // вычеркнули устаревшее значение из буфера исходных данных
             for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
             {
                 average_sum -= src_data_bufer [cur_pos_src_data_buf + sens_counter * FilterLength];
             };
         };

         // добавим новое значение в буфер исходных данных
         for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
         {
             src_data_bufer [cur_pos_src_data_buf + sens_counter * FilterLength] = 
                rowData[length * sens_counter + length_counter];
             average_sum += src_data_bufer [cur_pos_src_data_buf + sens_counter * FilterLength];
         };
         cur_src_data_buf_len++;
         // добавили новое значение в буфер исходных данных
         average_value = average_sum / (FilterLength * sens_num);

         // собственно усредненние занчений
         for (sens_counter = 0; sens_counter < sens_num; sens_counter ++) 
         {
             pos_in_buf_for_average = cur_pos_src_data_buf - FilterLength/2;
             if (pos_in_buf_for_average < 0) pos_in_buf_for_average += FilterLength;

             result =  src_data_bufer[ pos_in_buf_for_average + sens_counter * FilterLength]
                       - average_value;

             result *= PoligonAmplifer;

             result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

             if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
             if (result < 0 ) result = 0;

             pos_screen = length_counter - FilterLength/2;
             if (pos_screen < 0) pos_screen = 0;

             data[length * sens_counter + pos_screen] = (KRTDATA) result;

         }; // for sens_counter

         cur_pos_src_data_buf++;
         if (cur_pos_src_data_buf >= FilterLength) cur_pos_src_data_buf=0;
    }; // for (length_counter = 0; length_counter < length; length_counter++)

    free (src_data_bufer);

    if (Srez_ON_OF != 0)
    {
        Base_line_init( data, length, sens_num, FilterLength/2);
        Base_line_calkulate(data, length, sens_num);
    }

    return 0;

}; // long poligon_filter


// ***************************************************************
// функция медианного усреднения на весь экран
int fltMedianFullScreen (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long ExpAvrgWaightingCoefficient,
long Amlification,      // параметр усиление 0-64
long FilterLength,     // ширина окна фильтра (1-500 в отсчетах одометра, default = 100)
long AwerageLength     // параметр kolichestvo izmereniy dlya rascheta srednego (1-50, default =25)
)
{
    long sens_counter;
    long length_counter;

    long result;

    long* long_bufer;    // length * sens_num

    long start_filter;
    long end_filter;

    long zone_count;
    long cur_index;

    (void) dataOrnt;
    (void) ExpAvrgWaightingCoefficient;

    { // проверка на изменение параметров фильтра
       static long Amlification_pred = 0;
       static long FilterLength_pred = 0;
       static long AwerageLength_pred = 0;

       if ( ( Amlification != Amlification_pred ) ||
            ( FilterLength != FilterLength_pred ) ||
            ( AwerageLength != AwerageLength_pred )
          ) flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

       Amlification = Amlification;
       FilterLength_pred = FilterLength;
       AwerageLength_pred = AwerageLength;
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    // медиана на весь экран
    {
        long gistograma [MAX_DATA_VALUE];

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos = 0;

        long tube_size = 10;
        long tube_sum [MAX_DATA_VALUE];
        long tube_counter;

        long median_amlification;

        start_filter = length/2 - FilterLength/2;
        if (start_filter < 0) start_filter = 0;

        end_filter = length/2 + FilterLength/2;
        if (end_filter > length) end_filter = length;

        if ( FilterLength == 0) {
            start_filter = 0;
            end_filter = length;
            FilterLength = length;
        }

        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            memset(gistograma, 0, MAX_DATA_VALUE * sizeof(gistograma [0]));
            memset(tube_sum, 0, MAX_DATA_VALUE * sizeof(tube_sum [0]));

            for (length_counter = start_filter;
                 length_counter < end_filter;
                 length_counter++) 
            {
                if ( rowData[length * sens_counter + length_counter] >= MAX_DATA_VALUE-3)
                          rowData[length * sens_counter + length_counter] = 0;

                    gistograma[ rowData[length * sens_counter + length_counter] ] ++;
            }


            for ( gist_counter = 0; gist_counter < MAX_DATA_VALUE - tube_size; gist_counter++)
            {
                 for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                 {

                    tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                 }
            }

            max_gisttogram = 0;
            max_gist_pos = 0;
            for (gist_counter = 0; gist_counter < MAX_DATA_VALUE; gist_counter++)
            {
                if (max_gisttogram < tube_sum[gist_counter])
                {
                    max_gisttogram = tube_sum[gist_counter];
                    max_gist_pos = gist_counter;
                }
            }

            median_amlification = Amlification / 3;
            if (median_amlification < 1) median_amlification = 1;

            for (length_counter = 0;
                 length_counter < length;
                 length_counter++) 
            {
                if ( rowData[length * sens_counter + length_counter] >= MAX_DATA_VALUE-3)
                          rowData[length * sens_counter + length_counter] = 0;

                result = rowData[length * sens_counter + length_counter] - max_gist_pos;

                result *= median_amlification;

                long_bufer[length * sens_counter + length_counter] = result;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            }
        } // for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    } // закончилась медиана на весь экран
/*
    // фильтр поперек
    // скользящее среднее
    {
        long WinLen = 5;
        long startWin;
        long local_sum_do;
        long local_sum_posle;
        long local_average_do;
        long local_average_posle;
        long local_average;
        long i;


        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_sum_do = 0;   
                local_sum_posle = 0;
                for ( i = 1; i <= WinLen; i++)
                {
                    startWin = sens_counter - i;
                    if (startWin >= 0) local_sum_do += long_bufer[length * startWin + length_counter];
                       else local_sum_do += long_bufer[length * 0 + length_counter];

                    startWin = sens_counter + i;
                    if (startWin < sens_num) local_sum_posle += long_bufer[length * startWin + length_counter];
                       else local_sum_posle += long_bufer[length * (sens_num - 1) + length_counter];
                };

                local_average_do = local_sum_do / WinLen;   
                local_average_posle = local_sum_posle / WinLen;

                if (local_average_do < local_average_posle) local_average = local_average_do;
                       else local_average = local_average_posle;


                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
    };
    // посчитали фильтр поперек
*/
    { // Попробуем пристроить свертку Лапласиан

         // Данные фильтров сверток
         double k = Amlification / 3;

         #pragma warning(disable : 4204)  // давим варнинг про неконстанту
                                      // при инициализации массива
         // Лапласиан 5x5
         #define matr_size_5 5
         double matrix[matr_size_5][matr_size_5]
                         = {{     0,     0, -k/16,     0,     0 },
                            {     0, -k/16,  -k/8, -k/16,     0 },
                            { -k/16,  -k/8,   k+1,  -k/8, -k/16 },
                            {     0, -k/16,  -k/8, -k/16,     0 },
                            {     0,     0, -k/16,     0,     0 } };
         #pragma warning(default : 4204)

         double* Double_bufer;  // length * sens_num
         long matr_x, matr_y;
         double svertka;
         // закончились данные фильтров сверток

         Double_bufer = malloc(length * sens_num * sizeof(Double_bufer[0]));

         for (length_counter = matr_size_5 /2;
              length_counter < length - matr_size_5 /2;
              length_counter++) 
         {
             for (sens_counter = matr_size_5 /2;
                  sens_counter < sens_num - matr_size_5 /2;
                  sens_counter ++)
             {
                 svertka = 0;
                 for ( matr_x = 0; matr_x < matr_size_5; matr_x++)
                 {
                     for ( matr_y = 0; matr_y < matr_size_5; matr_y++)
                     {
                         svertka += 
                          (double) long_bufer[ length * (sens_counter + matr_y - matr_size_5 /2)
                                      + length_counter + matr_x - matr_size_5 /2]
                          *
                          matrix [matr_x][matr_y]; 
                     };
                 };
                 Double_bufer[length * sens_counter + length_counter] = svertka;
             };
         };

         for (length_counter = matr_size_5 /2;
              length_counter < length - matr_size_5 /2;
              length_counter++) 
         {
             for (sens_counter = matr_size_5 /2;
                  sens_counter < sens_num - matr_size_5 /2;
                  sens_counter ++)
             {
                 long_bufer[length * sens_counter + length_counter] = (long)
                    Double_bufer[length * sens_counter + length_counter];

                    // копируем данные в "экран"
                    result = long_bufer[length * sens_counter + length_counter];

                    result *= Amlification;

                    result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                    if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                    if (result < 0) result = 0;

                    data[length * sens_counter + length_counter] =  (KRTDATA) result;
             };
         };

         free(Double_bufer);
    } // Пристроили  свертку Лапласиан

    // "негатив" цвета на границы зоны вычисления среднего
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        for (zone_count = 0; zone_count < 10/2; zone_count ++)
        {
            cur_index = length * sens_counter + start_filter + zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
            cur_index = length * sens_counter + end_filter - zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE-1;
        }
    }

    free(long_bufer);

    return 0;

}; // int fltMedianFullScreen (

// ***************************************************************
// фильтр к центру //2021 // volosok
int flt_filament (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long filterParam,      // параметр усиление 0-64
long FilterLength,     // ширина окна фильтра (1-500 в отсчетах одометра, default = 100)
long AwerageLength     // параметр kolichestvo izmereniy dlya rascheta srednego (1-50, default =25)
) {
    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long * average_value_for_sens;

    long flag_viravnivanie=0;
    long i;

    long zone_count;

    long start_filter;
    long end_filter;

    long cur_index;

    (void *) dataOrnt;

    if (FilterLength == 0) FilterLength = length;

    { // проверка на изменение параметров фильтра
       static long filterParam_pred = 0;
       static long FilterLength_pred = 0;
       static long AwerageLength_pred = 0;

       if ( ( filterParam != filterParam_pred ) ||
            ( FilterLength != FilterLength_pred ) ||
            ( AwerageLength != AwerageLength_pred )
          ) flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

       filterParam_pred = filterParam;
       FilterLength_pred = FilterLength;
       AwerageLength_pred = AwerageLength;
    }

    // отработка флага "выравнивание"
    for (i = 0; i < sens_num; i ++) {
       if (sensor[i].value != 0) {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));
    average_value_for_sens = malloc(sens_num * sizeof(average_value_for_sens[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    start_filter = length/2 - FilterLength/2;
    if (start_filter < 0) start_filter = 0;

    end_filter = length/2 + FilterLength/2;
    if (end_filter > length) end_filter = length;

    if ( FilterLength == 0) {
        start_filter = 0;
        end_filter = length;
        FilterLength = length;
    }

    for (length_counter = start_filter; length_counter < end_filter; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            if (flag_viravnivanie != 0)
            {
                long srez_value  = sensor[sens_counter].value;

                average_value_for_sens[sens_counter] = srez_value;
            } else {
                average_value_for_sens[sens_counter] = 0;
                for (zone_count = 0; zone_count < AwerageLength/2; zone_count ++)
                {
                    average_value_for_sens[sens_counter] += rowData[length * sens_counter + start_filter + zone_count];
                    average_value_for_sens[sens_counter] += rowData[length * sens_counter + end_filter - zone_count-1];
                }
                average_value_for_sens[sens_counter] /= AwerageLength;
            }
        }
    }

    for (length_counter = 0; length_counter < length; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens[sens_counter];
        }
    }


    // Усиление
    for (length_counter = 0; length_counter < length; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * filterParam / 10;
        }
    }

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     (long_bufer[length * sens_counter + length_counter]
                      * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    } // for (length_counter = 0; length_counter < length; length_counter++)

/*
    // рисуем вертикальные черные полоски по ширине фильтра
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        data[length * sens_counter + start_filter] = 0;
        data[length * sens_counter + end_filter] = 0;
    }
*/

    // негатив цвета на зону вычисления среднего
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        for (zone_count = 0; zone_count < AwerageLength/2; zone_count ++)
        {
            cur_index = length * sens_counter + start_filter + zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
            cur_index = length * sens_counter + end_filter - zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
        }
    }


    free(average_value_for_sens);
    free(long_bufer);

    return 0;
} // int flt_filament (

// ***************************************************************
// функция фильтра Фильтр 2019
int flt_Filter2019 (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long filterParam       // параметр усиление 0-64
) {

    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long average_value_for_sens=0;

    long flag_viravnivanie=0;
    long i;

    #define zone_len 128
    long zone_count;

    (void *) dataOrnt;

    { // проверка на изменение параметров фильтра
       static long filterParam_pred = 0;

       if ( filterParam != filterParam_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       filterParam_pred = filterParam;
    }


    for (i = 0; i < sens_num; i ++)
    {
       if (sensor[i].value != 0)
       {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        if (flag_viravnivanie != 0)
        {
            long srez_value  = sensor[sens_counter].value;

            average_value_for_sens = srez_value;
        } else {
            average_value_for_sens = 0;
            for (zone_count = length/2 - zone_len/2;
                 zone_count < length/2 + zone_len/2;
                 zone_count ++)
            {
                average_value_for_sens += rowData[length * sens_counter + zone_count];
            }
            average_value_for_sens /= (zone_len);
        }

        for (length_counter = 0;
             length_counter < length;
             length_counter++) 
        {

            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens;
        }
    };

    // Усиление
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * filterParam / 10;
        };
    };

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;

        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     (long_bufer[length * sens_counter + length_counter]
                      * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    }; // for (length_counter = 0; length_counter < length; length_counter++)
    free(long_bufer);

    return 0;
} // функция фильтра Фильтр 2019   int flt_Filter2019 (

// ***************************************************************
// функция фильтра Свертка 1 Лапласиан
int flt_convolution_1 (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long Amlification       // параметр усиление 0-64
) {

    long* long_bufer;    // length * sens_num
    long* long_bufer_2;    // length * sens_num

    long sens_counter;
    long length_counter;

    long average_value_for_sens=0;

    long flag_viravnivanie=0;
    long i;

    #define zone_len 128
    long zone_count;

    // Данные фильтров сверток
    double k = Amlification;

#pragma warning(disable : 4204)  // давим варнинг про неконстанту
                                 // при инициализации массива
    // Лапласиан 5x5
    #define matr_size_5 5
    double matrix[matr_size_5][matr_size_5]
                    = {{     0,     0, -k/16,     0,     0 },
                       {     0, -k/16,  -k/8, -k/16,     0 },
                       { -k/16,  -k/8,   k+1,  -k/8, -k/16 },
                       {     0, -k/16,  -k/8, -k/16,     0 },
                       {     0,     0, -k/16,     0,     0 } };
#pragma warning(default : 4204)

    double* Double_bufer;  // length * sens_num
    long matr_x, matr_y;
    double svertka;
    // закончились данные фильтров сверток

    (void *) dataOrnt;

    { // проверка на изменение параметров фильтра
       static long Amlification_pred = 0;

       if ( Amlification != Amlification_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       Amlification_pred = Amlification;
    }

    for (i = 0; i < sens_num; i ++)
    {
       if (sensor[i].value != 0)
       {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));
    long_bufer_2 = malloc(length * sens_num * sizeof(long_bufer_2[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            if (flag_viravnivanie != 0)
            {
                long srez_value  = sensor[sens_counter].value;

                average_value_for_sens = srez_value;
            } else {
                average_value_for_sens = 0;
                for (zone_count = length/2 - zone_len/2;
                     zone_count < length/2 + zone_len/2;
                     zone_count ++)
                {
                    average_value_for_sens += rowData[length * sens_counter + zone_count];
                }
                average_value_for_sens /= zone_len;
            }

            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens;
        };
    };

    // Усиление
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * 1;
        };
    };


    { // Попробуем пристроить Лапласиан
         Double_bufer = malloc(length * sens_num * sizeof(Double_bufer[0]));

         for (length_counter = matr_size_5 /2;
              length_counter < length - matr_size_5 /2;
              length_counter++) 
         {
             for (sens_counter = matr_size_5 /2;
                  sens_counter < sens_num - matr_size_5 /2;
                  sens_counter ++)
             {
                 svertka = 0;
                 for ( matr_x = 0; matr_x < matr_size_5; matr_x++)
                 {
                     for ( matr_y = 0; matr_y < matr_size_5; matr_y++)
                     {
                         svertka += 
                          (double) long_bufer[ length * (sens_counter + matr_y - matr_size_5 /2)
                                      + length_counter + matr_x - matr_size_5 /2]
                          *
                          matrix [matr_x][matr_y]; 
                     };
                 };
                 Double_bufer[length * sens_counter + length_counter] = svertka;
             };
         };

         for (length_counter = matr_size_5 /2;
              length_counter < length - matr_size_5 /2;
              length_counter++) 
         {
             for (sens_counter = matr_size_5 /2;
                  sens_counter < sens_num - matr_size_5 /2;
                  sens_counter ++)
             {
                 long_bufer[length * sens_counter + length_counter] = (long)
                    Double_bufer[length * sens_counter + length_counter];
             };
         };

         free(Double_bufer);
    } // Пристроили свертку Лапласиан

    // фильтр поперек
    // скользящее среднее
    {
        long WinLen = 5;

        long startWin;
        long local_sum_do;
        long local_sum_posle;
        long local_average_do;
        long local_average_posle;
        long local_average;
        long result;

        for (length_counter = 0; length_counter < length; length_counter++)
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_sum_do = 0;   
                local_sum_posle = 0;
                for ( i = 1; i <= WinLen; i++)
                {
                    startWin = sens_counter - i;
                    if (startWin >= 0) local_sum_do += long_bufer[length * startWin + length_counter];
                       else local_sum_do += long_bufer[length * 0 + length_counter];

                    startWin = sens_counter + i;
                    if (startWin < sens_num) local_sum_posle += long_bufer[length * startWin + length_counter];
                       else local_sum_posle += long_bufer[length * (sens_num - 1) + length_counter];
                };

                local_average_do = local_sum_do / WinLen;   
                local_average_posle = local_sum_posle / WinLen;

                if (local_average_do < local_average_posle) local_average = local_average_do;
                       else local_average = local_average_posle;

                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                long_bufer_2[length * sens_counter + length_counter] = result;
            };
        };
    }; // for (length_counter = 0; length_counter < length; length_counter++)
    // посчитали фильтр поперек

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     (long_bufer_2[length * sens_counter + length_counter]
                      * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    }; // for (length_counter = 0; length_counter < length; length_counter++)


    free(long_bufer_2);
    free(long_bufer);

    return 0;
} // функция фильтра Свертка 1   int flt_convolution_1 (

// ***************************************************************
// функция фильтра Свертка 2
int flt_convolution_2 (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long filterParam       // параметр усиление 0-64
) {

    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long average_value_for_sens=0;

    long flag_viravnivanie=0;
    long i;

    #define zone_len 128
    long zone_count;

    // Данные фильтров сверток

    double k = filterParam;

#pragma warning(disable : 4204)  // давим варнинг про неконстанту
                                 // при инициализации массива
    // Контрасность 3x3
    #define matr_size 3
    double matrix[matr_size][matr_size]
                    = {{ -k/8, -k/8, -k/8 },
                       { -k/8,  k+1, -k/8 },
                       { -k/8, -k/8, -k/8 } };
#pragma warning(default : 4204)

    double* Double_bufer;  // length * sens_num
    long matr_x, matr_y;
    double svertka;
    // закончились данные фильтров сверток

    (void *) dataOrnt;

    { // проверка на изменение параметров фильтра
       static long filterParam_pred = 0;

       if ( filterParam != filterParam_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       filterParam_pred = filterParam;
    }


    for (i = 0; i < sens_num; i ++)
    {
       if (sensor[i].value != 0)
       {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            if (flag_viravnivanie != 0)
            {
                long srez_value  = sensor[sens_counter].value;

                average_value_for_sens = srez_value;
            } else {
                average_value_for_sens = 0;
                for (zone_count = length/2 - zone_len/2;
                     zone_count < length/2 + zone_len/2;
                     zone_count ++)
                {
                    average_value_for_sens += rowData[length * sens_counter + zone_count];
                }
                average_value_for_sens /= zone_len;
            }

            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens;
        };
    };

    // Усиление
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * 1;
        };
    };


    { // Попробуем пристроить контрасность
         Double_bufer = malloc(length * sens_num * sizeof(Double_bufer[0]));

         for (length_counter = matr_size /2;
              length_counter < length - matr_size /2;
              length_counter++) 
         {
             for (sens_counter = matr_size /2;
                  sens_counter < sens_num - matr_size /2;
                  sens_counter ++)
             {
                 svertka = 0;
                 for ( matr_x = 0; matr_x < matr_size; matr_x++)
                 {
                     for ( matr_y = 0; matr_y < matr_size; matr_y++)
                     {
                         svertka += 
                          (double) long_bufer[ length * (sens_counter + matr_y - matr_size /2)
                                      + length_counter + matr_x - matr_size /2]
                          *
                          matrix [matr_x][matr_y]; 
                     };
                 };
                 Double_bufer[length * sens_counter + length_counter] = svertka;
             };
         };

         for (length_counter = matr_size /2;
              length_counter < length - matr_size /2;
              length_counter++) 
         {
             for (sens_counter = matr_size /2;
                  sens_counter < sens_num - matr_size /2;
                  sens_counter ++)
             {
                 long_bufer[length * sens_counter + length_counter] = (long)
                    Double_bufer[length * sens_counter + length_counter];
             };
         };

         free(Double_bufer);
    } // Пристроили контрастность

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     (long_bufer[length * sens_counter + length_counter]
                      * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    }; // for (length_counter = 0; length_counter < length; length_counter++)
    free(long_bufer);

    return 0;
} // функция фильтра Свертка 2   int flt_convolution_2 (

// ***************************************************************
// функция фильтра Свертка 3
int flt_convolution_3 (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long filterParam       // параметр усиление 0-64
) {

    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long average_value_for_sens=0;

    long flag_viravnivanie=0;
    long i;

    #define zone_len 128
    long zone_count;

    // Данные фильтров сверток

//    double k = filterParam;
    double k = 1;

#pragma warning(disable : 4204)  // давим варнинг про неконстанту
                                 // при инициализации массива
    // Сглаживание 3x3
    #define matr_size 3
    double matrix[matr_size][matr_size]
                    = {{ k/9, k/9, k/9 },
                       { k/9, k/9, k/9 },
                       { k/9, k/9, k/9 } };
#pragma warning(default : 4204)

    double* Double_bufer;  // length * sens_num
    long matr_x, matr_y;
    double svertka;
    // закончились данные фильтров сверток

    (void *) dataOrnt;

    { // проверка на изменение параметров фильтра
       static long filterParam_pred = 0;

       if ( filterParam != filterParam_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       filterParam_pred = filterParam;
    }

    for (i = 0; i < sens_num; i ++)
    {
       if (sensor[i].value != 0)
       {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            if (flag_viravnivanie != 0)
            {
                long srez_value  = sensor[sens_counter].value;

                average_value_for_sens = srez_value;
            } else {
                average_value_for_sens = 0;
                for (zone_count = length/2 - zone_len/2;
                     zone_count < length/2 + zone_len/2;
                     zone_count ++)
                {
                    average_value_for_sens += rowData[length * sens_counter + zone_count];
                }
                average_value_for_sens /= zone_len;
            }

            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens;
        };
    };

    // Усиление
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * filterParam;
        };
    };


    { // Попробуем пристроить сглаживание
         Double_bufer = malloc(length * sens_num * sizeof(Double_bufer[0]));

         for (length_counter = matr_size /2;
              length_counter < length - matr_size /2;
              length_counter++) 
         {
             for (sens_counter = matr_size /2;
                  sens_counter < sens_num - matr_size /2;
                  sens_counter ++)
             {
                 svertka = 0;
                 for ( matr_x = 0; matr_x < matr_size; matr_x++)
                 {
                     for ( matr_y = 0; matr_y < matr_size; matr_y++)
                     {
                         svertka += 
                          (double) long_bufer[ length * (sens_counter + matr_y - matr_size /2)
                                      + length_counter + matr_x - matr_size /2]
                          *
                          matrix [matr_x][matr_y]; 
                     };
                 };
                 Double_bufer[length * sens_counter + length_counter] = svertka;
             };
         };

         for (length_counter = matr_size /2;
              length_counter < length - matr_size /2;
              length_counter++) 
         {
             for (sens_counter = matr_size /2;
                  sens_counter < sens_num - matr_size /2;
                  sens_counter ++)
             {
                 long_bufer[length * sens_counter + length_counter] = (long)
                    Double_bufer[length * sens_counter + length_counter];
             };
         };

         free(Double_bufer);
    } // Пристроили снлаживание

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     (long_bufer[length * sens_counter + length_counter]
                      * KRT_PALLETE_SIZE) / 4096;

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    }; // for (length_counter = 0; length_counter < length; length_counter++)
    free(long_bufer);

    return 0;
} // функция фильтра Свертка 3   int flt_convolution_3 (


// ***************************************************************
// функция фильтра обрезание верхнего и нижнего сигнала
int flt_Cut_big_small (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
long filterParam1,     // параметр1 фильтра
long filterParam2,     // параметр2 фильтра
long rolledAmplifer    // параметр усиления фильтра
) {

    // константы для работв фильтра
    long average_zone_len = 32;
    long big_level   =   50;
    long small_level =  -25; 
    // объявили константы для работв фильтра


    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long average_value_for_sens=0;

    long zone_count;

    (void *) dataOrnt;
    (void *) filterParam1;
    (void *) filterParam2;
    (void *) rolledAmplifer;

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    for (length_counter = 0;
         length_counter < length;
         length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
                average_value_for_sens = 0;
                for (zone_count = 0;
                     zone_count < average_zone_len/2;
                     zone_count ++)
                {
                    average_value_for_sens += rowData[length * sens_counter + zone_count];
                    average_value_for_sens += rowData[length * sens_counter + length - zone_count-1];
                }
                average_value_for_sens /= average_zone_len;

            long_bufer[length * sens_counter + length_counter] =
               rowData[length * sens_counter + length_counter]
               - average_value_for_sens;

            if (long_bufer[length * sens_counter + length_counter] > big_level)
                long_bufer[length * sens_counter + length_counter] = big_level;
            if (long_bufer[length * sens_counter + length_counter] < small_level)
                long_bufer[length * sens_counter + length_counter] = small_level;
        };
    };

    // Усиление
//    for (length_counter = 0;
//         length_counter < length;
//         length_counter++) 
//    {
//        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
//        {
//            long_bufer[length * sens_counter + length_counter] =
//                long_bufer[length * sens_counter + length_counter] * filterParam / 10;
//        };
//    };

    // готовые значения копируем в "Экран"
    for (length_counter = 0; length_counter < length; length_counter++)
    {
        long result;
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            result = KRT_PALLETE_SIZE/2 + 
                     long_bufer[length * sens_counter + length_counter];

            if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE - 1;
            if (result < 0 ) result = 0;
            data[length * sens_counter + length_counter] = (KRTDATA) result;
        }
    }; // for (length_counter = 0; length_counter < length; length_counter++)
    free(long_bufer);


    return 0;
} // int flt_Cut_big_small (

// ***************************************************************
// функция скользящее среднее по 5 предыдущим или 5 последующим смотря как больше знач
// плюс поперек показать дельты между значениями
int flt_Skolz_plus_poperek (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long Amlification
)
{
    long * tmp_bufer; //[length] ;

    long sens_counter;
    long length_counter;

    long result;
    
    long i;
    
    (void) dataOrnt;
    (void) sensor;
    
    tmp_bufer = malloc(sens_num * length * sizeof(tmp_bufer[0]));
    if (tmp_bufer == NULL) return 0;

    //  скользящее среднее
    {
        long WinLen = 5;
        long startWin;
        long local_sum_do;
        long local_sum_posle;
        long local_average_do;
        long local_average_posle;
        long local_average;

        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        
        {
            for (length_counter = 0; length_counter < length; length_counter++) 
            {
                local_sum_do = 0;   
                local_sum_posle = 0;
                for ( i = 1; i <= WinLen; i++)
                {
                    startWin = length_counter - i;
                    if (startWin >= 0) local_sum_do += rowData[length * sens_counter + startWin];
                       else local_sum_do += rowData[length * sens_counter + 0];

                    startWin = length_counter + i;
                    if (startWin < length) local_sum_posle += rowData[length * sens_counter + startWin];
                       else local_sum_posle += rowData[length * sens_counter + length-1];
                };

                local_average_do = local_sum_do / WinLen;   
                local_average_posle = local_sum_posle / WinLen;

                if (local_average_do < local_average_posle) local_average = local_average_do;
                       else local_average = local_average_posle;

                // вычтем постоянную составляющюю
                result = rowData[length * sens_counter + length_counter] - local_average;

                tmp_bufer[length * sens_counter + length_counter] = result;
            };
        };
    }
    // посчитали скользящее среднее

    // фильтр поперек
    //  скользящее среднее
    {
        long WinLen = 5;
        long startWin;
        long local_sum_do;
        long local_sum_posle;
        long local_average_do;
        long local_average_posle;
        long local_average;


        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_sum_do = 0;   
                local_sum_posle = 0;
                for ( i = 1; i <= WinLen; i++)
                {
                    startWin = sens_counter - i;
                    if (startWin >= 0) local_sum_do += tmp_bufer[length * startWin + length_counter];
                       else local_sum_do += tmp_bufer[length * 0 + length_counter];

                    startWin = sens_counter + i;
                    if (startWin < sens_num) local_sum_posle += tmp_bufer[length * startWin + length_counter];
                       else local_sum_posle += tmp_bufer[length * (sens_num - 1) + length_counter];
                };

                local_average_do = local_sum_do / WinLen;   
                local_average_posle = local_sum_posle / WinLen;

                if (local_average_do < local_average_posle) local_average = local_average_do;
                       else local_average = local_average_posle;


                // вычтем постоянную составляющюю
                result = tmp_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
/*
        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 1; sens_counter < sens_num; sens_counter ++)
            {
                result = tmp_bufer[length * sens_counter + length_counter] -
                         tmp_bufer[length * (sens_counter-1) + length_counter];

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
*/
    };
    // посчитали фильтр поперек

    free(tmp_bufer);

    return 0;

}; // int flt_Skolz_plus_poperek


// ***************************************************************
// фильтр к центру плюс поперек
int flt_k_tsentru_plus_poperek (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long Amlification,      // параметр усиление 0-64
long FilterLength,     // ширина окна фильтра (1-500 в отсчетах одометра, default = 100)
long AwerageLength     // параметр kolichestvo izmereniy dlya rascheta srednego (1-50, default =25)
) {
    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;

    long * average_value_for_sens;

    long flag_viravnivanie=0;
    long i;

    long zone_count;

    long start_filter;
    long end_filter;

    long cur_index;

    long result;

    long median_src_min;
    long median_src_max;
    long median_src_razmax;

    (void *) dataOrnt;


    if (FilterLength == 0) FilterLength = length;

    { // проверка на изменение параметров фильтра
       static long Amlification_pred = 0;
       static long FilterLength_pred = 0;
       static long AwerageLength_pred = 0;

       if ( ( Amlification != Amlification_pred ) ||
            ( FilterLength != FilterLength_pred ) ||
            ( AwerageLength != AwerageLength_pred )
          ) flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

       Amlification_pred = Amlification;
       FilterLength_pred = FilterLength;
       AwerageLength_pred = AwerageLength;
    }

    for (i = 0; i < sens_num; i ++) {
       if (sensor[i].value != 0) {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));
    average_value_for_sens = malloc(sens_num * sizeof(average_value_for_sens[0]));

    // вычитаем "среднее значение" (нормализация сигнала)
    start_filter = length/2 - FilterLength/2;
    if (start_filter < 0) start_filter = 0;

    end_filter = length/2 + FilterLength/2;
    if (end_filter > length) end_filter = length;

    if ( FilterLength == 0) {
        start_filter = 0;
        end_filter = length;
        FilterLength = length;
    }

    for (length_counter = start_filter; length_counter < end_filter; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            if (flag_viravnivanie != 0)
            {
                long srez_value  = sensor[sens_counter].value;

                average_value_for_sens[sens_counter] = srez_value;
            } else {
                average_value_for_sens[sens_counter] = 0;
                for (zone_count = 0; zone_count < AwerageLength/2; zone_count ++)
                {
                    average_value_for_sens[sens_counter] += rowData[length * sens_counter + start_filter + zone_count];
                    average_value_for_sens[sens_counter] += rowData[length * sens_counter + end_filter - zone_count-1];
                }
                average_value_for_sens[sens_counter] /= (AwerageLength/2)*2;
            }
        }
    }

    median_src_min =  4095; //* Amlification;
    median_src_max = -4095; //* Amlification;

    for (length_counter = 0; length_counter < length; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            cur_index = length * sens_counter + length_counter;

            long_bufer[cur_index] = rowData[cur_index] - average_value_for_sens[sens_counter];

            if ( median_src_max < long_bufer[cur_index] ) median_src_max = long_bufer[cur_index];
            if ( median_src_min > long_bufer[cur_index] ) median_src_min = long_bufer[cur_index];
        };
    };

    median_src_razmax = median_src_max - median_src_min + 1;

/*
    // Усиление
    for (length_counter = 0; length_counter < length; length_counter++) 
    {
        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            long_bufer[length * sens_counter + length_counter] =
                long_bufer[length * sens_counter + length_counter] * Amlification;
        }
    }
*/


    // фильтр поперек
    // экспоненциальное усреднение с коэф 0.2
    {
        long local_average;

        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_average = 0;

//                а это собственно и есть усреднение поперек если коментарий убрать  
//                local_average = Exponent_smooth (long_bufer[length * sens_counter + length_counter]);

                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
    };
    // посчитали фильтр поперек


/*
    // фильтр поперек
    // скользящее среднее
    {
        long WinLen = 11;

        long local_average;
        long curWinSens;


        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_average = 0;
                for ( i = 0; i < WinLen; i++)
                {
                    curWinSens =  sens_counter - WinLen / 2 + i;
                    if ( curWinSens < 0 ) curWinSens = sens_num + curWinSens;
                    if ( curWinSens >= sens_num ) curWinSens = curWinSens - sens_num;

                    local_average += long_bufer[length * curWinSens + length_counter];
                };

                local_average = local_average / WinLen;   

                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
    };
    // посчитали фильтр поперек
*/
/*
    // медиана на весь экран поперек
    {
        long *gistograma;

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos = 0;

        long tube_size = 10;
        long *tube_sum;
        long tube_counter;

        long median_amlification;

        gistograma = malloc( median_src_razmax * sizeof(long_bufer[0]) );
        tube_sum = malloc( median_src_razmax * sizeof(long_bufer[0]) );

        for ( length_counter = 0;
              length_counter < length;
              length_counter++ )
        {
            memset(gistograma, 0, median_src_razmax * sizeof(gistograma [0]));
            memset(tube_sum, 0, median_src_razmax * sizeof(tube_sum [0]));

            for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                 gistograma[ long_bufer[length * sens_counter + length_counter] - median_src_min ] ++;
            }
//   {
//      char tmp_info[1024];
//
//      sprintf (tmp_info, "size=%ld\n", size);
//      MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"Save_mem_ArrFile 4", MB_OK | MB_ICONERROR);
//   }
            for ( gist_counter = 0; gist_counter < median_src_razmax - tube_size; gist_counter++)
            {
                 for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                 {
                    tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                 }
            }

            max_gisttogram = 0;
            max_gist_pos = 0;
            for (gist_counter = 0; gist_counter < median_src_razmax; gist_counter++)
            {
                if (max_gisttogram < tube_sum[gist_counter])
                {
                    max_gisttogram = tube_sum[gist_counter];
                    max_gist_pos = gist_counter;
                }
            }

            max_gist_pos += median_src_min;

            median_amlification = Amlification / 3;
            if (median_amlification < 1) median_amlification = 1;

            for (sens_counter = 0;
                 sens_counter < sens_num;
                 sens_counter++) 
            {
                result = long_bufer[length * sens_counter + length_counter] - max_gist_pos;

                result *= median_amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            }
        } // for ( length_counter = 0;

        free(tube_sum);
        free(gistograma);

    } // закончилась медиана на весь экран поперек
*/
/*
    // медиана поперек в скользящем окне
    {

        long *gistograma;

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos;

        long tube_size = 10; // типа объединяет соседние значения гистограм в "трубку" 
        long *tube_sum;
        long tube_counter;

        long median_win_size = 35; // размер скользящего окна для вычисления медианного усреднения
        long cur_median_pos;
        long median_win_counter;

        long median_amlification;

        gistograma = malloc( median_src_razmax * sizeof(long_bufer[0]) );
        tube_sum = malloc( median_src_razmax * sizeof(long_bufer[0]) );

        median_amlification = Amlification / 3;
        if (median_amlification < 1) median_amlification = 1;

        for ( length_counter = 0;
              length_counter < length;
              length_counter++ )
        {
            for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                 memset(gistograma, 0, median_src_razmax * sizeof(gistograma [0]));
                 for ( median_win_counter = 0; median_win_counter < median_win_size; median_win_counter++)
                 {
                     cur_median_pos = sens_counter - median_win_size / 2 + median_win_counter;
                     if (cur_median_pos <  0) cur_median_pos += sens_num;
                     if (cur_median_pos >= sens_num) cur_median_pos -= sens_num;
     
                      gistograma[ long_bufer[length * cur_median_pos + length_counter] - median_src_min ] ++;
                 } // построили гистограму для окна

                 memset(tube_sum, 0, median_src_razmax * sizeof(tube_sum [0]));
                 for ( gist_counter = 0; gist_counter < median_src_razmax - tube_size; gist_counter++)
                 {
                      for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                      {
                         tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                      }
                 } // пересчитали гистограмму с учетом "трубки"

                 max_gisttogram = 0;
                 max_gist_pos = 0;
                 for (gist_counter = 0; gist_counter < median_src_razmax; gist_counter++)
                 {
                     if (max_gisttogram < tube_sum[gist_counter])
                     {
                         max_gisttogram = tube_sum[gist_counter];
                         max_gist_pos = gist_counter;
                     }
                 } // нашли максимум гистограммы
                 max_gist_pos += median_src_min; // скомпенсировали обратно наличие отрицательных значений

                 result = long_bufer[length * sens_counter + length_counter] - max_gist_pos;
                 result *= median_amlification;
                 result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                 if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                 if (result < 0) result = 0;

                 data[length * sens_counter + length_counter] =  (KRTDATA) result;
            } // for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        } // for ( length_counter = 0;

        free(tube_sum);
        free(gistograma);

    } // закончилась медиана поперек в скользящем окне
*/

    // негатив цвета на зону вычисления среднего
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        for (zone_count = 0; zone_count < AwerageLength/2; zone_count ++)
        {
            cur_index = length * sens_counter + start_filter + zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
            cur_index = length * sens_counter + end_filter - zone_count;
            data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
        }
    }


    free(average_value_for_sens);
    free(long_bufer);

    return 0;
} // int flt_k_tsentru_plus_poperek


// ***************************************************************
// функция фильтра выравнивание от центра плюс поперек
int flt_ot_tsentra_plus_poperek (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long Amlification      // параметр усиление 0-64
) {

    long* long_bufer;      // length * sens_num

    long sens_counter;
    long length_counter;

    long flag_viravnivanie=0;
    long i;

    static long average_value_for_sens=0;
    static long avrg_zone_len = 25;
    static long avrg_zone_left_shift_from_center_screen = 100;

    long zone_count;

    long cur_index;

    long result;

    long median_src_min;
    long median_src_max;
    long median_src_razmax;

    (void *) dataOrnt;

    { // проверка на изменение параметров фильтра
       static long Amlification_pred = 0;

       if ( Amlification != Amlification_pred ) 
            flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;
       Amlification_pred = Amlification;
    }

    for (i = 0; i < sens_num; i ++)
    {
       if (sensor[i].value != 0)
       {
          flag_viravnivanie = 1;
          break;
       }
    }

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));


    if (avrg_zone_len > length) avrg_zone_len = length;
    if (avrg_zone_left_shift_from_center_screen > length/2 + avrg_zone_len/2)
        avrg_zone_left_shift_from_center_screen = length - (length/2 + avrg_zone_len/2);

    median_src_min =  4095; //* Amlification;
    median_src_max = -4095; //* Amlification;

// {
//    char tmp_info[1024];
//
//    sprintf (tmp_info, "median_src_min = %ld\nmedian_src_max = %ld\nmedian_src_razmax = %ld\n",
//                        median_src_min, median_src_max, median_src_razmax);
//    MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"flt_ot_tsentra_plus_poperek _ 1", MB_OK | MB_ICONERROR);
// }

    // вычитаем "среднее значение" (нормализация сигнала)
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        if (flag_viravnivanie != 0)
        {
            long srez_value  = sensor[sens_counter].value;

            average_value_for_sens = srez_value;
        } else {
            long begin_avrg_zone = length/2 - avrg_zone_left_shift_from_center_screen - avrg_zone_len/2;

            average_value_for_sens = 0;
            for (zone_count = begin_avrg_zone;
                 zone_count < begin_avrg_zone + avrg_zone_len;
                 zone_count ++)
            {
                average_value_for_sens += rowData[length * sens_counter + zone_count];
            }
            average_value_for_sens /= avrg_zone_len;
        }

        for (length_counter = 0;
             length_counter < length;
             length_counter++) 
        {
             cur_index = length * sens_counter + length_counter;

             long_bufer[cur_index] = rowData[cur_index] - average_value_for_sens;

             if ( median_src_max < long_bufer[cur_index] ) median_src_max = long_bufer[cur_index];
             if ( median_src_min > long_bufer[cur_index] ) median_src_min = long_bufer[cur_index];
        }
    }

    median_src_razmax = median_src_max - median_src_min + 1;

//  {
//     char tmp_info[1024];
//
//     sprintf (tmp_info, "median_src_min = %ld\nmedian_src_max = %ld\nmedian_src_razmax = %ld\n",
//                         median_src_min, median_src_max, median_src_razmax);
//     MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"flt_ot_tsentra_plus_poperek _ 2", MB_OK | MB_ICONERROR);
//  }

    // Усиление
//    for (length_counter = 0;
//         length_counter < length;
//         length_counter++) 
//    {
//        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++) {
//            long_bufer[length * sens_counter + length_counter] =
//                long_bufer[length * sens_counter + length_counter] * Amlification; // / 10;
//        };
//    };


    // фильтр поперек
    // экспоненциальное усреднение с коэф 0.2
    {
        long local_average;

        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_average = 0;

//                а это собственно и есть усреднение поперек если коментарий убрать  
//                local_average = Exponent_smooth (long_bufer[length * sens_counter + length_counter]);

                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
    };
    // посчитали фильтр поперек

/*
    // фильтр поперек
    // скользящее среднее
    {
        long WinLen = 11;

        long local_average;
        long curWinSens;


        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                local_average = 0;
                for ( i = 0; i < WinLen; i++)
                {
                    curWinSens =  sens_counter - WinLen / 2 + i;
                    if ( curWinSens < 0 ) curWinSens = sens_num + curWinSens;
                    if ( curWinSens >= sens_num ) curWinSens = curWinSens - sens_num;

                    local_average += long_bufer[length * curWinSens + length_counter];
                };

                local_average = local_average / WinLen;   

                // вычтем постоянную составляющюю
                result = long_bufer[length * sens_counter + length_counter] - local_average;

                result *= Amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            };
        };
    };
    // посчитали фильтр поперек
*/

/*
    // медиана на весь экран поперек
    {
        long *gistograma;

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos = 0;

        long tube_size = 10;
        long *tube_sum;
        long tube_counter;

        long median_amlification;

        gistograma = malloc( median_src_razmax * sizeof(long_bufer[0]) );
        tube_sum = malloc( median_src_razmax * sizeof(long_bufer[0]) );

        for ( length_counter = 0;
              length_counter < length;
              length_counter++ )
        {
            memset(gistograma, 0, median_src_razmax * sizeof(gistograma [0]));
            memset(tube_sum, 0, median_src_razmax * sizeof(tube_sum [0]));

            for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                 gistograma[ long_bufer[length * sens_counter + length_counter] - median_src_min ] ++;
            }

            for ( gist_counter = 0; gist_counter < median_src_razmax - tube_size; gist_counter++)
            {
                 for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                 {
                    tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                 }
            }

            max_gisttogram = 0;
            max_gist_pos = 0;
            for (gist_counter = 0; gist_counter < median_src_razmax; gist_counter++)
            {
                if (max_gisttogram < tube_sum[gist_counter])
                {
                    max_gisttogram = tube_sum[gist_counter];
                    max_gist_pos = gist_counter;
                }
            }

            max_gist_pos += median_src_min;

            median_amlification = Amlification / 3;
            if (median_amlification < 1) median_amlification = 1;

            for (sens_counter = 0;
                 sens_counter < sens_num;
                 sens_counter++) 
            {
                result = long_bufer[length * sens_counter + length_counter] - max_gist_pos;

                result *= median_amlification;

                result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[length * sens_counter + length_counter] =  (KRTDATA) result;
            }
        } // for ( length_counter = 0;

        free(tube_sum);
        free(gistograma);

    } // закончилась медиана на весь экран поперек
*/
/*
    // медиана поперек в скользящем окне
    {

        long *gistograma;

        long gist_counter;
        long max_gisttogram;
        long max_gist_pos;

        long tube_size = 10; // типа объединяет соседние значения гистограм в "трубку" 
        long *tube_sum;
        long tube_counter;

        long median_win_size = 35; // размер скользящего окна для вычисления медианного усреднения
        long cur_median_pos;
        long median_win_counter;

        long median_amlification;

        gistograma = malloc( median_src_razmax * sizeof(long_bufer[0]) );
        tube_sum = malloc( median_src_razmax * sizeof(long_bufer[0]) );

        median_amlification = Amlification / 3;
        if (median_amlification < 1) median_amlification = 1;

        for ( length_counter = 0;
              length_counter < length;
              length_counter++ )
        {
            for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                 memset(gistograma, 0, median_src_razmax * sizeof(gistograma [0]));
                 for ( median_win_counter = 0; median_win_counter < median_win_size; median_win_counter++)
                 {
                     cur_median_pos = sens_counter - median_win_size / 2 + median_win_counter;
                     if (cur_median_pos <  0) cur_median_pos += sens_num;
                     if (cur_median_pos >= sens_num) cur_median_pos -= sens_num;
     
                      gistograma[ long_bufer[length * cur_median_pos + length_counter] - median_src_min ] ++;
                 } // построили гистограму для окна

                 memset(tube_sum, 0, median_src_razmax * sizeof(tube_sum [0]));
                 for ( gist_counter = 0; gist_counter < median_src_razmax - tube_size; gist_counter++)
                 {
                      for ( tube_counter = 0; tube_counter < tube_size; tube_counter++)
                      {
                         tube_sum[gist_counter + tube_size/2] += gistograma[gist_counter + tube_counter];
                      }
                 } // пересчитали гистограмму с учетом "трубки"

                 max_gisttogram = 0;
                 max_gist_pos = 0;
                 for (gist_counter = 0; gist_counter < median_src_razmax; gist_counter++)
                 {
                     if (max_gisttogram < tube_sum[gist_counter])
                     {
                         max_gisttogram = tube_sum[gist_counter];
                         max_gist_pos = gist_counter;
                     }
                 } // нашли максимум гистограммы
                 max_gist_pos += median_src_min; // скомпенсировали обратно наличие отрицательных значений

                 result = long_bufer[length * sens_counter + length_counter] - max_gist_pos;
                 result *= median_amlification;
                 result = KRT_PALLETE_SIZE/2 + (result * KRT_PALLETE_SIZE) / 4096;

                 if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                 if (result < 0) result = 0;

                 data[length * sens_counter + length_counter] =  (KRTDATA) result;
            } // for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        } // for ( length_counter = 0;

        free(tube_sum);
        free(gistograma);

    } // закончилась медиана поперек в скользящем окне
*/

    // негатив цвета на зону вычисления среднего
    for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    {
        for (zone_count = length/2 - avrg_zone_len/2 - avrg_zone_left_shift_from_center_screen;
             zone_count < length/2 + avrg_zone_len/2 - avrg_zone_left_shift_from_center_screen;
             zone_count ++)
        {
            cur_index = length * sens_counter + zone_count;
                if (flag_viravnivanie == 0) data[cur_index] = data[cur_index] + KRT_PALLETE_SIZE / 10;
              if (data[cur_index] >= KRT_PALLETE_SIZE) data[cur_index] = KRT_PALLETE_SIZE - 1;
        }
    }

    free(long_bufer);

    return 0;
} // int flt_ot_tsentra_plus_poperek



//------------------------------------------------------------------------------------------------------------
// Функция формирует правую половину свертки "Мексиканская шляпа"
// с положительными индексами, этого достаточно, т.к. вторая половина
// с отрицательными индексами зеркальна.

//                                    1                     / t \ ^ 2    /     / t \ ^ 2\
// Базовая функция такая  W(t,a) = --------  * exp (-0.5 * ( --- )    * ( 1 - ( --- )    )
//                                  sqrt(a)                 \ a /        \     \ a /    /
// где t - время сигнала , а - масштабный коэффициет (меняет частоту сигнала)
                              
#define BASE_HALF_SIZE_CONVOLUTION (long) 6 // эталонный размер половины свертки, при а == 1

int get_MHAT_half_convolution (long required_half_size_convolution, // требуемый размер половины свертки
                               double *half_size_convolution_array) // массив с весовыми коэффициентами
{
    long MHAT_index = 0;
    double result;

    double scale_multiplier = 1.0 * required_half_size_convolution / BASE_HALF_SIZE_CONVOLUTION;

    for ( MHAT_index = 0;
          MHAT_index <= required_half_size_convolution;
          MHAT_index ++
        )
    {
          result = 1.0 / sqrt(scale_multiplier) *
                   exp ( ((double) MHAT_index * MHAT_index) / scale_multiplier / scale_multiplier / (-2) ) *
                   ( 1.0 - ((double) MHAT_index * MHAT_index) / scale_multiplier / scale_multiplier );

          half_size_convolution_array[MHAT_index] = result;
    }

    return 0;
}


double convolution_array[65536];

// ***************************************************************
// фильтр MHAT
int flt_MHAT (
KRTDATA* data,         // указатель на массив обработанных данных страницы развертки
KRTROW *rowData,       // указатель на буфер сырых данных для страницы
long *dataOrnt,        // указатель на вектор ориентации страничного буфера
long length,           // размер массива по X
long sens_num,         // размер массива по Y(количество датчиков)
T_SENSOR *sensor,      // указатель на вектор настроек датчиков. для доступа к элементам базовой линии: crz->sensor[i].delta
long Amlification,     // параметр усиление 0-64
long FilterLength,     // ширина окна фильтра (1-500 в отсчетах одометра, default = 100)
long AwerageLength     // not use (1-50, default =25)
) {
    long* long_bufer;    // length * sens_num

    long sens_counter;
    long length_counter;
    long MHAT_counter;

    long cur_index;
    double MHAT_sum;

    long result;

    (void *) dataOrnt;
    (void *) sensor;
    (void *) AwerageLength;

    if (FilterLength < 4) FilterLength = length/2;

    { // проверка на изменение параметров фильтра
        static long Amlification_pred = 0;
        static long FilterLength_pred = 0;
        static long AwerageLength_pred = 0;

        if ( ( Amlification != Amlification_pred ) ||
             ( FilterLength != FilterLength_pred ) ||
             ( AwerageLength != AwerageLength_pred )
           ) flag_change_parameters_filter = FLAG_CHANGE_PARAMETER_OF_FILTER_UP;

        Amlification_pred = Amlification;
        FilterLength_pred = FilterLength;
        AwerageLength_pred = AwerageLength;
    } // // проверили на изменение параметров фильтра

    long_bufer = malloc(length * sens_num * sizeof(long_bufer[0]));

    { // наложение MHAT вдоль датчиков
        long left_point;
        long right_point;

        if ( Amlification < 4) Amlification = 4;

        get_MHAT_half_convolution (Amlification /*FilterLength*/, convolution_array);

        for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
        {
            for (length_counter = 0; length_counter < length; length_counter++)
            {
                cur_index = length * sens_counter + length_counter;

                MHAT_sum = convolution_array[0] * rowData[cur_index];

                for (MHAT_counter = 1; MHAT_counter <= Amlification /* FilterLength */; MHAT_counter ++)
                {
                    left_point = length_counter - MHAT_counter;
                    if (left_point < 0) left_point = 0;

                    right_point = length_counter + MHAT_counter;
                    if (right_point >= length) right_point = length - 1;

                    MHAT_sum += convolution_array[MHAT_counter] *
                          (rowData[length * sens_counter + left_point] + rowData[length * sens_counter + right_point]);
                } // for (MHAT_counter = 1; MHAT_counter < FilterLength; MHAT_counter ++)


                long_bufer[cur_index] = (long) (MHAT_sum * 2 /*Amlification*/); // резултат свертки и Усиление

            } // for (length_counter = 0; length_counter < length; length_counter++) 
        } // for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
    } // наложили MHAT вдоль датчиков

    // приведение к 200 градаций палитры
    {
        for (length_counter = 0; length_counter < length; length_counter++) 
        {
            for (sens_counter = 0; sens_counter < sens_num; sens_counter ++)
            {
                cur_index = length * sens_counter + length_counter;

                result = KRT_PALLETE_SIZE/2 + (long_bufer[cur_index] * KRT_PALLETE_SIZE) / 4096;

                if (result >= KRT_PALLETE_SIZE) result = KRT_PALLETE_SIZE-1;
                if (result < 0) result = 0;

                data[cur_index] =  (KRTDATA) result;
            };
        };
    };
    // привели к 200 градаций палитры

    free(long_bufer);

    return 0;
} // int flt_MHAT (
