#include <iostream>
#include <windows.h>
#include "tester.h"

using std::cout;

KrtTester::KrtTester(void) {
 TestPassed = 0;
 TestFailed = 0;
}

bool KrtTester::result(void) {
 return (TestFailed > 0);
}

void KrtTester::make(bool condition, const char *msg, const char *ruMsg) {
 if (condition) {
  TestPassed++;
 } else {
  TestFailed++;
  cout << "FAILED: " << msg << "\n";
  if (ruMsg) {
   CharToOem(ruMsg, oemMsg);
   cout << oemMsg << "\n";
  }
 }
}

void KrtTester::print(void) {
 if (TestFailed > 0) {
  cout << "FAIL. passed: " << TestPassed << " failed: " << TestFailed;
 } else {
  cout << "OK. test passed: " << TestPassed;
 } 
}
