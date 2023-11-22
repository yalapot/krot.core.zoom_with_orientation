#define MAX_ERRSTR_LENGTH 1024

class KrtTester {
public:
KrtTester(void);
void make(bool condition, const char *msg, const char *ruMsg);
void print(void);
bool result(void);
private:
char oemMsg[MAX_ERRSTR_LENGTH];
int TestPassed;
int TestFailed;
};
