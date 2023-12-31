// ��������� � dll ������� ������ � ����� ��������� krotw32
// Defect Artificial Intelligence (DAI)

#define DAIAPI _stdcall                  // ���������� � ������ ����� ���������� ��������� � ��������� DAI
#define EXPORT __declspec(dllexport)     // ���������� ��������� ���� � �������������� ��������
#define DAIDATA long                     // ��� ������ � ������

// ������������� ������������� �������� ��� ����������� � �����.
typedef struct {

long             internal;               // 0 - �������� ������ ��� 1 - ����������
long             wallThickness;          // ������� ������ ����� � ��

// ���������� ������������� ������������� ������� ������ ������� ������, 
// ��������������, �� ��� ������, ���������� �������. 
// ���������� ���� � ��������� �������. ������ � 0.

long             x1;                     // ����� ������� ������ �������� ���� �������
long             y1;                     // ����� ������ ������ �������� ���� �������
long             x2;                     // ����� ������� ������� ������� ���� �������
long             y2;                     // ����� ������ ������� ������� ���� �������

} T_USERDAI;

// ������ ��� ������� 
typedef struct {

T_USERDAI        user;                   // ��������������� ���������� �� ������������
long             xSize;                  // ������ ������� ������ �� x
long             ySize;                  // ������ ������� ������ �� y
DAIDATA         *data;                   // ��������� ������ ������

// ��������� ������ ��������� ������� ���������� ��������� �������������� ���������
// ��������� �� ����� �� ����������, ������ ������� � ��.

long             orntStart;              // ��������� ������ ������ ������� ������ ��
                                         // ���������� ����� � �������� (1-360)
long             orntLen;                // ������ ������� �� ���������� ����� � �������� (1-360)
                                         // ���������� ������������� ����������
long             itemX;                  // ������������� �������� ������� ������ ����� ����� 
                                         // � ��. (��� ������ �������)
long             itemY;                  // ������������� �������� ������� ������ �� ���������� ����� 
                                         // � ��. (���������� ����� ���������)
} T_DAI; 

// ������ ������ explainText � ������
#define DAI_EXPLAIN_TEXT_MAX_LENGTH 4096
// �������� ������������ ��� ����������� ������ � �������� ������� ������
#define DAI_QUEST_ERROR -1L

// **************************************************************
long EXPORT DAIAPI daiQuest (
 T_DAI      *quest, 
 long (DAIAPI *informUser) (short percentDone), 
 char       *explainText

// ������� ����������� ������ ��������� quest ��� ������ �� ������:
// **************************************************************
// ����� �� ������ �������� ������ �� ������� ������-��������?  *
// **************************************************************

// �� ������ ������ ��� ������� ��������� ���� quest->data. 
// ��������! ������� �� ������ �������� �������� ��������� �������, �����������
// ���� ����������. ��� ������ ������������ ��� �������� ����������� � �����.

// ���� ������� ������� �������� ��������������� �� �������, �� �� ����� ������
// ����� ������������ �������� �� ��������� ������� informUser. � �������� ���������
// ���� ������� ����� ���������� ����� �� 0 �� 100, ������������ ��������������� �������
// ����������� ����������. ������� informUser ���������� ���� �� ���� ��������: 0 � 1.
// 0 ��������, ��� ���������� ����� ����������, 1 ��������, ��� ������������ ������ ��������
// ������.

// � ������, ���� � ���� ������� ��������� ����������� ������ (��������, ������������ ������, 
// ���������� ������� ������ ����� � �.�.), ������� ������ ������� �������� DAI_QUEST_ERROR
// ��������� �������� ������ ����� ��������� � ������ explainText. ������ ����� ������ �����
// �������� DAI_EXPLAIN_TEXT_MAX_LENGTH.

// � ������ ��������� ���������� ������� ������� ������ ������� ����� � ��������� �� 1 �� 99.
// ��� ����� �������� ������� ����������� ������� �� ������������� ��������� ������ 
// ������-������������ ������. 99 - ������������ �����������, 1 - ����������� �����������.
// � ������ explainText ����� ��������� ��������� �������� ���� �������, ���������
// � ����������� ����������.

// ������� �������� �������� ������� �������� �������� 0 � 100.

// 0 ��������, ��� �� ������������� ��������� ������ ���, � ���� ��������� ������� �����������.
// � ���� ������ � ������ explainText ����� ��������� ��������� �������� ����, ��� �������
// ���������� �� ������������� ���������.

// 100 ��������, ��� �� ������������� ��������� ���������� ����� ������� ������ �� �������,
// � ������� ����� ���������� ��������� ���� ������ (�����, �������, ��������� � �.�). 
// � ���� ������ � ������ explainText ����� ��������� ��������� �������� ���������� ������������ 
// ������ � ������������ �����.

);