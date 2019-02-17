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

	//����һ�½��̵�ID
	m_TargetProcessID = TargetProcessID;

	//����һ�����̵��ڴ���Ϣ�����
	m_MemoryList.clear();

	//�򿪽���
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE, m_TargetProcessID);
	if (hProcess == NULL)
		return FALSE;

	MEMORY_BASIC_INFORMATION MemoryInfo = { 0 };
	DWORD dwMemorySize = sizeof(MemoryInfo);

	//ָ��Ӧ�ó���Ͷ�̬���ӿ�(DLL)���Է��ʵ�����ڴ��ַ��
	DWORD dwMinAddress = (DWORD)m_SystemInfo.lpMinimumApplicationAddress;

	//ָ��Ӧ�ó���Ͷ�̬���ӿ�(DLL)���Է��ʵ�����ڴ��ַ��
	DWORD dwMaxAddress = (DWORD)m_SystemInfo.lpMaximumApplicationAddress;

	while (dwMinAddress < dwMaxAddress)
	{
		//��ѯָ�����̵�ָ����ַ״̬��Ϣ
		::VirtualQueryEx(hProcess, (LPVOID)dwMinAddress, &MemoryInfo, dwMemorySize);
		//����Щ��Ϣ��ӵ�����
		m_MemoryList.push_back(MemoryInfo);
		//ָ����һ���ڴ���Ϣ[��ǰ�ڴ��λ�ü����ڴ�����Ĵ�С]
		dwMinAddress = (DWORD)MemoryInfo.BaseAddress + MemoryInfo.RegionSize;
	}
	//�ر���Դ
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

	//ҳ�滺����
	LPBYTE lpBuf = new BYTE[m_SystemInfo.dwPageSize];

	for (list<MEMORY_BASIC_INFORMATION>::iterator it = m_MemoryList.begin();
		it != m_MemoryList.end(); it++)
	{
		//���û�е��������ڴ�Ļ�
		if (it->State != MEM_COMMIT)
			continue;

		//������ǰ�ڴ����������ҳ��
		DWORD dwSize = NULL, dwReadByte = NULL;
		while (dwSize < it->RegionSize)
		{
			LPVOID lpAddress = (LPVOID)((DWORD)it->BaseAddress+dwSize);
			if(::ReadProcessMemory(hProcess, lpAddress, lpBuf, m_SystemInfo.dwPageSize, &dwReadByte)==FALSE)
			{
				dwSize += m_SystemInfo.dwPageSize;
				continue;
			}

			//�ڵ�ǰҳ��������ֵ
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
	//���޸�Ŀ���ַΪ�ɶ���д״̬
	::VirtualProtectEx(hProcess, (LPVOID)dwTargetAddress, sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
	//д������
	::WriteProcessMemory(hProcess, (LPVOID)dwTargetAddress, &dwValue, sizeof(DWORD), &dwWriteByte);
	//��ԭĿ���ַ��״̬
	::VirtualProtectEx(hProcess, (LPVOID)dwTargetAddress, sizeof(DWORD), dwOldProtect, NULL);

	::CloseHandle(hProcess);
	return TRUE;
}

