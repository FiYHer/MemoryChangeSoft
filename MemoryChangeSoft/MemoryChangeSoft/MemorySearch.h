#ifndef _CMS_
#define _CMS_

#include <windows.h>
#include <assert.h>

#include <list>
using namespace std;

class CMemorySearch
{
private:
	//Ŀ����̵�ID��
	DWORD m_TargetProcessID;

	//ϵͳ������Ϣ
	SYSTEM_INFO m_SystemInfo;

	//�ڴ�״̬
	MEMORYSTATUS m_MemoryStatus;

	//�ڴ���Ϣ
	list<MEMORY_BASIC_INFORMATION> m_MemoryList;
public:
	//�ڴ��ַ�б�
	list<DWORD> m_AddressList;
private:
	//��ʼ��
	VOID InitMemorySearch();

	//����
	VOID ClearMemorySearch();

	//��ȡϵͳ��ǰ��Ϣ
	VOID GetCurrentSystemInfo();

	//��ȡϵͳ��ǰ�ڴ�״̬
	VOID GetCurrentSystemStatus();
public:
	CMemorySearch();
	~CMemorySearch();
public:

	//����Ŀ����̵�ID
	BOOL SetTargetProcessID(DWORD TargetProcessID);

	//��һ���ڴ����ݲ���
	DWORD FirstMemoryValueFind(DWORD dwValue);

	//�ڶ����ڴ����ݲ���
	DWORD NextMemoryValueFind(DWORD dwValue);

	//�ڴ����ݵ��޸�
	BOOL MemoryValueChange(DWORD dwTargetAddress,DWORD dwValue);
};




#endif




