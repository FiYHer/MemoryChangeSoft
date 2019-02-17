#include "MemorySearch.h"




CMemorySearch::CMemorySearch()
{
	InitMemorySearch();
}

CMemorySearch::~CMemorySearch()
{
	ClearMemorySearch();
}

VOID CMemorySearch::InitMemorySearch()
{
	m_TargetProcessID = -1;
	m_MemoryStatus = { 0 };

	GetCurrentSystemInfo();
	GetCurrentSystemStatus();
}

VOID CMemorySearch::ClearMemorySearch()
{

}

VOID CMemorySearch::GetCurrentSystemInfo()
{
	::GetSystemInfo(&m_SystemInfo);
}

VOID CMemorySearch::GetCurrentSystemStatus()
{
	::GlobalMemoryStatus(&m_MemoryStatus);
}

BOOL CMemorySearch::SetTargetProcessID(DWORD TargetProcessID)
{
	if (TargetProcessID == NULL || ::GetCurrentProcessId()==TargetProcessID)
		return FALSE;

	//保存一下进程的ID
	m_TargetProcessID = TargetProcessID;

	//将上一个进程得内存信息都清空
	m_MemoryList.clear();

	//打开进程
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, m_TargetProcessID);
	if (hProcess == NULL)
		return FALSE;

	MEMORY_BASIC_INFORMATION MemoryInfo = { 0 };
	DWORD dwMemorySize = sizeof(MemoryInfo);

	//指向应用程序和动态链接库(DLL)可以访问的最低内存地址。
	DWORD dwMinAddress = (DWORD)m_SystemInfo.lpMinimumApplicationAddress;

	//指向应用程序和动态链接库(DLL)可以访问的最高内存地址。
	DWORD dwMaxAddress = (DWORD)m_SystemInfo.lpMaximumApplicationAddress;

	while (dwMinAddress < dwMaxAddress)
	{
		//查询指定进程的指定地址状态信息
		::VirtualQueryEx(hProcess, (LPVOID)dwMinAddress, &MemoryInfo, dwMemorySize);
		//把这些信息添加到链表
		m_MemoryList.push_back(MemoryInfo);
		//指向下一块内存信息[当前内存的位置加上内存区域的大小]
		dwMinAddress = (DWORD)MemoryInfo.BaseAddress + MemoryInfo.RegionSize;
	}
	//关闭资源
	::CloseHandle(hProcess);
	return TRUE;
}

DWORD CMemorySearch::FirstMemoryValueFind(DWORD dwValue)
{
	if (m_MemoryList.empty())
		return -1;

	m_AddressList.clear();

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
		m_TargetProcessID);
	if (hProcess == NULL)
		return -1;

	//页面缓冲器
	LPBYTE lpBuf = new BYTE[m_SystemInfo.dwPageSize];

	for (list<MEMORY_BASIC_INFORMATION>::iterator it = m_MemoryList.begin();
		it != m_MemoryList.end(); it++)
	{
		//如果没有调拨物理内存的话
		if (it->State != MEM_COMMIT)
			continue;

		//便利当前内存区域的所有页面
		DWORD dwSize = NULL, dwReadByte = NULL;
		while (dwSize < it->RegionSize)
		{
			LPVOID lpAddress = (LPVOID)((DWORD)it->BaseAddress+dwSize);
			if(::ReadProcessMemory(hProcess, lpAddress, lpBuf, m_SystemInfo.dwPageSize, &dwReadByte)==FALSE)
			{
				dwSize += m_SystemInfo.dwPageSize;
				continue;
			}

			//在当前页面搜索数值
			for (INT i = 0; i < (INT)m_SystemInfo.dwPageSize - 3;i++)
			{
				if (*((DWORD*)(lpBuf + i)) == dwValue)
					m_AddressList.push_back((DWORD)lpAddress + i);

			}
			dwSize += m_SystemInfo.dwPageSize;
		}
	}

	delete[] lpBuf;
	::CloseHandle(hProcess);
	return m_AddressList.size();
}

DWORD CMemorySearch::NextMemoryValueFind(DWORD dwValue)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, m_TargetProcessID);
	if (hProcess == NULL)
		return -1; 

	list<DWORD> TempList;
	DWORD dwReadByte = NULL, dwCurrentValue = NULL;
	for (list<DWORD>::iterator it = m_AddressList.begin();
		it != m_AddressList.end();it++)
	{
		::ReadProcessMemory(hProcess, (LPVOID)*it, &dwCurrentValue, sizeof(DWORD), &dwReadByte);
		if (dwCurrentValue == dwValue)
			TempList.push_back(*it);
	}
	m_AddressList.clear();
	m_AddressList.insert(m_AddressList.begin(), TempList.begin(),TempList.end());
	::CloseHandle(hProcess);
	return m_AddressList.size();
}

BOOL CMemorySearch::MemoryValueChange(DWORD dwTargetAddress, DWORD dwValue)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_TargetProcessID);
	if (hProcess == NULL)
		return FALSE;

	DWORD dwWriteByte = NULL, dwOldProtect = NULL;
	//先修改目标地址为可读可写状态
	::VirtualProtectEx(hProcess, (LPVOID)dwTargetAddress, sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
	//写入数据
	::WriteProcessMemory(hProcess, (LPVOID)dwTargetAddress, &dwValue, sizeof(DWORD), &dwWriteByte);
	//还原目标地址的状态
	::VirtualProtectEx(hProcess, (LPVOID)dwTargetAddress, sizeof(DWORD), dwOldProtect, NULL);

	::CloseHandle(hProcess);
	return TRUE;
}

