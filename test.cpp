#include <iostream>
#include <sstream>
extern "C" { 
#include <string.h>

#include "krotw32.h" 
#include "filter.h"
#include "vbapi.h"
}
#include "tester.h"

using std::cout;
extern char lastError[];

int main(int argc, char *argv[]) {

 cout << "Test suit for krotw32.dll code. (C) 2006 by Vitaly Bogomolov\n";
 KrtTester test;
 std::ostringstream o;

 // *************************************************
 // memory leak trace load test
 o.str("");
 VB_TRACE_INFO inf;
 const unsigned long passNum = 1000; 
 KRTHANDLE i; 
 for (i=0; i < passNum; i++) {
  if (krotOpenTrace (i, "C:\\user\\ogd\\data\\coronly.trc", "", "common.dll", NULL, &inf) == KRT_ERR) break;
  krotCloseTrace(i);
 }

 o << "memory leak trace load test: " << i << " from " << passNum << " passes";
 test.make(i == passNum, o.str().c_str(), lastError);

 // *************************************************
 // filter.c tests
 o.str("");
 const int sizeX = 3, sizeY = 3;
 KRTDATA inp[sizeX * sizeY];
 KRTDATA out1[sizeX * sizeY];
 KRTDATA out2[sizeX * sizeY];

 // test fltDerivation return value: must be equal array size.
 int rslt = -1;
 try {
  rslt = fltDerivation (inp, sizeX, sizeY, out1, out2);
 } catch (...) {
  o << "exeption on execute ";
 }

 o << "fltDerivation: return " << rslt << " exept " << (sizeX * sizeY);
 test.make(rslt == (sizeX * sizeY), o.str().c_str(), NULL);

 // end filter.c tests
 // *************************************************

 // *************************************************
 // krotCheckDriver tests

 char outDrvSign[KRT_TEXT];
 char outDrvName[KRT_TEXT];
 char outDrvCopyRt[KRT_TEXT];
 char outFullDllPath[KRT_TEXT];
 VB_DRV_INFO drvInfo;

 o.str("");
 if (krotCheckDriver ("common.dll", outDrvSign, outDrvName, outDrvCopyRt, outFullDllPath,  &drvInfo) == KRT_ERR) {
  cout << "krotCheckDriver call fail";
 } else {
  cout << "krotCheckDriver outFullDllPath: " << outFullDllPath << "\n";
 }
 test.make(strlen(outFullDllPath) > 0, o.str().c_str(), NULL);

 // end krotCheckDriver tests
 // *************************************************


 // *************************************************
 // LoadLibrary tests
 test.make(LoadLibrary("\\\\NTS\\ag_programs\\Krot32\\1200StrsNan512_v1.dll") != NULL, "LoadLibrary \\\\NTS\\ag_programs\\Krot32\\1200StrsNan512_v1.dll return NULL", NULL);
 test.make(LoadLibrary("\\\\NTS\\ag_programs\\Krot32\\1200CorNan512_v1.dll") != NULL, "LoadLibrary \\\\NTS\\ag_programs\\Krot32\\1200CorNan512_v1.dll return NULL", NULL);
 test.make(LoadLibrary("\\\\NTS\\ag_programs\\Krot32\\1000CorNan512_v1.dll") != NULL, "LoadLibrary \\\\NTS\\ag_programs\\Krot32\\1000CorNan512_v1.dll return NULL", NULL);

 // end LoadLibrary tests
 // *************************************************

 // *************************************************
 // print total result
 test.print();
 return test.result();
}