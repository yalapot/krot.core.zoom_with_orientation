/*
������ ��������� ��������� ��������.
*/
#include "krotw32.h"
#include "filter.h"

#include "debuglog.h"

#include <math.h>

// LOG_1 - ������ ����� ���������� ����������
//#define LOG_1

// LOG_BASE_LINE - ������� ������� �����
//#define LOG_BASE_LINE

short ScaleMode = 0;      //������ ��� ������������ ����� ��� ���������������: 0 - ������������ ��������
                          //                                                   1 - ����������� ��������
                          //                                                   2 - ������� ��������
                          //                                                   3 - ��������� ��� ������ ���������� ��������
short OldScaleMode = 0;

void krotGraphBlt (
  T_CRZSENS *crz, 
  KRTDATA *data,
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
  long top
) {
HDC hdc, dc;
HBRUSH brush;
RECT r;
HPEN pen1, pen2;
long i, j, scrollPixels, x, yPlace, y0, evenFlag;
double y, k;

 GetClientRect(pic, &r);
 dc = GetDC(pic);

 hdc = CreateCompatibleDC(dc);
 SelectObject(hdc, bmp);

 brush = CreateSolidBrush(crz->vbGraphs.clrBackGround);
 SelectObject(hdc, brush);
 Rectangle(hdc, bmpXstart-1, r.top-1, bmpXend+1, r.bottom+1); 
 pen1 = CreatePen(PS_SOLID, 0, crz->vbGraphs.clrEven);
 pen2 = CreatePen(PS_SOLID, 0, crz->vbGraphs.clrOdd);
 SelectObject(hdc, pen1);
 DeleteObject(brush);

 y0 = KRT_PALLETE_SIZE / 2;
 k = crz->vbGraphs.amplif;
 k = k * crz->vbGraphs.gap / KRT_PALLETE_SIZE;
 scrollPixels = bmpXend - bmpXstart;
 evenFlag = 0;

 for (j = 0; j < datY; j += crz->vbGraphs.gap) {

  yPlace = (j + (datY - top)) % datY ;
  yPlace = yPlace * bmpY / datY;

  y = yPlace - (data[j*datX] - y0) * k;
  MoveToEx(hdc, bmpXstart, (int) y, NULL); 
  evenFlag ^= 1;
  SelectObject(hdc, (evenFlag ? pen1 : pen2));

  for (i=1; i<datX; i++) {
   y = yPlace - (data[j*datX + i] - y0) * k;
   x = i * scrollPixels / datX;
   LineTo(hdc, bmpXstart+x, (int) y); 
  }
  // last segment - need border data column
  x = i * scrollPixels / datX;
  LineTo(hdc, bmpXstart+x, (int) y); 
 }

 DeleteDC(hdc);
 DeleteObject(pen1);
 DeleteObject(pen2);
 GetBitmapBits(bmp, bmpX * bmpY * bytesPerPixel, bmpBuffGrafh);
 ReleaseDC(pic, dc);

 for (i=0; i < bmpY; i++) {
  j= (i * bmpX + bmpXstart) * bytesPerPixel;
  memmove(((BYTE *) bmpBuff)+j, ((BYTE *) bmpBuffGrafh)+j, scrollPixels * bytesPerPixel);
 }

 return;
} // void krotGraphBlt (

void krotStretchBlt (
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
) {
long bmpXout, bx, by, xStart, xEnd, yStart, yEnd, dx, dy, bi, compressX, compressY;
KRTDATA dataVal, dXdY;
int tmp, tmpVal, count;     //tmpVal � count ��� �������� �������� ��� �������� ScaleMode == 2

long * long_buffer;

    (void) ornt;

    bmpXout = bmpXend - bmpXstart;

    // ��������� ������� ������ ������ � ������ ������ ����� �����.
    // ����� ������ ��� ��������� ������� ��������� ��� ������� �������� ������ (���� 1:64)
    // ����� ������ �������� �� ����.
    if (bmpXout == 0) return;

    if (crz->drawMode == ZOOM_LINES) {
        krotGraphBlt(crz, data, datX, datY, bmpBuff, bmpBuffGrafh, pic, bmp, bmpX, bmpY, bmpXstart, bmpXend, top);
        return;
    }

    long_buffer = malloc(bmpX * bmpY * sizeof(long_buffer[0]));

    // ��������� �������� �� X � Y
    compressY = datY > bmpY ? datY / bmpY -1 : 0;
    compressX = datX > bmpXout ? datX / bmpXout -1 : 0;

    // ���� �� �������� �������
    for (bx=bmpXstart; bx<bmpXend; bx++) {
        // ���� �� ������� �������
        for (by=0; by<bmpY; by++) {
            dataVal = 0; // �������� ������ ��� ������� bx:by �� ���������

            //��� �������� �������� ��� ScaleMode == 2
            count = 0;  
            tmpVal=0;

            // ��������� ������ �� X ��� ������� bx:by � ������� ������
            xStart = datX * (bx - bmpXstart) /  bmpXout;
            // �������� ������ �� Y ��� ������� bx:by � ������� ������
            xEnd = xStart + compressX;

            // ���� �� �������� ������� ������, ��������������� ������� bx:by
            for (dx=xStart; dx<=xEnd; dx++) {
                // ��������� ������ �� Y ��� ������� bx:by � ������� ������
                yStart = datY * by /  bmpY + top;
                // �������� �� ����������� ������� ����� ������� ���-�� ��������
                yStart = (yStart >= datY) ? yStart - datY : yStart;
                // �������� ������ �� Y ��� ������� bx:by � ������� ������
                yEnd = yStart + compressY;
            	if ((ScaleMode==1)||(ScaleMode==3)) dataVal = (KRTDATA) data[datX * yStart + dx];
            	//������ �� ��������� ��� ���������� ������������ �������� ��� ��������������� � ScaleMode == 1 ��� 3
                // ���� �� ������� ������ � ������� ������, ��������������� ������� bx:by
                for (dy=yStart; dy<=yEnd; dy++) {
                    bi = dy;
                    bi = (bi >= datY) ? bi -datY : bi;

//                    if (smooth == 0) {

                        // ������� ������� ������
                        dXdY = (KRTDATA) data[datX * bi + dx];

                        // �������� �� ������� �����, ���� ��� ������ � �� ������� ������
                        if ((crz->vbFilter.active == 0) && (crz->sensor[bi].delta != 0)) {
                          tmp = dXdY + crz->sensor[bi].delta;
                          if (tmp < 0) {
                            dXdY = 0; 
                          } else if (tmp > (KRT_PALLETE_SIZE - 1)) {
                            dXdY = KRT_PALLETE_SIZE - 1;
                          } else {
                            dXdY = (KRTDATA) tmp;
                          }
                        }
//                    }

                    // �������� ����� ��� ������� ������ ���������������
                    switch (ScaleMode) {
                      case 0: {dataVal = (KRTDATA) ((dXdY > dataVal) ? dXdY : dataVal); break;}   //������������ ��������
                      case 1: {dataVal = (KRTDATA) ((dXdY < dataVal) ? dXdY : dataVal); break;}   //����������� ��������
                      case 2: {tmpVal = tmpVal+dXdY; count++; break;}                             //������� ��������
                      case 3: {dataVal = (KRTDATA) dataVal; break;}                               //������ ����������
                    }
                }
            }

            // ��������� ��������� �������� ������ � ����� �������
            if (ScaleMode==2) dataVal=(KRTDATA)(tmpVal/count); //����������� ������� ��������
         	
            long_buffer[bmpX * by + bx] = dataVal;
        }
    }

    if (smooth == 1)
    { // ��������� ���������� �����������
         long sens_counter;
         long length_counter;

         double k = 1;

         #pragma warning(disable : 4204)  // ����� ������� ��� �����������
                                      // ��� ������������� �������
         // ����������� 13x13
         #define matr_size 13
         #define matr_kub (matr_size * matr_size - 40)
         double matrix[matr_size][matr_size]
                         = {{          0,          0,          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0,          0,          0 },
                            {          0,          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0,          0 },
                            {          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0 },
                            {          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0 },
                            { k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub },
                            { k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub },
                            { k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub },
                            { k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub },
                            { k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub },
                            {          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0 },
                            {          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0 },
                            {          0,          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0,          0 },
                            {          0,          0,          0,          0, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub, k/matr_kub,          0,          0,          0,          0 } };
         #pragma warning(default : 4204)

         double* Double_bufer;  // length * sens_num
         long matr_x, matr_y;
         double svertka;
         // ����������� ������ �������� �������

         Double_bufer = malloc(bmpX * bmpY * sizeof(Double_bufer[0]));

         for (length_counter = matr_size/2;
              length_counter < bmpX - matr_size/2;
              length_counter++) 
         {
             for (sens_counter = matr_size/2;
                  sens_counter < bmpY - matr_size/2;
                  sens_counter ++)
             {
                 svertka = 0;
                 for ( matr_x = 0; matr_x < matr_size; matr_x++)
                 {
                     for ( matr_y = 0; matr_y < matr_size; matr_y++)
                     {
                         svertka += 
                          (double) long_buffer[ bmpX * (sens_counter + matr_y - matr_size/2)
                                      + length_counter + matr_x - matr_size /2]
                          *
                          matrix [matr_x][matr_y]; 
                     };
                 };
                 Double_bufer[bmpX * sens_counter + length_counter] = svertka;
             };
         };

         for (length_counter = matr_size /2;
              length_counter < bmpX - matr_size /2;
              length_counter++) 
         {
             for (sens_counter = matr_size /2;
                  sens_counter < bmpY - matr_size /2;
                  sens_counter ++)
             {
                 long_buffer[bmpX * sens_counter + length_counter] = (long)
                    Double_bufer[bmpX * sens_counter + length_counter];
             };
         };

         free(Double_bufer);
    } // ���������� �����������

    for (bx=bmpXstart; bx<bmpXend; bx++) {
        // ���� �� ������� �������
        for (by=0; by<bmpY; by++) {
            putPixel(bmpBuff, bmpX * by + bx, long_buffer[bmpX * by + bx], pal);
        }
    }
    free(long_buffer);
} // void krotStretchBlt (

/*
������� ������� ������ �������� ��������� �� ��������������� �������
������ ��������
*/
void showBmp(T_CRZSENS *crz, HWND hWnd) {
HDC hdcSrcOld, hdcSrc, hdcDest;

 hdcDest = GetDC(hWnd);
 hdcSrc = CreateCompatibleDC(hdcDest);
 hdcSrcOld = SelectObject(hdcSrc, crz->bmp);
 BitBlt (hdcDest, 0, 0, crz->pixelX, crz->pixelY, hdcSrc, 0, 0, SRCCOPY);
 SelectObject(hdcSrc, hdcSrcOld);
 DeleteDC(hdcSrc);
 ReleaseDC(hWnd, hdcDest);
}

/*
������� ������� ������ ���������
*/
short readData(
    T_TRACE *trc,
    long crzIndx,
    long dStart,
    long dLength,
    KRTDATA *buf,
    KRTROW *row,

    // ��� ��������� ��� #KRT_APIVER_3
    long *orient,                   // ��������� �� ������ ����������
    T_DimensionPixel *bendingPlane, // ��������� �� ������ � ��������� ������
    T_DimensionPixel *odomSens      // ��������� �� ������ � ���������� ��������
)
{
T_CRZSENS *crz;
long arrSize;

 crz = &(trc->crz[crzIndx]);
 // �������� ������� ������
 arrSize = crz->sNum * dLength;
 memset(buf, 0, arrSize * sizeof(KRTDATA));
 memset(row, 0, arrSize * sizeof(KRTROW));
 
////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "readData crzIndx %ld %ld:%ld (page %ld)", 
   crzIndx, 
   dStart,
   dLength,
   crz->pageDat 
 );
 Log();
#endif

    row[0] = (long) orient;
    row[1] = (long) bendingPlane;
    row[2] = (long) odomSens;

 /* ��������� � �������� ������ � ��������� */
 if ((*(trc->record.krtDrvFillData))(trc->vbHandle, crzIndx, dStart, dLength, buf, row) == KRT_ERR) {
  sprintf (lastError, "������ ��� ������ krtDrvFillData %s", driverError(trc));
  return 1;
 }

 return 0;

} // short readData(T_TRACE *trc, long crzIndx, long dStart, long dLength, KRTDATA *buf, KRTROW *row, long *orient) {

/*
������� ����������� ���� �������� ������� � ����� ����� ������� � ������ �������� � ������� ����������
*/
void degree2sens(
 long *ornt,          // ������ ����������
 long vLen,           // ����� �������
 long sensNum         // ����� ���-�� �������� 
 ) {
long i;

 for (i=0; i<vLen; i++) {
    ornt[i] = (long) (ornt[i] * sensNum / ORNT_MAX_VAL);
 }
}

 // use orientation for bufers 
void use_orientation_for_bufers (
   KRTDATA *data,                    // ��������� �� ������ ������������ ������ �������� ���������
   KRTROW *rowData,                  // ��������� �� ����� ����� ������ ��� ��������
   long *dataOrnt,                   // ��������� �� ������ ���������� ����������� ������
   long length,                      // ������ ������� �� X
   long sens_num,                    // ������ ������� �� Y (���������� ��������)
   T_TRACE *trc,                     // ���������� �� �������� ������
   T_DimensionPixel *bendingPlane,   // ��������� �� ������ � ��������� ������
   T_DimensionPixel *odomSens        // ��������� �� ������ � ���������� ��������
   
){
  long sens_counter;
  long length_counter;
  long new_sens_ornt;

  long *copy_dimension;

/*
      {  // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
          char tmp_info[10240];

          sprintf (tmp_info, "trc->drv.apiVer = %ld\ntrc->Orientation_OFF = %ld\nlength = %ld\n ", trc->drv.apiVer, trc->Orientation_OFF, length);
          MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"use_orientation_for_bufers", MB_OK | MB_ICONERROR);
      } // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
*/

  if (trc->drv.apiVer != KRT_APIVER_3 || trc->Orientation_OFF != 0) return;

/*
      {  // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
          char tmp_info[65536];
          long i;
          char num_txt[128];

          sprintf (tmp_info, "length = %ld\n ", length);
          for ( i = 0; i < 200; i ++)
          {
              sprintf(num_txt, ", %ld", dataOrnt[i]);
              strcat(tmp_info, num_txt);
          }
          MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"use_orientation_for_bufers", MB_OK | MB_ICONERROR);
      } // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
*/
  if (dataOrnt == NULL) return;

  copy_dimension = malloc(sens_num * sizeof(long));

  // ���������� �������� ��������� �������� ������� ���������� � ������ ���������
  // ��� ���� ��������� ����� ������� (��� length_counter = 0 �� �������), ��� ����
  // ����� ��������� ������� ����� "������������"
  for (length_counter = 1; length_counter < length; length_counter++) {
     for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++) { 
          copy_dimension[sens_counter] = data[length * sens_counter + length_counter];
     }

     for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++) { 
         new_sens_ornt = sens_counter + dataOrnt[length_counter];
         while ( new_sens_ornt < 0) new_sens_ornt += sens_num;
         while ( new_sens_ornt >= sens_num) new_sens_ornt -= sens_num;

         data[length * new_sens_ornt + length_counter] = (KRTDATA) copy_dimension[sens_counter];
     }
  }

  // ���������� �������� ��������� �������� ������� ���������� � ����� ������
  for (length_counter = 0; length_counter < length; length_counter++) {
     for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++) { 
          copy_dimension[sens_counter] = rowData[length * sens_counter + length_counter];
     }

     for ( sens_counter = 0; sens_counter < sens_num; sens_counter ++) { 
         new_sens_ornt = sens_counter + dataOrnt[length_counter];
         while ( new_sens_ornt < 0) new_sens_ornt += sens_num;
         while ( new_sens_ornt >= sens_num) new_sens_ornt -= sens_num;

         rowData[length * new_sens_ornt + length_counter] = (KRTROW) copy_dimension[sens_counter];
     }
  }

  free(copy_dimension);

  if (bendingPlane != NULL) { // ��������� �� ��������� ������ bending Plane
       long line_thick;
       long bending_plane_beg_sens;

       for (length_counter = 0; length_counter < length; length_counter++) {
           if ( bendingPlane[length_counter].color > 0 ) {
               bending_plane_beg_sens = bendingPlane[length_counter].sens;

               bending_plane_beg_sens += sens_num /4;
               bending_plane_beg_sens -= sens_num / 20;

               for (line_thick = 0; line_thick < 8; line_thick ++ ) {
                   if (line_thick == 4) {
                       bending_plane_beg_sens += sens_num / 10 - 4;
                   }
                   bending_plane_beg_sens += 1;

                   // ������ �����
                   if (bending_plane_beg_sens < 0) bending_plane_beg_sens = sens_num + bending_plane_beg_sens;
                   if ( bending_plane_beg_sens >= sens_num ) bending_plane_beg_sens -= sens_num;

                   data[length * bending_plane_beg_sens + length_counter]
                       = (KRTDATA) bendingPlane[length_counter].color;

                   // ��������������� �����
                   bending_plane_beg_sens += sens_num / 2;

                   if ( bending_plane_beg_sens >= sens_num ) bending_plane_beg_sens -= sens_num;

                   data[length * bending_plane_beg_sens + length_counter]
                       = (KRTDATA) bendingPlane[length_counter].color;

               } // for (line_thick = 0; line_thick < 8; line_thick ++ ) {
           } // if ( bendingPlane[length_counter].color > 0 )
       } // for (length_counter = 0; length_counter < length; length_counter++)
  } // ����������� ��������� �� ��������� ������ bending Plane

  // ��������� �� ��������� ������ � ���������� ��������
  if (odomSens != NULL) {
       long odom_orient;

       for ( length_counter = 0; length_counter < length; length_counter++ ) {
           if ( odomSens[length_counter].color > 0 ) {
               odom_orient = odomSens[length_counter].sens + dataOrnt[length_counter];
               if ( odom_orient < 0) odom_orient += sens_num;
               if ( odom_orient >= sens_num) odom_orient -= sens_num;

               data[length * odom_orient + length_counter] = (KRTDATA) odomSens[length_counter].color;
           }
       }
  } // ����������� ��������� �� ��������� ������ � ���������� ��������

} // void use_orientation_for_bufers (


void fillBitmapPage (T_CRZSENS *crz) {
 krotStretchBlt (
  crz, 
  crz->dat0PageBuff,
  crz->vbScreen.orntOff ? NULL : crz->datPageOrnt,
  crz->pageDat,
  crz->sNum - crz->hide,
  crz->bmpBuff,
  crz->bmpBuffScroll,
  crz->pic,
  crz->bmp,
  crz->pixelX,
  crz->pixelY,
  0,
  crz->pixelX,
  crz->topSens,
  &(crz->pal), 0 );
}

#define PI  3.14159265359

long Calculate_profil_mm(long src_data, long prof_sens_num, T_CRZSENS* crz)
{
    double translate_res;

    translate_res = src_data;

    if (crz->profil_sens_type == ENCODER_SENS)
    {
         // ������� ��� ������� ���������� ����������� ������� �������:
         // � ������� ����������� ��� ��������� (��������� ����� ��������)
         // crz->length_sens ������������ ���������� � �� (������:
         //                                    ��� ������� ��������;
         //                                    oc� ��������������� ������ �������)
         // Zerro_angle_C (� ��������) ���� ����� ���� �������
         //                            � ������� ������� � ������� ������� ���������
         //                          (�.�. ������ ����������� � ���� ��������� �������)
         double Zerro_angle_rad = ((crz->zero_angle_gradus * 2 * PI) / 360); // ���� ����� ���� ������� � ������� ������� � ������� ������� ��������� ������������ � �������

         translate_res = translate_res
             - crz->profil_calibrate[prof_sens_num][0]
             + crz->profil_calibrate[0][0];

         translate_res = crz->length_sens *
             (sin(Zerro_angle_rad) - sin(Zerro_angle_rad - (2 * PI) / 4095 * (1000 - translate_res)));

         //  #define PROFIL_MM_SHIFT_UP     0
         //  #define PROFIL_MM_INVERSE      0
         translate_res += 0;  //    translate_res += PROFIL_MM_SHIFT_UP;

         if (translate_res >= KRT_PALLETE_SIZE) translate_res = KRT_PALLETE_SIZE - 1;
         if (translate_res < 0) translate_res = 0;

         //    if (PROFIL_MM_INVERSE > 0) translate_res = PROFIL_MM_INVERSE - translate_res;
         return (long) translate_res;
    } else {
         // ��� �������� ��� ����������� �������
         long prof_calibr_index;
         long calibr_delta;
         long calibr_scale;

         if (crz->profil_row_inverse > 0) src_data = crz->profil_row_inverse - src_data;

         translate_res = src_data;

         prof_calibr_index=0;
         while (translate_res < (crz->profil_calibrate[prof_sens_num][prof_calibr_index]) )
         {
             prof_calibr_index++;
         }; //while (translate_res ...

         if (prof_calibr_index <= 1) {
             translate_res = 0; //translate_res = PROFIL_MM_SHIFT_UP;
         } else {
             translate_res = 10 * (prof_calibr_index);

             calibr_delta = crz->profil_calibrate[prof_sens_num][prof_calibr_index - 1] - src_data;
             calibr_scale = crz->profil_calibrate[prof_sens_num][prof_calibr_index - 1] - crz->profil_calibrate[prof_sens_num][prof_calibr_index];
             translate_res += 10 * calibr_delta / calibr_scale;
             
//             translate_res += 20; //translate_res += PROFIL_MM_SHIFT_UP;
         };

         if ( translate_res >= KRT_PALLETE_SIZE) translate_res = KRT_PALLETE_SIZE - 1;
         if ( translate_res <   0 ) translate_res =   0;

//         if (PROFIL_MM_INVERSE > 0) translate_res = PROFIL_MM_INVERSE - translate_res;
         return (long) translate_res;
    }
}


void switch_filters(T_TRACE *trc, long crzIndx)
{
 T_CRZSENS *crz = &(trc->crz[crzIndx]);

/*
 {
     char tmp_info[10240];

     sprintf (tmp_info, "crz->vbFilter.active = %ld\n trc->drv.apiVer = %ld\n",
              crz->vbFilter.active, trc->drv.apiVer);
     MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"1", MB_OK | MB_ICONERROR);
 }
*/

 if ( crz->sType == SENS_TYPE_PROFIL && crzIndx == 0 )
 {
      long sens_counter;
      long length_counter;
      long screen_pos;

      static long profil_crz_call_counter = 0;

      screen_pos = 0;
      for (sens_counter = 0; sens_counter < crz->sNum; sens_counter++) {
          for (length_counter = 0; length_counter < crz->pageDat; length_counter++) {
              screen_pos = crz->pageDat * sens_counter + length_counter;
              crz->dat0PageBuff[screen_pos] = (KRTDATA)Calculate_profil_mm(crz->datPageBuff[screen_pos], sens_counter, crz);
          } //for (length_counter = 0; length_counter < crz->pageDat; length_counter++) {
      } // for (sens_counter = 0, sens_counter < crz->profil_sens_quantity; sens_counter++) {

      profil_crz_call_counter++;
 } else { // if (crz->sType == SENS_TYPE_PROFIL)
/*
      {  // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
          char tmp_info[10240];

          sprintf (tmp_info, "crz->vbFilter.active = %ld\n", crz->vbFilter.active);
          MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"2", MB_OK | MB_ICONERROR);
      } // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
*/

     switch (crz->vbFilter.active) {

       case 0: // ��� �������

           if (trc->drv.apiVer != KRT_APIVER_3 ) break;

           if (crz->Amplification_dflt <= 0) crz->Amplification_dflt = 10;

           fltExponent (
               crz->dat0PageBuff, // ��������� �� ������ ������������ ������ �������� ���������
               crz->datPageBuff,  // ��������� �� ����� ����� ������ ��� ��������
               crz->datPageOrnt,  // ��������� �� ������ ���������� ����������� ������
               crz->pageDat,      // ������ ������� �� X
               crz->sNum,         // ������ ������� �� Y (���������� ��������)

    //           crz->sensor,     // ��������� �� ������ �������� ��������. ��� ������� � ��������� ������� �����: crz->sensor[i].delta

               14, // ������� ����������� ������� (�������� �� 0 �� 100)
               crz->Amplification_dflt  // �������� �������� ������� (�������� �� 1 �� 100)
           );

           break;


       case 1: // cheshka
           fltExponent (
               crz->dat0PageBuff, // ��������� �� ������ ������������ ������ �������� ���������
               crz->datPageBuff,  // ��������� �� ����� ����� ������ ��� ��������
               crz->datPageOrnt,  // ��������� �� ������ ���������� ����������� ������
               crz->pageDat,      // ������ ������� �� X
               crz->sNum,         // ������ ������� �� Y (���������� ��������)

    //           crz->sensor,     // ��������� �� ������ �������� ��������. ��� ������� � ��������� ������� �����: crz->sensor[i].delta

               14, // ������� ����������� ������� (�������� �� 0 �� 100)
               10  // �������� �������� ������� (�������� �� 1 �� 100)
           );
    /*     fltUnweld(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum, 
           crz->vbFilter.unweldParam
         );
    */
         break;

       case 2: // tselnotyanutaya
         fltRolled(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum, 
           crz->vbFilter.rolledParam1, 
           crz->vbFilter.rolledParam2,
           crz->vbFilter.rolledAmplifer
         );
         break;

       case 3: // prigruz
         fltPrigruz(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum
         );
         break;

       case 4: // prodol treschiny
         fltPoligon(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum, 
           crz->vbFilter.tfiParam1, 
           crz->vbFilter.tfiParam2,
           crz->vbFilter.tfiBase
         );
         break;

       case 5: // no mathematic
         fltNo_math(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           crz->sensor
         );
         break;

       case 6: // ������ � ������ //2021 // volosok
    //     flt_k_tsentru_plus_poperek (
         flt_filament(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam,
           crz->vbFilter.tfiParam1,
           crz->vbFilter.tfiParam2
         );
         break;

       case 7: //������ �� ������ // ������ 2019
    //     flt_ot_tsentra_plus_poperek (
         flt_Filter2019(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 8: // ������� 1 // ���������
         flt_convolution_1(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 9: // ������� 2 // ��������
         flt_Skolz_plus_poperek( // ���������� ������� ����� ����� �������
    //     flt_Cut_big_small( // ������� �� ��������� ������� � ������������� ��������
    //     flt_convolution_2(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 10: // ��������� ��������
    //     flt_MHAT ( // MHAT ������� ���������� �� ������ ������
         fltMedianFullScreen ( // ��������� ����������
    //     flt_convolution_3( // ������� 3 �������� ����������
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum,
           100,
           crz->vbFilter.unweldParam,
           crz->vbFilter.tfiParam1,
           crz->vbFilter.tfiParam2
         );
         break;

       case 11: // ������ ��
         flt_Cut_big_small(
           crz->dat0PageBuff, 
           crz->datPageBuff, 
           crz->datPageOrnt, 
           crz->pageDat, 
           crz->sNum, 
           crz->vbFilter.rolledParam1, 
           crz->vbFilter.rolledParam2,
           crz->vbFilter.rolledAmplifer
         );
         break;
     } // switch (crz->vbFilter.active) {
 } // else  // if (crz->sType == SENS_TYPE_PROFIL)
} // void switch_filters(T_TRACE *trc, long crzIndx)


/*
������� ������� ������ ��� ������ �������� ��������� � ������� start
*/
short readDataPage(T_TRACE *trc, long crzIndx, long start) {
T_CRZSENS *crz;

 crz = &(trc->crz[crzIndx]);

 if (readData( trc, crzIndx, start / crz->step, crz->pageDat, crz->dat0PageBuff, crz->datPageBuff,
               crz->datPageOrnt, crz->bendingPlanePageBuff, crz->odomSensPageBuff)) return 1;

/*
 {
     char tmp_info[10240];
     long i;
     char num_txt[128];

     sprintf (tmp_info, "adress crz->datPageBuff = %ld\n adress crz->datPageOrnt = %ld\n xSize = %ld\n ySize = %ld\ncrz->vbFilter.active=%ld\n",
            &crz->datPageBuff[0], &crz->datPageOrnt[0], crz->pageDat, crz->sNum, crz->vbFilter.active);

//     sprintf (tmp_info, "crz->datPageOrnt = %ld\n", crz->datPageOrnt[0]);
     for ( i = 0; i < crz->pageDat; i ++)
     {
         sprintf(num_txt, ", %ld", crz->datPageOrnt[i]);
         strcat(tmp_info, num_txt);
     }
     MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"1", MB_OK | MB_ICONERROR);
 }
*/
 switch_filters(trc, crzIndx);

 //degree2sens(crz->datPageOrnt, crz->pageDat, crz->sNum);

 // use orientation for bufers 
 use_orientation_for_bufers (
    crz->dat0PageBuff, 
    crz->datPageBuff, 
    crz->datPageOrnt,
    crz->pageDat,
    crz->sNum,
    trc,
    crz->bendingPlanePageBuff,
    crz->odomSensPageBuff
 );

 fillBitmapPage(crz);
 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 return 0;

} // short readDataPage(T_TRACE *trc, long crzIndx, long start) {

/*
������� ������������ �������� value �� ��������� � ������� �������� �������� �������� scale
�� �������� �����.
*/
long krtScaleX (long data, long scale) {

 if (scale > 1) {
  return  data / scale; 
 } else if (scale < -1) {
  return data * -scale;
 } else {
  return data;
 }
}

long krtScaleY (long value, T_CRZSENS *crz) {
long sizeY;

 sizeY = crz->sNum - crz->hide;
 if (sizeY > crz->pixelY) {
  return value / (sizeY / crz->pixelY);
 } else {
  return value * (crz->pixelY / sizeY);
 }
}

/*
������� ������������ �������� value �� �������� � ��������� �������� �������� �������� scale
�� �������� �����.
*/
long krtScaleXback (long pixel, long scale) {
 if (scale > 1) {
  return  pixel * scale;
 } else if (scale < -1) {
  return pixel / -scale;
 } else {
  return pixel;
 }
}

long krtScaleYback (long value, T_CRZSENS *crz) {
long sizeY;

 sizeY = crz->sNum - crz->hide;
 if (sizeY > crz->pixelY) {
  return value * (sizeY / crz->pixelY);
 } else {
  return value / (crz->pixelY / sizeY);
 }
}

/*
������� ������� ������ ��� ������ ���������� � ������� start
� ������������� ������ � ������ ������ ��������
���� sizeX ������ ����������� ������
side = -1 ����� �����
side =  1 ����� ������
*/
short readDataShift(T_TRACE *trc, long crzIndx, long start, long sizeX) {
long dStart, dLength, scrollPixels, pageX, pageY, i, j, x1, x2, x3;
KRTDATA *buf, *sbuf;
T_CRZSENS *crz;

 crz = &(trc->crz[crzIndx]);

 dStart  = start / crz->step;
 dLength = abs(sizeX / crz->step);

 if (readData( trc, crzIndx, dStart, dLength, crz->dat0ScrlBuff, crz->datScrlBuff,
               crz->datScrlOrnt, crz->bendingPlaneScrlBuff, crz->odomSensScrlBuff)) return 1;

 //degree2sens(crz->datScrlOrnt, dLength, crz->sNum);

 // use orientation for bufers 
 use_orientation_for_bufers (
       crz->dat0ScrlBuff,
       crz->datScrlBuff,
       crz->datScrlOrnt,
       dLength,
       crz->sNum,
       trc,
       crz->bendingPlaneScrlBuff,
       crz->odomSensScrlBuff
 );


 pageX   = crz->pageDat;
 pageY   = crz->sNum;
 buf     = crz->dat0PageBuff;
 sbuf    = crz->dat0ScrlBuff;

 if (sizeX>0) {
  x1 = 0;        x2 = dLength;  x3 = pageX - dLength;
 } else {
  x1 = dLength;  x2 = 0;        x3 = 0;
 }

 // �������� ����� ������ �������� � ����������� � ���� ������ �� ������ ������ ������
 for (i=0;i<pageY;i++) {
  j=i*pageX;
  memmove(buf+j+x1, buf+j+x2, (pageX - dLength) * sizeof(KRTDATA));
  memcpy (buf+j+x3, sbuf+i*dLength, dLength * sizeof(KRTDATA));

  memmove(crz->datPageBuff+j+x1, crz->datPageBuff+j+x2, (pageX - dLength) * sizeof(KRTROW));
  memcpy (crz->datPageBuff+j+x3, crz->datScrlBuff+i*dLength, dLength * sizeof(KRTROW));
 }
 // �������� ����� ���������� �������� � ����������� � ���� ������ �� ������ ���������� ������
  memmove(crz->datPageOrnt+x1, crz->datPageOrnt+x2, (pageX - dLength) * sizeof(long));
  memcpy (crz->datPageOrnt+x3, crz->datScrlOrnt, dLength * sizeof(long));

 // �������� ����� ������� ��������
 x1 = krtScaleX(x1, trc->scaleX) * bytesPerPixel;
 x2 = krtScaleX(x2, trc->scaleX) * bytesPerPixel;
 scrollPixels = crz->pixelX - krtScaleX(dLength, trc->scaleX);
 for (i=0;i<crz->pixelY;i++) {
  j=i*crz->pixelX*bytesPerPixel;
  memmove(((BYTE *) crz->bmpBuff)+j+x1, ((BYTE *) crz->bmpBuff)+j+x2, scrollPixels * bytesPerPixel);
 }
 // �������� � ����� ������� �������� ������ �� ������ ������ ������
 krotStretchBlt (
  crz, 
  crz->dat0ScrlBuff,
  crz->vbScreen.orntOff ? NULL : crz->datScrlOrnt,
  dLength,
  crz->sNum - crz->hide,
  crz->bmpBuff,
  crz->bmpBuffScroll,
  crz->pic,
  crz->bmp,
  crz->pixelX,
  crz->pixelY,
  (sizeX>0 ? scrollPixels : 0),
  (sizeX>0 ? crz->pixelX : crz->pixelX - scrollPixels),
  crz->topSens,
  &(crz->pal), 0 );

 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);

 return 0;
} // short readDataShift(T_TRACE *trc, long crzIndx, long start, long sizeX) {

/**************************************************************************
������� ������ ����������� ��������� crzIndx ������ Handle � ������� start (��)
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotPaint (
 KRTHANDLE Handle,
 long crzIndx,
 HWND hWnd,
 long start,
 long forceReadData,
 long drawMode
) {

T_TRACE *trc;
T_CRZSENS *crz;
long delta, dStart;
RECT r;
long pixelX, pixelY;

static long krotPaint_call_counter = 0;

krotPaint_call_counter++;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);
 
 // ������������ ����� ���� � ����� ������
 crz->pic = hWnd;
 crz->drawMode = drawMode;

  // ���������� ������� ���� ����������� � ��������
 GetClientRect(hWnd, &r);
 pixelX = r.right - r.left;
 pixelY = r.bottom - r.top;

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "krotPaint cIndx: %ld, %ld -> %ldx%ld , start %ld", crzIndx, crz->pageDat, pixelX, pixelY, start
 );
 Log();
#endif

 if ((pixelX != crz->pixelX) || (pixelY != crz->pixelY)) { 
  sprintf(lastError, "Saved %ld:%ld not equal current %ld:%ld", crz->pixelX, crz->pixelY, pixelX, pixelY);
  return KRT_ERR;
 }

 delta = (start - crz->oldPos);

   forceReadData = 1;
 if (forceReadData || (abs(delta) >= (crz->pageDat * crz->step))) {
  // ��������� ����������� ����������� �������� ���
  // ������ � ������������ ����� -> ��������� �������������� ��������
  if (readDataPage(trc, crzIndx, start)) return KRT_ERR;

 } else if ((abs(delta) > 0) && (abs(delta) < (crz->pageDat * crz->step)))  {
  // ����� � �������� �������� -> ���������� �������� ����������
  // ������ ����������� ������
  dStart = (delta<0) ? start : (crz->oldPos + (crz->pageDat * crz->step));
  if (readDataShift(trc, crzIndx, dStart, delta)) return KRT_ERR;

 } else if (crz->needRedraw)  {
  fillBitmapPage(crz);
  SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 }
 else if (OldScaleMode != ScaleMode) {
  OldScaleMode = ScaleMode;
  fillBitmapPage(crz);
  SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 }

 showBmp(crz, hWnd);
 crz->needRedraw = 0;
 crz->oldPos = start;
 return KRT_OK;
} // short EXPORT KRTAPI krotPaint (

/*
������� ��������� ������ � ������������� ����������� ������� sens ������ bufDat � bufRow
�� ������� �������� ������. ������� ������� ������ ��������������� ������� ��������.
*/
short EXPORT KRTAPI krotGetSingleSens (
 KRTHANDLE Handle,     // ���������� �������
 long      crzIndx, 
 long      sens,       // ����� �������
 KRTDATA  *bufDat,     // ��������� �� ����� ������������ ������
 KRTROW   *bufRow      // ��������� �� ����� ����� ������
) {
T_TRACE *trc;
T_CRZSENS *crz;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 memcpy(bufRow, crz->datPageBuff + (crz->pageDat * sens), crz->pageDat * sizeof(KRTROW));
 memcpy(bufDat, crz->dat0PageBuff + (crz->pageDat * sens), crz->pageDat * sizeof(KRTDATA));
 return KRT_OK;
}

/*
������� ����������� ��������� ������������ �������
*/
short EXPORT KRTAPI krotGetVectSens (
  KRTHANDLE Handle,      // ���������� �������
  long      crzIndx,
  long      pos,         // ������� � ��.
  KRTDATA  *buf,         // ��������� �� ����� ������������ ������
  KRTROW   *row          // ��������� �� ����� ����� ������
) {
T_TRACE *trc;
T_CRZSENS *crz;
long dStart, sz, i;

long tmp_orient;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 // ���� ������������� ������ ������ �������� ������ ������ ���������
 if ((pos >= crz->oldPos) && (pos <= (crz->oldPos + (crz->pageDat * crz->step)))) {
  // ��������� ������ �� ������ ������, �� ������ �������.
  dStart = (pos - crz->oldPos) / crz->step; // �������� �������������� ������� � ������ ������
  sz = crz->pageDat;
  for (i=0; i < crz->sNum; i++) {
   buf[i]    = crz->dat0PageBuff[sz*i+dStart];
   row[i]    = crz->datPageBuff[sz*i+dStart];
  }
 } else {
  // ����� ��������� ������ � ��������
  dStart  = pos / crz->step;

  if (readData(trc, crzIndx, dStart, 1, buf, row, &tmp_orient, NULL, NULL)) {return KRT_ERR;}

  // use orientation for bufers 
  use_orientation_for_bufers (
        buf,
        row,
        &tmp_orient,
        1,
        crz->sNum,
        trc,
        NULL,
        NULL
  );

 }

 return KRT_OK;
} // short EXPORT KRTAPI krotGetVectSens (

/* *************************************************************************
������� ��������� ����� datBuff ���������� ��������� �������� ����������� ���������
����� crzIndx �� ������ Handle. �������� ��������� ������ ���������� � ������� xStart (��)
� ����� ����� xLength (��).
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotCorozData (
 KRTHANDLE Handle, 
 long crzIndx, 
 long xStart,     // ��
 long xLength,    // ��
 KRTDATA *datBuff,
 KRTROW *rowBuff
) {
T_TRACE *trc;
T_CRZSENS *crz;
long i, j, y, ornt;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 // ���������� ������ ��������, ������ ����� �������������� ���������
 if ( makeDatBuff(crz, xLength / crz->step, "krotCorozData") ) return KRT_ERR;
 // ������� ������ � ����� ��������

 if (readData( trc, crzIndx, xStart / crz->step, crz->pageDat, crz->dat0PageBuff, crz->datPageBuff,
               crz->datPageOrnt, crz->bendingPlanePageBuff, crz->odomSensPageBuff)) return 1;

// ��������� ������ ���������� �� ����� ����� ������� � ������ ��������
// degree2sens(crz->datPageOrnt, crz->pageDat, crz->sNum);

  // use orientation for bufers 
  use_orientation_for_bufers (
        crz->dat0PageBuff,
        crz->datPageBuff,
        crz->datPageOrnt,
        crz->pageDat,
        crz->sNum,
        trc,
        crz->bendingPlanePageBuff,
        crz->odomSensPageBuff
  );


 // �������������� �������� �� ������� ����� � ������ ����������
 for (i=0; i<crz->pageDat; i++) {
  ornt = crz->datPageOrnt[i];
  for (j=0; j<crz->sNum; j++) {
   y = ornt + j;
   y = (y < crz->sNum) ? y : crz->sNum - y;
   datBuff[crz->pageDat * j + i] = crz->dat0PageBuff[crz->pageDat * y + i];
   rowBuff[crz->pageDat * j + i] = crz->datPageBuff[crz->pageDat * y + i];
  }
 }
 return KRT_OK;
} // short EXPORT KRTAPI krotCorozData (

/*
������� ��������� ������������ ������� ������ �������� ������ ������� xSize � ���
������������� ����������� ����� �������� ������.
*/
short makeDatBuff(T_CRZSENS *crz, long xSize, char *callStack) {

////////////// debug
#ifndef LOG_1
 (void) callStack;
#endif

#define orient_vect_to_RowBuf 1  // ��� ��������� ��� #KRT_APIVER_3
                                 // ������ �������� ������ � ������ ������� �� ������ ����� API
                                 // ���������� �� �������� ��� ����� ����������.
                                 // ������ � ������� �������� � ��������� �������������� � ������
                                 // ����� ������, ��� ��� ���� ������.

#define bending_plane_to_RowBuf 1  // ��� ��������� ��� #KRT_APIVER_3
                                   // � ������ � ������ ������� ����������� ������ � ��������� ������ 
                                   // (bending plane), ��� ��� ���� ������.
                                   // ������ � ���������� ��������� ������ ����� ������������ ��� ������ ��������
                                   // typedef struct {   
                                   //     short sens;     // ������ ����� ������� �������� ��������� ������ �� ������ ���������
                                   //     short color;    // ���� ��������� (����� >500 D = ���, ����� <500 D = 120, ����� <250 D = 200) 
                                   // } T_DimensionPixel;

#define odometer_sens_to_RowBuf 1  // ��� ��������� ��� #KRT_APIVER_3
                                   // � ������ � ������ ������� ����������� ������ �� ���������� ��������
                                   // ������ �� ���������� �������� ����� ������������ ��� ������ ��������
                                   // typedef struct {   
                                   //     short sens;     // ������ �� ������ �������� ���� �������� �� ������ ���������
                                   //     short color;    // ���� ��������� ( 0 - ���, 115 - ������ ��., 152 - ������ ��.) 
                                   // } T_DimensionPixel;

 // ���� ����������� ������ �������� ��������� ������������ ������ �� X
 // ������� �������, �� ���������� ��������� ������
 if (crz->maxDatPage < xSize) {
  if (!(
   allocMem(xSize * ( crz->sNum 
                      + orient_vect_to_RowBuf
                      + bending_plane_to_RowBuf
                      + odometer_sens_to_RowBuf
                    ) * sizeof(KRTROW), (void**) &(crz->datScrlBuff),  "����� ����� ������ ����������") &
//   allocMem(xSize * sizeof(long),                     (void**) &(crz->datScrlOrnt),  "����� ���������� ����������") &
   allocMem(xSize * ( crz->sNum
                      + orient_vect_to_RowBuf
                      + bending_plane_to_RowBuf
                      + odometer_sens_to_RowBuf
                    ) * sizeof(KRTROW), (void**) &(crz->datPageBuff),  "����� ����� ������ ��������") &
//   allocMem(xSize * sizeof(long),                     (void**) &(crz->datPageOrnt),  "����� ���������� ��������") &
   allocMem(xSize * (crz->sNum) * sizeof(KRTDATA), (void**) &(crz->dat0ScrlBuff), "����� ������������ ������ ����������") &
   allocMem(xSize * (crz->sNum) * sizeof(KRTDATA), (void**) &(crz->dat0PageBuff), "����� ������������ ������ ��������") 
   )) {
   return 1;
  }


  crz->datScrlOrnt = & (crz->datScrlBuff[xSize * crz->sNum]);
  crz->datPageOrnt = & (crz->datPageBuff[xSize * crz->sNum]);

  crz->bendingPlanePageBuff = (T_DimensionPixel *) &(crz->datScrlBuff[xSize * (crz->sNum + 1)]);
  crz->bendingPlaneScrlBuff = (T_DimensionPixel *) &(crz->datPageBuff[xSize * (crz->sNum + 1)]);

  crz->odomSensPageBuff = (T_DimensionPixel *) &(crz->datScrlBuff[xSize * (crz->sNum + 2)]);
  crz->odomSensScrlBuff = (T_DimensionPixel *) &(crz->datPageBuff[xSize * (crz->sNum + 2)]);

  { // ������� ������� ���������� 
      long i;
      for ( i = 0; i < xSize; i ++)
      {
          crz->datScrlOrnt[i] = 10;
          crz->datPageOrnt[i] = 10;
      }
  }

  // ����� ������������ ������ �������� ������ �� X ��� �������� ������.
  crz->maxDatPage = xSize;
 }

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "pageDat %ld -> %ld %s -> %s(%ld, %ld)", crz->pageDat, xSize, callStack, "makeDatBuff", crz->index, xSize
 );
 Log();
#endif

 crz->pageDat = xSize;
 return 0;
} // short makeDatBuff(T_CRZSENS *crz, long xSize, char *callStack) {

/*
������� ��������� ������������ ������� ������ ������� �������� �������� pixelX � pixelY � ���
������������� ����������� ����� ������� ��������.
*/
short makeBmpBuff(T_CRZSENS *crz, long pixelX, long pixelY) {
long arrSize;

 // ��� ������������� ���������������� ����� �������
 arrSize = pixelX * pixelY;
 if (crz->maxBmpSize < arrSize) {
  if (crz->bmpBuff) { free(crz->bmpBuff); }
  if (crz->bmpBuffScroll) { free(crz->bmpBuffScroll); }
  crz->maxBmpSize = 0L;
  crz->bmpBuff = (void *) malloc(arrSize * bytesPerPixel);
  crz->bmpBuffScroll = (void *) malloc(arrSize * bytesPerPixel);
  if ((crz->bmpBuff == NULL) || (crz->bmpBuff == NULL)) {
   sprintf (
    lastError, 
    "�� ���������� ������ ��� ������� ������� (%ld ����)", 
    arrSize * bytesPerPixel
           );
   return 1;
  }
  // ����� ������������ ������ ������ ������� � ��������
  crz->maxBmpSize = arrSize;
 }

 return 0;
}

/*
������� ����������� ������ ��������, ���� ��� ������� �� ������������ �������� pixelX � pixelY
*/
short makeBitmap(T_CRZSENS *crz, HWND hWnd, long pixelX, long pixelY) {
HDC hdcDest;

 // ��� ������������� �������������� ������ ��������
 if ((crz->pixelX != pixelX) || (crz->pixelY != pixelY)) {

  if (crz->bmp) { DeleteObject(crz->bmp); } 
  // ���������� �������� � ������ ���������� ����� ������ ���������
  // ������ ������, ��� �������������� vb-���� ����������.
  // ������� �� ���� �������������� ������������ ���������� � ����������
  // ��������� ������� CreateBitmap :
  // "Each scan line in the rectangle must be word aligned 
  // (scan lines that are not word aligned must be padded with zeros)."
  hdcDest = GetDC(hWnd);
  crz->bmp = CreateCompatibleBitmap(hdcDest, pixelX, pixelY);
  ReleaseDC(hWnd, hdcDest);

  if (crz->bmp == NULL) {
   sprintf(lastError, "������ ��� �������� ������� ������");
   return 1;
  }
 }

 return 0;
}

/* *************************************************************************
������� ���������� ��� ������������ ���������� ���� ��������� ��������� ����� crzIndx.
��������� ����� ������ ��� ��������� ����� ���������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotTopSens (
 KRTHANDLE Handle,
 long crzIndx,
 long topSens) {

T_TRACE *trc;
T_CRZSENS *crz;
long lineLen, shift, i, lineIndex;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 if (topSens == crz->topSens) return KRT_OK;

 if (crz->pixelY == 0) {
  crz->topSens = topSens;
  return KRT_OK;
 }
 
 shift = krtScaleY(topSens, crz) - krtScaleY(crz->topSens, crz);
 if (shift == 0) return KRT_OK;
 if (shift<0) { shift += crz->pixelY; }

 lineLen = crz->pixelX * bytesPerPixel;

 for (i = 0; i < crz->pixelY; i ++) {
  lineIndex = shift + i;
  lineIndex = lineIndex < crz->pixelY ? lineIndex : lineIndex - crz->pixelY ;
  memcpy((BYTE *) crz->bmpBuffScroll + (i * lineLen), (BYTE *) crz->bmpBuff + (lineIndex * lineLen), lineLen);
 }

 memcpy((BYTE *) crz->bmpBuff, (BYTE *) crz->bmpBuffScroll, lineLen* crz->pixelY);
 crz->topSens = topSens;
 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 return KRT_OK;
} // short EXPORT KRTAPI krotTopSens (

/*
������� ������������ ���������� ������� �������� ��������� ����� crz, 
���������� ������ �������� ������, ���������� ����������� ������, 
���������� ������ ������ ������� � �������� � ���� ����������� ������
*/
short increasePage(T_TRACE *trc, T_CRZSENS *crz, long delta, char *callStack) {
long oldSizeDat, oldSizeBmp, i, j1, j2, j3, lineIndex;
long oldPage, newPage, lineLenDat, lineLenRow, lineAddDat, lineAddRow;
long newPixelX, dltPixelX;

char calltrace[10000];

 sprintf(calltrace, "%s -> %s", callStack, "increasePage");


 oldSizeDat = crz->maxDatPage;
 if (makeDatBuff(crz, crz->pageDat + delta, calltrace)) return 1;
 oldSizeBmp = crz->maxBmpSize;
 dltPixelX = krtScaleX(delta, trc->scaleX);
 newPixelX = crz->pixelX + dltPixelX;
 if (makeBmpBuff(crz, newPixelX, crz->pixelY)) return 1;

 if (oldSizeDat != crz->maxDatPage) {

  return readDataPage(trc, crz->index, crz->oldPos);

 } else {

  oldPage = crz->pageDat - delta;
  newPage = crz->pageDat;
  lineLenDat = oldPage * sizeof(KRTDATA);
  lineLenRow = oldPage * sizeof(KRTROW); 
  lineAddDat = delta * sizeof(KRTDATA);
  lineAddRow = delta * sizeof(KRTROW); 

  //��������� ����������� ������
  if (readData( trc, crz->index, crz->oldPos / crz->step + crz->pageDat - delta, delta, crz->dat0ScrlBuff, crz->datScrlBuff,
                crz->datScrlOrnt, crz->bendingPlaneScrlBuff, crz->odomSensScrlBuff)) return 1;

  //degree2sens(crz->datScrlOrnt, delta, crz->sNum);

  // use orientation for bufers 
  use_orientation_for_bufers (
        crz->dat0ScrlBuff,
        crz->datScrlBuff,
        crz->datScrlOrnt,
        delta,
        crz->sNum,
        trc,
        crz->bendingPlaneScrlBuff,
        crz->odomSensScrlBuff
  );

  // �������������� �������� ������ 
  for (i=1; i<crz->sNum; i++) {

   // �������� � �����, ����� �� �������� ��������� ������
   lineIndex = crz->sNum - i;
   j1= lineIndex * oldPage;
   j2= lineIndex * newPage;
   j3= lineIndex * delta;

   memmove(crz->dat0PageBuff+j2, crz->dat0PageBuff+j1, lineLenDat);
   memcpy(crz->dat0PageBuff+j2+oldPage, crz->dat0ScrlBuff+j3, lineAddDat);

   memmove(crz->datPageBuff+j2,  crz->datPageBuff+j1,  lineLenRow);
   memcpy(crz->datPageBuff+j2+oldPage, crz->datScrlBuff+j3, lineAddRow);
  }

 // �������������� ����� ���������� �������� � ����������� � ���� ������ �� ������ ���������� ������
//
//     ���-�� ���� ������ ������� !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//  memmove(crz->datPageOrnt, crz->datPageOrnt, lineLenRow);
  memcpy (crz->datPageOrnt+oldPage, crz->datScrlOrnt, lineAddRow);

  // ����������� ������ ������ ������������ ������
  memcpy(crz->dat0PageBuff+oldPage, crz->dat0ScrlBuff, lineAddDat);
  memcpy(crz->datPageBuff+oldPage, crz->datScrlBuff, lineAddRow);

 }

 if (oldSizeBmp != crz->maxBmpSize) {

  fillBitmapPage(crz);

 } else {

  // �������������� ����� ������� 
  oldPage = crz->pixelX * bytesPerPixel;
  newPage = newPixelX * bytesPerPixel;

  for (i=1; i < crz->pixelY; i++) {
   // �������� � �����, ����� �� �������� ��������� ������
   lineIndex = crz->pixelY - i;
   j1= lineIndex * oldPage;
   j2= lineIndex * newPage;
   memmove(((BYTE *) crz->bmpBuff)+j2, ((BYTE *) crz->bmpBuff)+j1, oldPage);
  }

  // �������� ���� ����� ��������� ������
  krotStretchBlt (
   crz, 
   crz->dat0ScrlBuff,
   crz->vbScreen.orntOff ? NULL : crz->datScrlOrnt,
   delta,
   crz->sNum - crz->hide,
   crz->bmpBuff,
   crz->bmpBuffScroll,
   crz->pic,
   crz->bmp,
   newPixelX,
   crz->pixelY,
   crz->pixelX,
   newPixelX,
   crz->topSens,
   &(crz->pal), 0 );
 }

 return 0;
} // short increasePage(T_TRACE *trc, T_CRZSENS *crz, long delta, char *callStack) {

/*
������� ������������ ���������� ������� �������� ��������� ����� crz, 
�������� ������� �������� ������ � ������ �������. 
������������ �������� ������ � ����� �������.
*/
short decreasePage(T_TRACE *trc, T_CRZSENS *crz, long delta) {
long i, j1, j2, pageY, lineLenDat, lineLenRow, oldPage, newPage;
long newPixelX, dltPixelX;

 // ������ ������
 pageY = crz->sNum;
 oldPage = crz->pageDat;
 newPage = crz->pageDat - delta;
 lineLenDat = oldPage * sizeof(KRTDATA);
 lineLenRow = oldPage * sizeof(KRTROW); 

 for (i=1; i<pageY; i++) {
  j1= i * oldPage;
  j2= i * newPage;
  memmove(crz->dat0PageBuff+j2, crz->dat0PageBuff+j1, lineLenDat);
  memmove(crz->datPageBuff+j2,  crz->datPageBuff+j1,  lineLenRow);
 }

 // ����� �������
 pageY = crz->pixelY;
 dltPixelX = krtScaleX(delta, trc->scaleX);
 newPixelX = crz->pixelX - dltPixelX;

 oldPage = crz->pixelX * bytesPerPixel;
 newPage = newPixelX * bytesPerPixel;

 for (i=1; i<pageY; i++) {
  j1= i * oldPage;
  j2= i * newPage;
  memmove((BYTE *) crz->bmpBuff+j2, (BYTE *) crz->bmpBuff+j1, newPage);
 }

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "pageDat %ld -> %ld %s(%ld, %ld)", crz->pageDat, crz->pageDat - delta, "decreasePage", crz->index, delta
 );
 Log();
#endif

 crz->pageDat = crz->pageDat - delta;
 return 0;
} // short decreasePage(

/*
������� ������������ �������� ������ ����� crz � ������������ � ����� ��������� �� x scale
��� ������������� ����������� ����������� ������.
������ ��������� ����� ������� ��������.
*/
short changePage(T_TRACE *trc, T_CRZSENS *crz, long scale) {
long newPageDat, oldSizeDat, oldMaxDat, delta, i, j1, j2, j3, lineLenDat, lineLenRow, lineAddDat, lineAddRow, lineIndex;

 trc->scaleX = scale;
 newPageDat = krtScaleXback(crz->pixelX, scale);
 if (newPageDat == crz->pageDat) return 0;

 if (newPageDat < crz->pageDat) {

  // �������������� ����� ������ � ������ ��� ����������

  lineLenDat = newPageDat * sizeof(KRTDATA);
  lineLenRow = newPageDat * sizeof(KRTROW); 

  for (i=1; i<crz->sNum; i++) {
   j1= i * crz->pageDat;
   j2= i * newPageDat;
   memmove(crz->dat0PageBuff+j2, crz->dat0PageBuff+j1, lineLenDat);
   memmove(crz->datPageBuff+j2,  crz->datPageBuff+j1,  lineLenRow);
  }

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "pageDat %ld -> %ld %s(%ld, %ld)", crz->pageDat, newPageDat, "changePage", crz->index, scale
 );
 Log();
#endif

  crz->pageDat = newPageDat;

 } else {

  oldSizeDat = crz->pageDat;
  oldMaxDat = crz->maxDatPage;
  if (makeDatBuff(crz, newPageDat, "changePage")) return 1;

  if (oldMaxDat != crz->maxDatPage) {

   return readDataPage(trc, crz->index, crz->oldPos);

  } else {

   // �������������� ����� ������ � ������ ��� ����������
   delta = crz->pageDat - oldSizeDat;
   lineLenDat = oldSizeDat * sizeof(KRTDATA);
   lineLenRow = oldSizeDat * sizeof(KRTROW); 
   lineAddDat = delta * sizeof(KRTDATA);
   lineAddRow = delta * sizeof(KRTROW); 

   //��������� ����������� ������
   if (readData( trc, crz->index, crz->oldPos / crz->step + oldSizeDat, crz->pageDat - oldSizeDat, crz->dat0ScrlBuff, crz->datScrlBuff,
                 crz->datPageOrnt, crz->bendingPlaneScrlBuff, crz->odomSensScrlBuff)) return 1;

   //degree2sens(crz->datScrlOrnt, delta, crz->sNum);

   // use orientation for bufers 
   use_orientation_for_bufers (
       crz->dat0ScrlBuff,
       crz->datScrlBuff,
       crz->datScrlOrnt,
       delta,
       crz->sNum,
       trc,
       crz->bendingPlaneScrlBuff,
       crz->odomSensScrlBuff
   );


   // �������������� �������� ������ 
   for (i=1; i<crz->sNum; i++) {
    // �������� � �����, ����� �� �������� ��������� ������
    lineIndex = crz->sNum - i;
    j1= lineIndex * oldSizeDat;
    j2= lineIndex * crz->pageDat;
    j3= lineIndex * delta;

    memmove(crz->dat0PageBuff+j2, crz->dat0PageBuff+j1, lineLenDat);
    memcpy(crz->dat0PageBuff+j2+oldSizeDat, crz->dat0ScrlBuff+j3, lineAddDat);

    memmove(crz->datPageBuff+j2,  crz->datPageBuff+j1,  lineLenRow);
    memcpy(crz->datPageBuff+j2+oldSizeDat, crz->datScrlBuff+j3, lineAddRow);
   }
   // ����������� ������ ������ ������������ ������
   memcpy(crz->dat0PageBuff+oldSizeDat, crz->dat0ScrlBuff, lineAddDat);
   memcpy(crz->datPageBuff+oldSizeDat, crz->datScrlBuff, lineAddRow);

// ����������� ������ ����������
//  memmove(crz->datPageOrnt, crz->datPageOrnt, lineLenRow);
  memcpy (crz->datPageOrnt+oldSizeDat, crz->datScrlOrnt, lineAddRow);

  }
 }

 // ��������������� ������ ��������
 fillBitmapPage(crz);
 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 return 0;
} // short changePage(T_TRACE *trc, T_CRZSENS *crz, long scale)

/* *************************************************************************
������� ���������� ��� ��������� ��������������� �������� ���������.
��������� ���������� �������� ������ � ������� ��� ���� ������ ���������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotPageScale (
 KRTHANDLE Handle,
 long scale) {

T_TRACE *trc;
long i;

////////////// debug
#ifdef LOG_1
long dbgOldScale, dbgOldPage;
#endif

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;

 
////////////// debug
#ifdef LOG_1
 dbgOldPage = trc->crz[0].pageDat;
 dbgOldScale = trc->scaleX;
#endif

 for (i=0; i < trc->record.sensGroups; i++) {
  if (changePage(trc, &(trc->crz[i]), scale)) return KRT_ERR;
 }

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "krotPageScale %ld -> %ld page: (%ld -> %ld)", dbgOldScale, trc->scaleX, dbgOldPage, trc->crz[0].pageDat
 );
 Log();
#endif

 return KRT_OK;
}

/* *************************************************************************
������� ���������� ��� ��������� �������� ���� ��������� ��������� ����� crzIndx.
��������� ���������� �������� ������ � ������ ��� ��������� ����� ���������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotChangePic (
 KRTHANDLE Handle,
 long crzIndx,
 HWND hWnd) {

 T_TRACE *trc;
 T_CRZSENS *crz;
 RECT r;
 long pixelX, pixelY, shift; //, ret;

////////////// debug
#ifdef LOG_1
 long dbgOldPixelX, dbgOldPixelY, dbgOldPage;
#endif


 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

////////////// debug
#ifdef LOG_1
 dbgOldPixelX = crz->pixelX;
 dbgOldPixelY = crz->pixelY;
 dbgOldPage = crz->pageDat;
#endif

 // ���������� ������� ���� ����������� � ��������
 GetClientRect(hWnd, &r);
 pixelX = r.right - r.left;
 pixelY = r.bottom - r.top;

 if (pixelX == crz->pixelX) {
     if (pixelY == crz->pixelY) { 
         return KRT_OK;
     } else {
         if (makeBmpBuff(crz, pixelX, pixelY) || makeBitmap(crz, hWnd, pixelX, pixelY)) return KRT_ERR;
         crz->pixelX = pixelX;
         crz->pixelY = pixelY;
         fillBitmapPage(crz);
     }
 } else {

//  if (crz->pixelX > 0) {
//
//   shift = krtScaleXback(pixelX, trc->scaleX) - krtScaleXback(crz->pixelX, trc->scaleX);
//   if (makeBitmap(crz, hWnd, pixelX, pixelY)) return KRT_ERR;
//   ret = (shift<0) ? decreasePage(trc, crz, -shift) : increasePage(trc, crz, shift, "krotChangePic");
//   if (ret) return KRT_ERR;
//   crz->pixelX = pixelX;
//   crz->pixelY = pixelY;
//  } else {
   shift = krtScaleXback(pixelX, trc->scaleX);
   if (makeDatBuff(crz, shift, "krotChangePic")) return KRT_ERR;
   if (makeBmpBuff(crz, pixelX, pixelY)) return KRT_ERR;
   if (makeBitmap(crz, hWnd, pixelX, pixelY)) return KRT_ERR;
   crz->pixelY = pixelY;
   crz->pixelX = pixelX;
   if (readDataPage(trc, crz->index, crz->oldPos)) return KRT_ERR;
   return KRT_OK;
//  }
 }

////////////// debug
#ifdef LOG_1
 sprintf(
  LogString, 
  "krotChangePic crzIndx %ld bmp %ldx%ld -> %ldx%ld page: %ld -> %ld", 
   crzIndx, 
   dbgOldPixelX,
   dbgOldPixelY,
   crz->pixelX,
   crz->pixelY,
   dbgOldPage, 
   crz->pageDat
 );
 Log();
#endif

 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 return KRT_OK;
} // short EXPORT KRTAPI krotChangePic (

/* *************************************************************************
������� ���������� ��� ��������� ���������� �������� ��������� ���������.
������ ��������� �� ���������� ��������� �������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotScreenFilter (
 KRTHANDLE Handle, 
 long crzIndx, 
 VB_FILTER_INFO *vbFilter
) {
T_TRACE *trc;
T_CRZSENS *crz;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 // ��������� ���������� �� VB ���������
 crz->vbFilter = *vbFilter;
 // ��������� ���� ����������� ����������� �����������
 crz->needRedraw = 1;

 return KRT_OK;
}


/*
������� ���������� ��� ������ krotScreenMode (��������� ���� ������� ����� ������������)
������ switch_filters(T_TRACE *trc, long crzIndx), �� ��� ������������ ������ ������,
� �� ��� �������� ���������
*/
void base_switch_filters(T_TRACE *trc, long crzIndx,
                         long xSize, KRTDATA* data_tmp, KRTROW* rowData_tmp, long* tmp_orient)
{
 T_CRZSENS *crz = &(trc->crz[crzIndx]);

/*
 {
     char tmp_info[10240];

     sprintf (tmp_info, "crz->vbFilter.active = %ld\n trc->drv.apiVer = %ld\n",
              crz->vbFilter.active, trc->drv.apiVer);
     MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"1", MB_OK | MB_ICONERROR);
 }
*/

 if ( crz->sType == SENS_TYPE_PROFIL && crzIndx == 0 )
 {
      long sens_counter;
      long length_counter;
      long screen_pos;

      static long profil_crz_call_counter = 0;

      screen_pos = 0;
      for (sens_counter = 0; sens_counter < crz->sNum; sens_counter++) {
          for (length_counter = 0; length_counter < xSize; length_counter++) {
              screen_pos = xSize * sens_counter + length_counter;
              data_tmp[screen_pos] = (KRTDATA)Calculate_profil_mm(rowData_tmp[screen_pos], sens_counter, crz);
          } //for (length_counter = 0; length_counter < xSize; length_counter++) {
      } // for (sens_counter = 0, sens_counter < crz->profil_sens_quantity; sens_counter++) {

      profil_crz_call_counter++;
 } else { // if (crz->sType == SENS_TYPE_PROFIL)
/*
      {  // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
          char tmp_info[10240];

          sprintf (tmp_info, "crz->vbFilter.active = %ld\n", crz->vbFilter.active);
          MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"2", MB_OK | MB_ICONERROR);
      } // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
*/

     switch (crz->vbFilter.active) {

       case 0: // ��� �������

           if (trc->drv.apiVer != KRT_APIVER_3 ) break;

           if (crz->Amplification_dflt <= 0) crz->Amplification_dflt = 10;

           fltExponent (
               data_tmp, // ��������� �� ������ ������������ ������ �������� ���������
               rowData_tmp,  // ��������� �� ����� ����� ������ ��� ��������
               tmp_orient,  // ��������� �� ������ ���������� ����������� ������
               xSize,      // ������ ������� �� X
               crz->sNum,         // ������ ������� �� Y (���������� ��������)

    //           crz->sensor,     // ��������� �� ������ �������� ��������. ��� ������� � ��������� ������� �����: crz->sensor[i].delta

               14, // ������� ����������� ������� (�������� �� 0 �� 100)
               crz->Amplification_dflt  // �������� �������� ������� (�������� �� 1 �� 100)
           );

           break;


       case 1: // cheshka
           fltExponent (
               data_tmp, // ��������� �� ������ ������������ ������ �������� ���������
               rowData_tmp,  // ��������� �� ����� ����� ������ ��� ��������
               tmp_orient,  // ��������� �� ������ ���������� ����������� ������
               xSize,      // ������ ������� �� X
               crz->sNum,         // ������ ������� �� Y (���������� ��������)

    //           crz->sensor,     // ��������� �� ������ �������� ��������. ��� ������� � ��������� ������� �����: crz->sensor[i].delta

               14, // ������� ����������� ������� (�������� �� 0 �� 100)
               10  // �������� �������� ������� (�������� �� 1 �� 100)
           );
    /*     fltUnweld(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum, 
           crz->vbFilter.unweldParam
         );
    */
         break;

       case 2: // tselnotyanutaya
         fltRolled(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum, 
           crz->vbFilter.rolledParam1, 
           crz->vbFilter.rolledParam2,
           crz->vbFilter.rolledAmplifer
         );
         break;

       case 3: // prigruz
         fltPrigruz(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum
         );
         break;

       case 4: // prodol treschiny
         fltPoligon(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum, 
           crz->vbFilter.tfiParam1, 
           crz->vbFilter.tfiParam2,
           crz->vbFilter.tfiBase
         );
         break;

       case 5: // no mathematic
         fltNo_math(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           crz->sensor
         );
         break;

       case 6: // ������ � ������ //2021 // volosok
    //     flt_k_tsentru_plus_poperek (
         flt_filament(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam,
           crz->vbFilter.tfiParam1,
           crz->vbFilter.tfiParam2
         );
         break;

       case 7: //������ �� ������ // ������ 2019
    //     flt_ot_tsentra_plus_poperek (
         flt_Filter2019(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 8: // ������� 1 // ���������
         flt_convolution_1(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 9: // ������� 2 // ��������
         flt_Skolz_plus_poperek( // ���������� ������� ����� ����� �������
    //     flt_Cut_big_small( // ������� �� ��������� ������� � ������������� ��������
    //     flt_convolution_2(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           crz->sensor,
           crz->vbFilter.unweldParam
         );
         break;

       case 10: // ��������� ��������
    //     flt_MHAT ( // MHAT ������� ���������� �� ������ ������
         fltMedianFullScreen ( // ��������� ����������
    //     flt_convolution_3( // ������� 3 �������� ����������
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum,
           100,
           crz->vbFilter.unweldParam,
           crz->vbFilter.tfiParam1,
           crz->vbFilter.tfiParam2
         );
         break;

       case 11: // ������ ��
         flt_Cut_big_small(
           data_tmp, 
           rowData_tmp, 
           tmp_orient, 
           xSize, 
           crz->sNum, 
           crz->vbFilter.rolledParam1, 
           crz->vbFilter.rolledParam2,
           crz->vbFilter.rolledAmplifer
         );
         break;
     } // switch (crz->vbFilter.active) {
 } // else  // if (crz->sType == SENS_TYPE_PROFIL)
} // void base_switch_filters(T_TRACE *trc, long crzIndx)



/* *************************************************************************
������� ���������� ��� ��������� ���������� ��������� ���������.
������ ��������� �� ���������� ��������� �������.
��� ������  ���������� KRT_OK, KRT_ERR ��� ������.
����������� �������� ������ �������� ����� ������� krotError.
*/
short EXPORT KRTAPI krotScreenMode (
 KRTHANDLE Handle, 
 long crzIndx, 
 long *sens,         
 VB_PAINT_INFO *vbScreen,
 VB_GRAPH_INFO *vbGraphs
) {

T_TRACE *trc;
T_PAL *pal;
T_CRZSENS *crz;
long i, back;

// ��������� ������ ��� ������� ��������� ��� ������ ������� �����
#define LENGTH_FOR_LOAD_DATA 512

KRTDATA* data_tmp;
KRTROW *rowData_tmp;
long tmp_orient[LENGTH_FOR_LOAD_DATA];
long xDataSize = LENGTH_FOR_LOAD_DATA;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 pal = &(crz->pal);

 // ���������� ����� ��������
 if (sens) {
  crz->hide = 0;
  for (i=0; i < crz->sNum; i++) { 
   crz->sensor[i].mode = sens[i]; 
   if (sens[i] == SMODE_DELETE) { crz->hide++; }
  }
 }

 // ��������� ������� �����
 if (vbScreen->baseLine < 0) {

////////////// debug
#ifdef LOG_BASE_LINE
 sprintf(
  LogString, 
  "Clear bl for %ld sensors", 
   crz->sNum
 );
 Log();
#endif

  for (i=0; i < crz->sNum; i++) {
    crz->sensor[i].delta = 0;
    crz->sensor[i].value = 0;
  }

 } else {

  if (vbScreen->baseLine != crz->vbScreen.baseLine) {

////////////// debug
#ifdef LOG_BASE_LINE
 sprintf(
  LogString, 
  "Set bl at dist %ld mm (%ld step)", 
   vbScreen->baseLine,
   vbScreen->baseLine / crz->step
 );
 Log();
#endif

/*
      {  // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
          char tmp_info[10240];

          sprintf(
           tmp_info, 
           "Set bl at dist %ld mm (%ld step)", 
            vbScreen->baseLine,
            vbScreen->baseLine / crz->step
          );
          MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"2", MB_OK | MB_ICONERROR);
      } // ------ DEBUG ------ DEBUG ------ DEBUG ------ DEBUG ------
*/

       data_tmp   = (KRTDATA *) malloc(xDataSize * crz->sNum * sizeof(KRTDATA));
       rowData_tmp = (KRTROW *) malloc(xDataSize * crz->sNum * sizeof(KRTROW));

       if (readData(trc, crzIndx, vbScreen->baseLine / crz->step, xDataSize, data_tmp, rowData_tmp, tmp_orient, NULL, NULL)) return KRT_ERR;

       base_switch_filters(trc, crzIndx, xDataSize, data_tmp, rowData_tmp, tmp_orient);


       back = KRT_PALLETE_SIZE / 2;
       for (i=0; i < crz->sNum; i++) { 
          crz->sensor[i].delta = back - data_tmp[i*xDataSize]; 
          crz->sensor[i].value = rowData_tmp[i*xDataSize]; 

////////////// debug
#ifdef LOG_BASE_LINE
 sprintf(
  LogString, 
  "[%ld] = %ld -> %ld", 
   i,
   crz->dat0ScrlBuff[i],
   crz->sensor[i].delta
 );
 Log();
#endif

      }

      free(data_tmp);
      free(rowData_tmp);
  }

 }
 
 // ��������� ���������� �� VB ���������
 crz->vbScreen  = *vbScreen;
 crz->vbGraphs = *vbGraphs;

 // ��������� ���� ����������� ����������� �����������
 crz->needRedraw = 1;

 return KRT_OK;
} // short EXPORT KRTAPI krotScreenMode (

/*
������� ������ ��� ������������ ����� ��� ��������� �������� �����������
i = 0 - ���������� ������������ ��������
i = 1 - ���������� ����������� ��������
i = 2 - ���������� ������� ��������
i = 3 - ���������� ������ ���������� ��������

*/
void EXPORT KRTAPI SetScaleMode (short i)
{
	ScaleMode = i;
}

/*
������� ���������� ��� ������������� ���������������
*/
short EXPORT KRTAPI GetScaleMode ()
{
	return ScaleMode;
}

#include "draw_my.c"
