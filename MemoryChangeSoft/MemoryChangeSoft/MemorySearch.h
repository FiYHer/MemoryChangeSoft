#ifndef _CMS_
#define _CMS_

#include <windows.h>
#include <assert.h>

#include <list>
using namespace std;

class CMemorySearch
{
private:
	//目标进程的ID号
	DWORD m_TargetProcessID;

	//系统基本信息
	SYSTEM_INFO m_SystemInfo;

	//内存状态
	MEMORYSTATUS m_MemoryStatus;

	//内存信息
	list<MEMORY_BASIC_INFORMATION> m_MemoryList;
public:
	//内存地址列表
	list<DWORD> m_AddressList;
private:
	//初始化
	VOID InitMemorySearch();

	//清理
	VOID ClearMemorySearch();

	//获取系统当前信息
	VOID GetCurrentSystemInfo();

	//获取系统当前内存状态
	VOID GetCurrentSystemStatus();
public:
	CMemorySearch();
	~CMemorySearch();
public:

	//设置目标进程得ID
	BOOL SetTargetProcessID(DWORD TargetProcessID);

	//第一次内存数据查找
	DWORD FirstMemoryValueFind(DWORD dwValue);

	//第二次内存数据查找
	DWORD NextMemoryValueFind(DWORD dwValue);

	//内存数据的修改
	BOOL MemoryValueChange(DWORD dwTargetAddress,DWORD dwValue);
};




#endif




