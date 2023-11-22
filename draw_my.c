
char tmp_info[1024];

//char* bmp_file_buf; 

// 32 бит
BOOL  Save_mem_ArrFile(const __int32* arr, 
                         int width, int height, int bpp,  char * bmp_file_buf )
{
    DWORD p_row; 
    DWORD size;
    BITMAPFILEHEADER  hdr;
    BITMAPINFO dib;

    DWORD bmp_file_buf_pos = 0;
    DWORD bmp_file_buf_size;

    DWORD arr_pos = 0;

    bpp = 32;

    p_row = (DWORD)((width * bpp + 31) & ~31) / 8uL;
    size  = (DWORD)(height * p_row);

    bmp_file_buf_size = size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//    bmp_file_buf = malloc( bmp_file_buf_size );

//    if(bmp_file_buf == INVALID_HANDLE_VALUE)
//    {
//        return FALSE;
//    }

    // формируем файловый заголовок
    ZeroMemory(&hdr, sizeof(BITMAPFILEHEADER));
    hdr.bfType    = 0x4D42;
    hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    hdr.bfSize    = hdr.bfOffBits + size;

    // заголовок описателя растра
    ZeroMemory(&dib, sizeof(BITMAPINFO));
    dib.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    dib.bmiHeader.biBitCount  = (WORD) bpp;
    dib.bmiHeader.biCompression  = BI_RGB;
    dib.bmiHeader.biPlanes  = 1u;
    dib.bmiHeader.biWidth   = (long)width;
    dib.bmiHeader.biHeight  = (long)-height;
    dib.bmiHeader.biSizeImage   = size;
    dib.bmiHeader.biXPelsPerMeter = 11811L;
    dib.bmiHeader.biYPelsPerMeter = 11811L;
    dib.bmiHeader.biClrImportant  = 0uL;
    dib.bmiHeader.biClrUsed  = 0uL;


    // записываем заголовки...
    memcpy ( & (bmp_file_buf[bmp_file_buf_pos]), 
             &hdr, sizeof(BITMAPFILEHEADER)
           );
    bmp_file_buf_pos += sizeof(BITMAPFILEHEADER);

    memcpy ( & (bmp_file_buf[bmp_file_buf_pos]), 
             &dib.bmiHeader, sizeof(BITMAPINFOHEADER)
           );
    bmp_file_buf_pos += sizeof(BITMAPINFOHEADER);

    // запись массива пикселей
    while ( size > 32000)
    {
        memcpy ( & (bmp_file_buf[bmp_file_buf_pos]),
                 & (arr[arr_pos]), 32000
               );
        bmp_file_buf_pos += 32000;
        size -= 32000;
        arr_pos += 32000/4;

    }
    memcpy ( &(bmp_file_buf[bmp_file_buf_pos]),
             & (arr[arr_pos]), size
           );


//    sprintf (tmp_info, "size=%ld\n", size);
//    MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"Save_mem_ArrFile 4", MB_OK | MB_ICONERROR);

/*
    // далее запись в файл
    fp = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(fp == INVALID_HANDLE_VALUE)
           return FALSE;

    WriteFile(fp, (LPCVOID)bmp_file_buf, bmp_file_buf_size, &dwr, NULL);

   FlushFileBuffers(fp);
   CloseHandle(fp);
*/

//   free(bmp_file_buf);

   return TRUE;
} // BOOL  Save_mem_ArrFile(



// 24/32 бит
BOOL  SaveArrFile(const TCHAR* filename, const __int32* arr, 
                         int width, int height, int bpp)
{
    DWORD p_row; 
    DWORD size;
    BITMAPFILEHEADER  hdr;
    BITMAPINFO dib;
    HANDLE fp;
    DWORD  dwr = 0uL;

    if((bpp < 24) || (bpp > 32)) // только 24/32 бит
         bpp = 24;

    p_row = (DWORD)((width * bpp + 31) & ~31) / 8uL;
    size  = (DWORD)(height * p_row);

    // формируем файловый заголовок
    ZeroMemory(&hdr, sizeof(BITMAPFILEHEADER));
    hdr.bfType    = 0x4D42;
    hdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    hdr.bfSize    = hdr.bfOffBits + size;

    // заголовок описателя растра
    ZeroMemory(&dib, sizeof(BITMAPINFO));
    dib.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    dib.bmiHeader.biBitCount  = (WORD) bpp;
    dib.bmiHeader.biCompression  = BI_RGB;
    dib.bmiHeader.biPlanes  = 1u;
    dib.bmiHeader.biWidth   = (long)width;
    dib.bmiHeader.biHeight  = (long)-height;
    dib.bmiHeader.biSizeImage   = size;
    dib.bmiHeader.biXPelsPerMeter = 11811L;
    dib.bmiHeader.biYPelsPerMeter = 11811L;
    dib.bmiHeader.biClrImportant  = 0uL;
    dib.bmiHeader.biClrUsed  = 0uL;

    // далее запись в файл
    fp = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(fp == INVALID_HANDLE_VALUE)
           return FALSE;

    // записываем заголовки...
    WriteFile(fp, (LPCVOID)&hdr, sizeof(BITMAPFILEHEADER), &dwr, NULL);
    WriteFile(fp, (LPCVOID)&dib.bmiHeader, sizeof(BITMAPINFOHEADER), &dwr, NULL);

    // запись массива пикселей
    if(bpp == 32) // 32-бит
           WriteFile(fp, (LPCVOID)arr, size, &dwr, NULL);
    else if(bpp == 24) { // 24-бит с дополнением до 32-разрядной границы

          BYTE   nil = 0u;
          int   cb  = sizeof(RGBQUAD);
          int  align = ((cb - ((width*bpp + 7) / 8) % cb) % cb);
          int x, y, i;
		
          for(y = 0; y < height; y++) {
               for(x = 0; x < width; x++) 
                    WriteFile(fp, (LPCVOID)&arr[y*width+x], sizeof(RGBTRIPLE), &dwr, NULL);
			
                    for(i = 0; i < align; i++) // до границы DWORD
                          WriteFile(fp, (LPCVOID)&nil, sizeof(BYTE), &dwr, NULL);
               }
   }

   FlushFileBuffers(fp);
   CloseHandle(fp);
   return TRUE;
}


/*
Функция пересоздает битмап страницы, если его размеры не сответствуют размерам pixelX и pixelY
*/
short CS_makeBitmap(T_CRZSENS *crz, long pixelX, long pixelY) {

 // при необходимости переопределить битмап страницы
 if ((crz->pixelX != pixelX) || (crz->pixelY != pixelY)) {

  if (crz->bmp) { free(crz->bmp); } 
  // Количество пикселов в строке клиентской части экрана развертки
  // всегда четное, это контролируется vb-шной программой.
  // Поэтому не надо заморачиваться обеспечением требования к последнему
  // аргументу функции CreateBitmap :
  // "Each scan line in the rectangle must be word aligned 
  // (scan lines that are not word aligned must be padded with zeros)."

  crz->bmp = malloc (pixelX * pixelY * 4);

  if (crz->bmp == NULL) {
      sprintf (tmp_info, "crz->bmp=%ld\n",crz->bmp);
      MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"CS_makeBitmap 5", MB_OK | MB_ICONERROR);

      sprintf(lastError, "Ошибка при создании битмапа экрана");
      return 1;
  }
 }

 return 0;
}


/**************************************************************************
Функция рисует корозионную развертку crzIndx записи Handle с позиции start (мм)
При успехе  возвращает KRT_OK, KRT_ERR при ошибке.
Расширенное описание ошибки доступно через функцию krotError.
*/
short EXPORT KRTAPI CS_krotPaint (
 KRTHANDLE Handle,
 long crzIndx,
 long start,

 long pixelX,
 long pixelY,
// LPSTR fileName,
 char * bmp_buf

) {

long drawMode = 0;

T_TRACE *trc;
T_CRZSENS *crz;
long delta;
//RECT r;

 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);
 
 // закешировать хендл окна и режим вывода
 crz->drawMode = drawMode;

 if ((pixelX != crz->pixelX) || (pixelY != crz->pixelY)) { 

    sprintf(lastError, "Saved %ld:%ld not equal current %ld:%ld", crz->pixelX, crz->pixelY, pixelX, pixelY);

    sprintf (tmp_info, "pixelX=%ld, crz->pixelX=%ld, pixelY=%ld, crz->pixelY=%ld\n",
             pixelX, crz->pixelX, pixelY, crz->pixelY);
    MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"err 1", MB_OK | MB_ICONERROR);

    return KRT_ERR;
 }

 delta = (start - crz->oldPos);

 crz->pageDat = pixelX;

 if (readDataPage(trc, crzIndx, start)) {
     MessageBox(NULL, (LPCSTR)"err 2", (LPCSTR)"err 2", MB_OK | MB_ICONERROR);
     return KRT_ERR;
 }

 fillBitmapPage(crz);

// SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);

// MessageBox(NULL, (LPCSTR)fileName, (LPCSTR)fileName, MB_OK | MB_ICONERROR);

 Save_mem_ArrFile(crz->bmpBuff, crz->pixelX, crz->pixelY, 32, bmp_buf);

// CS_showBmp(crz, CS_hdc);
 crz->needRedraw = 0;
 crz->oldPos = start;

 return KRT_OK;

} // short EXPORT KRTAPI CS_krotPaint (

/* *************************************************************************
Функция вызывается при изменении размеров окна отрисовки развертки пояса crzIndx.
Формирует актуальную страницу данных и битмап для заданного пояса развертки.
При успехе  возвращает KRT_OK, KRT_ERR при ошибке.
Расширенное описание ошибки доступно через функцию krotError.
*/
short EXPORT KRTAPI CS_krotChangePic (
 KRTHANDLE Handle,
 long crzIndx,

 long pixelX,
 long pixelY
) {

T_TRACE *trc;
T_CRZSENS *crz;
long shift, ret;

char tmp_info[1024];


 trc = TraceList(Handle);
 if (trc == NULL) return KRT_ERR;
 crz = &(trc->crz[crzIndx]);

 // определить размеры окна отображения в пикселах
 if ( (pixelX == crz->pixelX) && (pixelY == crz->pixelY) )
 {
     return KRT_OK;

 } else {

    if (makeBmpBuff(crz, pixelX, pixelY)) {
        sprintf (tmp_info, "pixelX=%ld, crz->pixelX=%ld, pixelY=%ld, crz->pixelY=%ld\n",
                 pixelX, crz->pixelX, pixelY, crz->pixelY);
        MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"CS_krotChangePic 3", MB_OK | MB_ICONERROR);
        return KRT_ERR;
    }

    if  (CS_makeBitmap(crz, pixelX, pixelY)) {
        sprintf (tmp_info, "pixelX=%ld, crz->pixelX=%ld, pixelY=%ld, crz->pixelY=%ld\n",
                 pixelX, crz->pixelX, pixelY, crz->pixelY);
        MessageBox(NULL, (LPCSTR)tmp_info, (LPCSTR)"CS_krotChangePic 4", MB_OK | MB_ICONERROR);
        return KRT_ERR;
    }

    crz->pixelX = pixelX;
    crz->pixelY = pixelY;
    fillBitmapPage(crz);

  if (crz->pixelX > 0) {

   shift = krtScaleXback(pixelX, trc->scaleX) - krtScaleXback(crz->pixelX, trc->scaleX);
   if (CS_makeBitmap(crz, pixelX, pixelY)) return KRT_ERR;
   ret = (shift<0) ? decreasePage(trc, crz, -shift) : increasePage(trc, crz, shift, "krotChangePic");
   if (ret) return KRT_ERR;
   crz->pixelX = pixelX;
   crz->pixelY = pixelY;

  } else {

   shift = krtScaleXback(pixelX, trc->scaleX);
   if (makeDatBuff(crz, shift, "krotChangePic")) return KRT_ERR;
   if (makeBmpBuff(crz, pixelX, pixelY)) return KRT_ERR;
   if (CS_makeBitmap(crz, pixelX, pixelY)) return KRT_ERR;
   crz->pixelY = pixelY;
   crz->pixelX = pixelX;

   return KRT_OK;

  }

 }

 SetBitmapBits(crz->bmp, crz->pixelX * crz->pixelY * bytesPerPixel, crz->bmpBuff);
 return KRT_OK;

} // short EXPORT KRTAPI CS_krotChangePic (

