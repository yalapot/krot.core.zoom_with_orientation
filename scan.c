/////////////////////////////////////////////////////////////////////////////
//
// Данный файл содержит определение для функции ProccessPage
// 
/////////////////////////////////////////////////////////////////////////////
//
// эта функция анализирует фрагмент развертки с целью определения параметров трубы, 
// находящейся в этом фрагменте. 
//
// входные параметры для анализа передаются в структуре T_SCAN_INPUT (аргумент inp), 
//
// результаты анализа записываются в структуру T_SCAN_OUTPUT (аргумент out).
// обе этих структуры определены в файле scan.h
// 
// фрагмент развертки подлежащий анализу содержится в двух массивах, адресуемых полями
// row и dat структуры T_SCAN_INPUT. 
// 
// оба массива содержат показания датчиков коррозионной развертки над одним и тем же участком
// трубопровода. массив dat является производным от массива row и предоставляется для удобства 
// разработчика функции анализа фрагмента развертки.
// 
// поле row указывает на массив элементов типа long, которые содержат сырые (необработанные) 
// данные датчиков.
// поле dat указывает на массив элементов типа unsigned char, которые содержат обработанные данные,
// готовые для визуализации в виде цветовой карты.
// 
// для анализа может использоваться лыбой из массивов dat и row, либо оба.
// оба массива имеют одинаковый размер, задаваемый полями x и y структуры T_SCAN_INPUT.
// левая граница массива задает предыдущий поперечный шов. 
// 
// следующие поля структуры T_SCAN_INPUT являются справочно-информационными и задаются
// пользователем при вызове функции расстановки швов. программа может учитывать их для 
// оптимизации процесса анализа либо игнорировать.
// 
// sensitiv - коэф-нт чувствительности, задающий визуальную "яркость" швов на развертке. (1-100)
// noDRC    - если 1, то в обрабатываемом фрагменте записи нет прямошовных труб
// noSPR    - то же для спиралешовных
// noWTO    - то же для бесшовных
// 
// первая задача анализа заключается в том, чтобы найти положение (номер столбца массива) 
// крайнего левого поперечного шва на переданном фрагменте.
// этот номер надо сохранить в поле wldIndex структуры T_SCAN_OUTPUT
// 
// вторая задача заключается в анализе области между левой границей массива данных и найденным
// поперечным швом и определении типа трубы между этими поперечными швами.
// тип трубы может принимать значение одной из трех констант, определенных в файле scan.h :
// TUBE_DIRECT (прямошовная), TUBE_SPIRAL (спиралешовная), TUBE_WITHOUT (безшошная)
// выявленное значение типа трубы надо сохранить в поле wldTyp структуры T_SCAN_OUTPUT
// 
// третья задача заключается в анализе области между левой границей массива данных и найденным
// поперечным швом и определении положения продольных швов в случае прямошовной трубы или
// мест примыкания спирального шва к поперечному в случае спиралешовной трубы.
// 
// если тип трубы определен как безшовная, то данную задачу выполнять не нужно.
// 
// если тип трубы определен как спиралешовная, то надо определить место примыкания спирального 
// шва к левой границе массива как номер строки, где пересекаются левая граница и спиральный шов 
// и сохранить найденное значение в поле slit1 структуры T_SCAN_OUTPUT
// затем надо определить место примыкания спирального  шва к столбцу найденного поперечного шва
// как номер строки, где пересекаются столбец поперечного шва и спиральный шов 
// и сохранить найденное значение в поле slit2 структуры T_SCAN_OUTPUT
// 
// если тип трубы определен как прямошовная, то надо определить номер строки массива, проходящей
// наиболее близко к центру продольного шва и сохранить найденное значение в поле 
// slit1 структуры T_SCAN_OUTPUT.
// если анализом выявлено наличие второго продольного шва, то номер строки массива, определяющий
// его положение надо сохранить в поле slit2 структуры T_SCAN_OUTPUT. 
// если второй продольный шов не выявлен, в это поле надо записать значение -1.
//
// в случае успешного завершения анализа функция должна вернуть 0. 
// если в процессе анализа возникли ошибки, функция должна вернуть код ошибки больший 0.

//	КРАТКИЙ алгоритм поиска ШВОВ:
//  1. Получаем массив данных размером по Х равным size  и по Y равным numberdat
//	2. Рассчитываем условную нормальную составляющую для полученного массива с помошью функции
//		preobraz, тем самым избавляемся от пригрузов, входов и выходов в патроны и прочей лабуды...
//		Записываем полученные результаты в массивы filtr_mass(с погашением темных участков) и filtr_mass1
//	3. Фильтруем массивы filtr_mass и filtr_mass1 с помощью функции PSOBY_X, которая позволяет погасить
//		всевозможные помехи и усилить сигналы швов.
//	4. Начинаем поиск крайнего левого шва, посредством движения маски размером maska(3) на количество датчиков
//		Если в маске обнаружено наличие количества сигналов, превышающих заданный уровень чувствительности, то
//		это означает, что в этом месте находится шов. НО не все так просто, во-первых, швы обычно имеют толшину
//      превышающую размер маски, поэтому на один шов попадет несколько масок, из которых нужно выбрать
//		значение с маскимальным количеством сигналов превышающих заданный порог чувствительности. Во-вторых,
//		маска имеет размер по X от 3 до 5 (или более), поэтому нужно еще внутри самой маски найти расположение
//		шва. 
//	5. После успешного нахождения поперечного шва, нужно определеть тип труб (прямошовная, спиралешовная).
//		Поиск БЕСШОВНЫХ труб не произодится, потому что качество записи на месте бесшовных труб оставляет
//		желать лучшего (беспорядочная, ярко-красная засветка). Сначала, осуществляется поиск спиральных швов в трубе
//		посредством движения маски по Y сверху вниз. Маска наклонена под угол (ширина прок.листа/число дат).
//		Если спиральные швы не обнаружены, начинается поиск продольных швов (вдоль всей трубы сверху вниз движется
//		маска размером 3 на количество датчиков). Если не найдены ни спиральные, ни продольные швы, то труба
//		считается прямошовной без продольных швов (швы плохо различимы).     

#include "scan.h"
#include "filter.h"

#include "debuglog.h"

#define WELD 0				//поиск поперечных швов
#define SLIT 1				//поиск продольных швов

typedef struct{
 int x;						//размер массива по Х    
 int y;						//количество датчиков    
 int msk;					//размер маски           
 double mat;				//математическое ожидание
 int lvl;					//уровень сигнала 
}T_COMMON;

//структура поперечных швов
typedef struct{
 double  percent;			//чувствительность поиска 
 int min_tube;				//минимальная длина трубы
}T_POPER;

//структура спиральных швов
typedef struct{
 double  percent_spir;		//чувствительность поиска 
 int weigth;				//ширина прокатного листа
 short spirweld1;			//первый спиральный шов
 short spirweld2;			//второй спиральный шов
 short spir;				//найден/не найден
}T_SPIR;

//структура продольных швов
typedef struct{
 double  percent_prod;		//чувствительность поиска
 int number_prod_weld;		//количество продольных швов(1 или 2)
 short prodweld1;			//первый продольный шов
 short prodweld2;			//второй продольный шов
 short prod;				//найден/не найден
}T_PROD;

int invpreobraz (
KRTDATA* screen,    // входной массив     
int numberdat,      // количество датчиков
int between,        // размер массива по X
KRTDATA* screen1    // выходной массив    
) {
        int n=0;
		int j,i;
        for(j=0;j<numberdat;j++) {
           for(i=0;i<numberdat*between;i+=numberdat) {
              screen1[n]=screen[i+j];
              n++;
           }
        }
		return n;
}
void turnArray (
KRTDATA* inp,    // входной массив     
int y,			 // количество датчиков
int x,			 // размер массива по X
KRTDATA* out     // выходной массив    
) {
int j,i;
        
for(i=0; i < y; i++)
 for(j=0; j < x; j++)
  out[y*j+i] = inp[i*x+j];
}

//функция поиска поперечных швов
int findweld (
T_COMMON* cmn,           //входная структура        
KRTDATA *sob_mass,
KRTDATA *sob_mass1,
T_POPER* p,
int type
) {

 int distance=0;		//текущая дистанция
 int number_shov=0;		//количество сигналов, превышающих заданный уровень
 int in_maska=0; 
 int number_shov_maska=0;	//количество сигналов, превышающих заданные уровень внутри маски
 int distance_maska=0;		//дистанция шва в маске
												
 int i=0;
 int j=0;
 int k=0;
 int X=-1;			//положение шва поперечного (по X )
 int number=0;		//количество сигналов, превышающих заданный уровень

    //пробегание маской по всему массиву данных
    for(i=0;i<cmn->y*(cmn->x-cmn->msk);i+=cmn->y)
    {
		for(j=0;j<cmn->y*cmn->msk;j++)
        {
			//if((sob_mass[i+j]>mat+level)||( (sob_mass1[i+j]>(mat+level))&&(sob_mass[i+j]<(mat+level))))
			if((sob_mass[i+j]>(cmn->mat+cmn->lvl))||( (sob_mass1[i+j]>(cmn->mat+cmn->lvl))&&(sob_mass[i+j]<(cmn->mat+cmn->lvl))))
			  //if(filtr_mass[i+j]>(mat+level))
            {
				number_shov++;//подсчет количества сигналов превышающих заданный уровень
            }
        }
		//проверка условия превышения количества сигналов над заданным уровнем чувствительности
        if(number_shov>p->percent*cmn->y*cmn->msk)
        {
			in_maska=0;				//число сигналов в маске	
            number_shov_maska=0;	//количество сигналов, превышающих заданные уровень внутри маски
            distance_maska=0;		//дистанция шва в маске
			//поиск наибольшего количеста сигналов прев. зад. уровень в маске
            for (j=0;j<cmn->y*cmn->msk;j+=cmn->y)
            {
				for(k=0;k<cmn->y;k++)
                {
					if((sob_mass[i+j+k]>cmn->mat+cmn->lvl)||(sob_mass1[i+j+k]>cmn->mat+cmn->lvl))
                    {
						number_shov_maska++;						number_shov_maska++;

                    }
                }
                if(number_shov_maska>in_maska)
                {
					in_maska=number_shov_maska;
                    distance_maska++;
                }
                number_shov_maska=0;
			}
			//Определение дистанции крайнего левого шва с учетом минимальной длины трубы
			if(number_shov>number&&((distance+distance_maska)>p->min_tube))
			{
				
				if((distance-X)>p->min_tube&&number!=0&&X>p->min_tube&&type==WELD)
				{
					i=cmn->y*cmn->x;
					j=cmn->y*cmn->msk;
				}
				else
				{
					number=number_shov;
					X=distance+distance_maska;
				}
			}
           
		}
        number_shov=0;
        distance++;
	}
	return X;
}

//функция поиска спиральных швов
int findspirweld (
int tube_space,          //размер массива по Х                
int numberdat,           //количество датчиков                
int maska,               //размер маски                       
double mat,              //математическое ожидание	     
int level,               //уровень сигнала                    
KRTDATA *screen2,        //входной  массив, в котором ищут швы
T_SPIR* spr
) {
        //Определение шва в начале трубы
        double num_spir=0;              //число сигналов превышающих барьер
        double num_spir_inv=0;		//число сигналов превышающих барьер
        int spirtube=0;
        int spirtube1=0;
        short dist_num_spir=0;
	int t,j,z;
        int number_spir=0;
        int number_spir_inv=0;
        int sensor=0;
        int x=0;			//нормальный наклон
        int x_inv=0;			//обратный наклон спиральной трубы 
	int zap=0;
	int zap_inv=0;
        if (tube_space>numberdat)
        {
                for(t=0;t<numberdat*tube_space;t+=tube_space)
                {
                        number_spir=0;
                        number_spir_inv=0;
                        sensor=0;

                        for (j=t;j<numberdat*tube_space;j+=tube_space)
                        {
                                x=(sensor*spr->weigth)/numberdat;						//нормальный наклон
                                x_inv=((numberdat-sensor)*spr->weigth)/numberdat;		//обратный наклон спиральной трубы
                                for(z=0;z<maska;z++)
                                {
                                        if(screen2[z+j+(x)]>mat+level)
                                        {
                                                number_spir++;
                                        }
                                      /*  if(screen2[z+j+(x_inv)]>mat+level)
                                        {
                                                number_spir_inv++;
                                        }*/
                                }
                                sensor++;
                        }
                        for (j=0;j<t;j+=tube_space)
                        {
                                x=(sensor*spr->weigth)/numberdat;
                                x_inv=((numberdat-sensor)*spr->weigth)/numberdat;		//обратный наклон спиральной трубы
                                for(z=0;z<maska;z++)
                                {
                                        if(screen2[z+j+(x)]>mat+level)
                                        {
                                                number_spir++;
                                        }
                                        /*if(screen2[z+j+(x_inv)]>mat+level)
                                        {
                                                number_spir_inv++;
                                        }*/
                                }
                                sensor++;
                        }
                        if(number_spir>((spr->percent_spir)*(numberdat)*maska))
                        {
                                if (num_spir<number_spir)
                                {
                                        num_spir=number_spir;
                                        dist_num_spir=(short)(t/tube_space);
                                        //t=0;
                                        spirtube=1;
                                }
                        }
                        if(number_spir_inv>((spr->percent_spir)*(numberdat)*maska))
                        {
                                if (num_spir_inv<number_spir_inv)
                                {
                                        num_spir_inv=number_spir_inv;
                                        dist_num_spir=(short)(t/tube_space);
                                        //t=0;
                                        spirtube=1;
                                }
                        }
                        number_spir=0;
                        number_spir_inv=0;
                }
                if(spirtube==1)
                {
                        spr->spirweld1=dist_num_spir;
						spr->spir=1;
                }
                num_spir=0;
                num_spir_inv=0;
                //Определение шва в конце трубы
                for(t=numberdat*tube_space;t>0;t-=tube_space)
                {
                        number_spir=0;
                        number_spir_inv=0;
                        sensor=0;
                        for (j=t;j>0;j-=tube_space)
                        {
                                x=(sensor*spr->weigth)/numberdat;
                                x_inv=((numberdat-sensor)*spr->weigth)/numberdat;
                                for(z=0;z<maska;z++)
                                {
                                        zap=j-z-x;
                                        zap_inv=j-z-x_inv;
                                        if(screen2[abs(zap)]>mat+level)
                                        {
                                                number_spir++;
                                        }
                                       /*f(screen2[abs(zaplatka_inv)]>mat+level)
                                        {
                                                number_spir_inv++;
                                        }*/
                                }
                                sensor++;
                        }
                        for (j=numberdat*tube_space;j>t;j-=tube_space)
                        {
                                x=(sensor*spr->weigth)/numberdat;
                                x_inv=((numberdat-sensor)*spr->weigth)/numberdat;
                                for(z=0;z<maska;z++)
                                {
                                        if(screen2[j-z-(x)]>mat+level)
                                        {
                                                number_spir++;
                                        }
                                        /*(screen2[j-z-(int)(floor(x_inv))]>mat+level)
                                        {
                                                number_spir_inv++;
                                        }*/
                                }
                                sensor++;
                        }
                        if(number_spir>((spr->percent_spir)*(numberdat)*maska))
                        {
                                if (num_spir<number_spir)
                                {
                                        num_spir=number_spir;
                                        dist_num_spir=(short)(t/tube_space);
                                        //t=0;
                                        spirtube1=1;
                                }
                        }
                        if(number_spir_inv>((spr->percent_spir)*(numberdat)*maska))
                        {
                                if (num_spir_inv<number_spir_inv)
                                {
                                        num_spir_inv=number_spir_inv;
                                        dist_num_spir=(short)(t/tube_space);
                                        //t=0;
                                        spirtube1=1;
                                }
                        } 
                        number_spir=0;
                        number_spir_inv=0;
                }
                if(spirtube1==1)
                {
                        spr->spirweld2=dist_num_spir;
						spr->spir=1;
                }
        }

        if (spirtube!=1) {
         return (spirtube1==1) ? 1 : 0;
        }  
        return 1;
}
// главная функция модуля, вызываемая снаружи
short ProccessPage (
const T_SCAN_INPUT *inp,		// входные данные
T_SCAN_OUTPUT *out				// место для результатов анализа
) {

KRTDATA *flt1, *flt2, *tube1, *tube2;
long i,x,arrSize;
double mat=0.0;					// матожидание
//структуры швов
T_COMMON cmn;
T_POPER p;
T_SPIR spr;
T_PROD prd;

 //общие данные	
 cmn.x=inp->x;					//размер массива по Х    
 cmn.y=inp->y;					//количество датчиков    
 cmn.msk=inp->maskSize;			//размер маски           
 cmn.mat=0.0;					//математическое ожидание
 cmn.lvl=inp->signalLevel;		//уровень сигнала 
 //поперчные швы
 p.percent=1.0-inp->weldSensitiv/100.0;		// чувствительность поиска поперечных швов
 p.min_tube=inp->minTubeLen;	// минимальная длина трубы

 //спиральные швы
 spr.percent_spir=1.0-inp->spirSensitiv/100.0;	// чувствительность поиска 
 spr.weigth=inp->listSize;		// ширина прокатного листа
 spr.spirweld1=-1;				// первый спиральный шов
 spr.spirweld2=-1;				// второй спиральный шов
 spr.spir=-1;
 
 //продольные швы
 prd.percent_prod=1.0-inp->slitSensitiv/100.0;
 prd.number_prod_weld=inp->slitNum;		// количество продольных швов(1 или 2)
 prd.prodweld1=-1;						// первый спиральный шов
 prd.prodweld2=-1;						// второй спиральный шов
 prd.prod=-1;							// найден/не найден

 // массив сырых данных не используется, делаем из него четыре байтовых массива
 // для внутенних нужд
 arrSize = inp->y * inp->x;
 flt1    = (KRTDATA*) inp->row;   //массив производной с погашенными темными участками
 flt2    = flt1 + arrSize;
 tube1   = flt2 + arrSize;
 tube2   = tube1 + arrSize;
 // расчет условной нормальной составляющей (убирает пригруза)
 fltDerivation(inp->dat, inp->x, inp->y, flt1, flt2);
    
 //Фильтрация фильтром Смаруня
 fltSobel(flt1,inp->x,inp->y,tube1);
 fltSobel(flt2,inp->x,inp->y,tube2);
 //подсчет математического ожидания для сигналов или, проще сказать, среднего арифмитеческого
 for(i=0; i < arrSize; i++) {
	mat+=tube1[i];
 }
 cmn.mat=mat/arrSize;
 //Поиск поперечных швов
 x = findweld(&cmn, tube1, tube2, &p,WELD);

 //Поиск поперечных швов закончен
 if(inp->noSPR!=1||inp->noDRC!=1&&x>0) {
  arrSize = x * inp->y;
	
	//fltSobel(inp->dat,inp->x,inp->y,flt1);
  fltDerivationY(inp->dat,inp->x,inp->y, tube2);
  //усиление сигналов продольных швов
  for (i=0; i < arrSize; i++) {
	  tube1[i] = (KRTDATA) (tube2[i]+ MIDLE_LEVEL);
	 // tube1[i] = (KRTDATA) (tube2[i]);
  }
  //tube1   = flt1 + arrSize;
  //расчет матожидания участка
  mat=0.0;
  for(i=0; i < arrSize; i++) {
	mat+=tube1[i];
  }
  mat=mat/arrSize;
  //поиск спиральных швов
  if(inp->noSPR!=1) {
   invpreobraz(tube1,inp->y,x, flt1);
   findspirweld(x, inp->y, inp->maskSize, mat, inp->signalLevel, tube1, &spr);
  }
  //поиск продольных швов
  if(inp->noDRC!=1&&spr.spir!=1) {
   //переворот массива для поиска продольных швов;
   turnArray(tube1,x,inp->y/inp->slitNum, flt1);  
   cmn.x=inp->y/inp->slitNum;					//размер массива по Х    
   cmn.y=x;										//количество датчиков    
   cmn.msk=inp->maskSize;						//размер маски           
   cmn.mat=mat;									//математическое ожидание 
   cmn.lvl=inp->signalLevel;					//уровень сигнала  
   p.min_tube=0;								//минимальная длина трубы
   p.percent=prd.percent_prod;					//чувствительность поиска продольных швов
   prd.prodweld1 = (short)(findweld(&cmn,flt1,flt1, &p,SLIT));
   if(prd.prodweld1>0) {
		prd.prod=1;
   }
   if(inp->slitNum==2) //продольных шва два
   {
	for (i=arrSize/2; i < arrSize; i++) {
	  tube1[i-arrSize/2] = (KRTDATA) (tube2[i]+ MIDLE_LEVEL);
	}
	turnArray(tube1,x,inp->y/inp->slitNum, flt1);
	cmn.x=inp->y/inp->slitNum;					//размер массива по Х    
	cmn.y=x;									//количество датчиков    
    cmn.msk=inp->maskSize;						//размер маски           
    cmn.mat=mat;								//математическое ожидание 
    cmn.lvl=inp->signalLevel;					//уровень сигнала  
    p.min_tube=0;								//минимальная длина трубы
    p.percent=prd.percent_prod;					//чувствительность поиска продольных швов
    prd.prodweld2 = (short)(findweld(&cmn,flt1,flt1, &p,SLIT)+inp->y/2);
    if(prd.prodweld1>0) {
		prd.prod=1;
	}
   }
  }

 }
 //дистанция поперечного шва
 out->wldIndex = x;

 // тип трубы - спиралешовная
 if(spr.spir==1) {
  out->wldTyp = TUBE_SPIRAL;
  out->slit1 =spr.spirweld1;
  out->slit2 =spr.spirweld2;

 } else if(prd.prod==1)	{

  out->wldTyp = TUBE_DIRECT;
  if (prd.prodweld1!=-1&&prd.prodweld2!=-1&&abs(prd.prodweld1-prd.prodweld2) < 10) {
   out->slit1 =(short)((prd.prodweld1+prd.prodweld2)/2);
   out->slit2 =-1;		
  } else {
   out->slit1 =prd.prodweld1;
   out->slit2 =prd.prodweld2;
  }

 }

 if(prd.prod!=1&&spr.spir!=1) {
  out->wldTyp = TUBE_DIRECT;
  out->slit1 =-1;
  out->slit2 =-1;
 }

 return 0;
}
