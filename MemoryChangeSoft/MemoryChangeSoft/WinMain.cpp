


#include "MemorySearch.h"
#include <process.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <commctrl.h>

//�б�ؼ�
#define LISTBOX_ADDRESS 5000

//��һ�������ڴ水ť
#define FIRSTFIND_BUTTON 6001

//ѡ����̰�ť
#define SELECTPROCESSS_BUTTON 6002

//��ʾ���̵�����ʽ�༭��
#define SHOWPROCESS_COMBOBOX 6003

//��ʾ��һ�β��ҵľ�̬����
#define FINDVALUE_STATIC 6004

//��ʾ�ı���ֵ�ľ�̬����
#define CHANGEVALUE_STATIC 6005

//��һ�β��ҵı༭��
#define FINDVALUE_EDIT 6006

//�ı���ֵ�ı༭��
#define CHANGEVALUE_EDIT 6007

//��һ�β��Ұ�ť
#define NEXTFIND_BUTTON 6008

//���²��Ұ�ť
#define FIRSTAGAIN_BUTTON 6009

//��ʾ������ʾ�ı༭��
#define SHOWTIPS_EDIT 6010

//���½����б�ť
#define UPDATEPROCESS_BUTTON 6011

//�޸�����
#define CHANGEVALUE_BUTTON 6012

//��ֵ��ַ��̬�ı�
#define VALUEADDRESS_STATIC 6013

//��ֵ��ַ�༭��
#define VALUEADDRESS_EDIT 6014

//�ð�ť����XP���
#pragma comment(linker,"\"/manifestdependency:type='win32' "\
						"name='Microsoft.Windows.Common-Controls' "\
						"version='6.0.0.0' processorArchitecture='*' "\
						"publicKeyToken='6595b64144ccf1df' language='*'\"")



CHAR g_WindowClassName[50] = "MemoryChangeSoft";
HINSTANCE g_Hinstance = NULL;
HWND g_WindowHWND = NULL;
CMemorySearch g_CE;
BOOL g_SelectProcess = FALSE;

HWND g_ShowAddress_ListBox = NULL;

HWND g_ShowProcess_ComboBox = NULL;

HWND g_FindValue_Static = NULL;
HWND g_ChangeValue_State = NULL;
HWND g_AddressValue_Static = NULL;

HWND g_FindValue_Edit = NULL;
HWND g_ChangeValue_Edit = NULL;
HWND g_ShowTips_Edit = NULL;
HWND g_ValueAddress_Edit = NULL;

HWND g_UpdateProcess_Button = NULL;
HWND g_SelectProcess_Button = NULL;
HWND g_FirstFind_Button = NULL;
HWND g_NextFind_Button = NULL;
HWND g_ChangeMemory_Button = NULL;
HWND g_FirstAgain_Button = NULL;
HWND g_ChangeValue_Button = NULL;

VOID InitWindow();
VOID DoWindow();
INT MsgLoop();
VOID GetDebugPower();
HRESULT CALLBACK Proc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);


VOID UpdateProcessList();
VOID SelectProcess();
VOID FirstFind(LPVOID);
VOID NextFind(LPVOID); 
VOID AgainFind();
VOID ChangeValue();
VOID SelectValueAddress();


int WINAPI WinMain( 
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, 
	_In_ int nShowCmd )
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	g_Hinstance = hInstance;

	InitWindow();
	DoWindow();
	return MsgLoop();
}

VOID InitWindow()
{
	WNDCLASSEXA Window = { 0 };
	Window.cbSize = sizeof(Window);
	Window.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
	Window.hCursor = ::LoadCursorA(NULL,IDC_ARROW);
	Window.hInstance = g_Hinstance;
	Window.lpfnWndProc = Proc;
	Window.hIcon = ::LoadIcon(NULL, IDI_HAND);
	Window.lpszClassName = g_WindowClassName;
	Window.style = CS_VREDRAW | CS_HREDRAW;
	assert(::RegisterClassExA(&Window) != NULL);
}

VOID DoWindow()
{
	g_WindowHWND = ::CreateWindowExA(NULL, g_WindowClassName, g_WindowClassName,
		WS_OVERLAPPEDWINDOW&~WS_THICKFRAME&~WS_MAXIMIZEBOX, 100, 100, 580, 400,
		NULL, NULL, g_Hinstance, NULL);
	assert(g_WindowHWND != NULL);

	::ShowWindow(g_WindowHWND, SW_SHOW);
	::UpdateWindow(g_WindowHWND);
	GetDebugPower();
}

INT MsgLoop()
{
	MSG Msg = { 0 };
	while (::GetMessage(&Msg,0,0,0))
	{
		::TranslateMessage(&Msg);
		::DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

VOID GetDebugPower()
{
	HANDLE hToken = NULL;
	BOOL bRet = ::OpenProcessToken(::GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken);
	if (bRet == TRUE)
	{
		TOKEN_PRIVILEGES tp = { 0 };
		tp.PrivilegeCount = 1;
		::LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		::CloseHandle(hToken);
	}
}

VOID UpdateProcessList()
{
	//��������е�����
	SendMessage(g_ShowProcess_ComboBox, CB_RESETCONTENT, 0, 0);

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == NULL)
		return;

	PROCESSENTRY32 Pro = { 0 };
	Pro.dwSize = sizeof(Pro);
	BOOL bRet = ::Process32First(hSnap, &Pro);
	CHAR szMessage[100] = { 0 };
	while (bRet)
	{
		wsprintf(szMessage, "%s-%d", Pro.szExeFile, Pro.th32ProcessID);
		//����ַ���
		SendMessage(g_ShowProcess_ComboBox, CB_ADDSTRING, 0, (LPARAM)szMessage);
		bRet = ::Process32Next(hSnap, &Pro);
	}
	::CloseHandle(hSnap);

	//��ȡ���̵�����
	UINT uCount = 0;
	uCount = ::SendMessage(g_ShowProcess_ComboBox, CB_GETCOUNT, 0, 0);

	//������ʾ�ַ���
	wsprintf(szMessage, "��ȡ���̳ɹ�,��ǰ��������Ϊ %d", uCount);
	::SetWindowText(g_ShowTips_Edit, szMessage);
}

VOID SelectProcess()
{
	//��ȡ��ǰѡ��Ľ���
	CHAR Temp[100] = { 0 };
	CHAR szNum[10] = { 0 };
	::SendMessage(g_ShowProcess_ComboBox, WM_GETTEXT, 100, (LPARAM)Temp);

	if (::strlen(Temp) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "��ѡ��һ�����̽�������...");
		return;
	}

	//��ȡ���̵�ID
	CHAR* pPos = ::strstr(Temp, "-");
	::strncpy(szNum, Temp+abs(pPos-Temp)+1, ::strlen(Temp)-abs(pPos-Temp));
	
	//ת��Ϊ����
	INT ProcessID = 0;
	::sscanf(szNum, "%d", &ProcessID);

	//���ý��̵�ID
	if (g_CE.SetTargetProcessID((DWORD)ProcessID))
	{
		::wsprintf(Temp, "�ɹ��򿪽��� %d", ProcessID);
		g_SelectProcess = TRUE;
	}
	else
	{
		::wsprintf(Temp,"�޷��򿪽��� %d�������Ƿ�Ϊϵͳ����",ProcessID);
	}
	::SetWindowText(g_ShowTips_Edit, Temp);
}

VOID FirstFind(LPVOID)
{
	//��ȡ��Ҫ���ҵ���ֵ
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_FindValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "������һ����ֵ��������...");
		return;
	}

	//���ԭ���༭�������
	::SetWindowText(g_FindValue_Edit, NULL);

	//�鿴�Ƿ�ѡ����һ�����̽��в���
	if (!g_SelectProcess)
	{
		::SetWindowText(g_ShowTips_Edit, "��ѡ��һ�����̽�������...");
		return;
	}

	//���ð�ť��״̬
	::EnableWindow(g_FirstAgain_Button, TRUE);
	::EnableWindow(g_FirstFind_Button, FALSE);

	//ת��Ϊ����
	DWORD Value = 0;
	::sscanf(szNum, "%d", &Value);

	//��һ�β���
	INT AddressNum = g_CE.FirstMemoryValueFind(Value);
	if (AddressNum < 0)
	{
		::SetWindowText(g_ShowTips_Edit, "���������޷�����");
		return;
	}

	//�����ԭ��������
	::SendMessage(g_ShowAddress_ListBox, LB_RESETCONTENT, 0, 0);

	//����µ�����
	CHAR szAddr[100] = { 0 };
	INT Index = 0;
	for (list<DWORD>::iterator it = g_CE.m_AddressList.begin();
		it != g_CE.m_AddressList.end();it++,Index++)
	{
		::wsprintf(szAddr, "0x%08x", *it);
		::SendMessage(g_ShowAddress_ListBox, LB_ADDSTRING, 0, (LPARAM)szAddr);
	}

	::wsprintf(szAddr, "�ڴ��������,��ǰ������%d����ַ",Index);
	::SetWindowText(g_ShowTips_Edit, szAddr);
}

VOID NextFind(LPVOID)
{
	//��ȡ��Ҫ���ҵ���ֵ
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_FindValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "������һ����ֵ��������...");
		return;
	}

	//�����ԭ�����б�����
	::SendMessage(g_ShowAddress_ListBox, LB_RESETCONTENT, 0, 0);

	//���ԭ���ı༭������
	::SetWindowText(g_FindValue_Edit, NULL);

	//ת��Ϊ����
	INT Value = 0;
	::sscanf(szNum, "%d", &Value);

	//�ٴ��������鿴���ж��ٸ���ַ��һ������ֵ
	INT ValueNum = g_CE.NextMemoryValueFind(Value);
	if (ValueNum < 0)
	{
		::SetWindowText(g_ShowAddress_ListBox,"��������,�޷�����");
		return;
	}

	//����µ�����
	CHAR szAddr[100] = { 0 };
	INT Index = 0;
	for (list<DWORD>::iterator it = g_CE.m_AddressList.begin();
		it != g_CE.m_AddressList.end(); it++,Index++)
	{
		::wsprintf(szAddr, "0x%08x", *it);
		::SendMessage(g_ShowAddress_ListBox, LB_ADDSTRING, 0, (LPARAM)szAddr);
	}

	//�����ʣ��һ����ַ�Ļ����Ǿ��������ַ��
	if (Index == 1)
	{
		::wsprintf(szAddr, "�����Ҹ���ֵ�ĵ�ַ,���Խ��������޸�");
		::EnableWindow(g_NextFind_Button, FALSE);
	}
	else
		::wsprintf(szAddr, "�ڴ��������,��ǰ������%d����ַ", Index);
	::SetWindowText(g_ShowTips_Edit, szAddr);
}

VOID AgainFind()
{
	::EnableWindow(g_FirstFind_Button, TRUE);
	::EnableWindow(g_NextFind_Button, TRUE);
	::EnableWindow(g_FirstAgain_Button, FALSE);

}

VOID ChangeValue()
{
	if (!g_SelectProcess)
	{
		::SetWindowText(g_ShowTips_Edit, "��ѡ��һ�����̽�������...");
		return;
	}

	//��ȡ��Ҫ���ҵ���ֵ
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_ChangeValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "������һ����ֵ��������...");
		return;
	}

	//����޸�ֵ�ı༭��
	::SetWindowText(g_ChangeValue_Edit, NULL);

	//ת��Ϊ����
	INT Value = 0;
	::sscanf(szNum, "%d", &Value);

	//��ȡ��ǰ��ֵ��ַ�༭��ĵ�ַ
	CHAR szAddress[100] = { 0 };
	CHAR szShowTips[100] = { 0 };
	::GetWindowText(g_ValueAddress_Edit, szAddress, 100);
	if (::strlen(szAddress) == NULL)
	{
		//�Ǿ���ֻ��һ����ַ�������
		if (g_CE.m_AddressList.size() == 1 && g_CE.MemoryValueChange(*g_CE.m_AddressList.begin(), Value))
			::wsprintf(szShowTips, "�Ե�ַ[%08x]��ֵ�޸ĳɹ�",*g_CE.m_AddressList.begin());
		else
			::wsprintf(szShowTips, "�Ե�ַ[%08x]��ֵ�޸�ʧ��", *g_CE.m_AddressList.begin());
	}
	else
	{
		//�кܶ��ַ�����
		//�Ȱ���ֵ��ַת������
		DWORD dwAddress = NULL;
		CHAR szTemp[20] = { 0 };
		::strncpy(szTemp, szAddress + 2, 20);
		::sscanf(szTemp, "%x", &dwAddress);

		if (g_CE.MemoryValueChange(dwAddress, Value))
			::wsprintf(szShowTips, "�Ե�ַ[%s]��ֵ�޸ĳɹ�", szAddress);
		else
			::wsprintf(szShowTips, "�Ե�ַ[%s]��ֵ�޸�ʧ��", szAddress);
	}
	::SetWindowText(g_ShowTips_Edit, szShowTips);
}

VOID SelectValueAddress()
{
	//��ȡ��ַ�б��ĵ�ַ����
	INT Num = ::SendMessage(g_ShowAddress_ListBox, LB_GETCOUNT, 0, 0);
	if (Num == NULL)
		return;

	//��ȡѡ�еĵ�ַ
	CHAR szAddress[100] = { 0 };
	INT Index = ::SendMessage(g_ShowAddress_ListBox, LB_GETCURSEL, 0, 0);
	::SendMessage(g_ShowAddress_ListBox, LB_GETTEXT, Index, (LPARAM)szAddress);

	//���͵���ֵ��ַ�༭��
	::SetWindowText(g_ValueAddress_Edit, szAddress);

	//������ʾ��
	CHAR szBuf[100] = { 0 };
	::wsprintf(szBuf, "ѡ����ֵ��ַ %s", szAddress);
	::SetWindowText(g_ShowTips_Edit, szBuf);
}

HRESULT CALLBACK Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = NULL;
	PAINTSTRUCT ps = { 0 };

	switch (uMsg)
	{
	case WM_CREATE:
		g_ShowAddress_ListBox = ::CreateWindowExA(NULL,"ListBox",NULL,
			WS_BORDER | WS_VSCROLL | WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
			0, 0, 150, 400, hWnd, (HMENU)LISTBOX_ADDRESS,g_Hinstance,NULL);

		g_ShowProcess_ComboBox = ::CreateWindowExA(NULL, "ComboBox", NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWN,
			160, 0, 200, 25, hWnd, (HMENU)SHOWPROCESS_COMBOBOX, g_Hinstance, NULL);

		g_FindValue_Static = ::CreateWindowExA(NULL, "Static", "Find Value",
			WS_CHILD | WS_VISIBLE,
			160, 50, 70 ,15, hWnd, (HMENU)FINDVALUE_STATIC, g_Hinstance, NULL);

		g_AddressValue_Static = ::CreateWindowExA(NULL, "Static", "Value Address",
			WS_CHILD | WS_VISIBLE,
			160, 80, 95, 15, hWnd, (HMENU)VALUEADDRESS_STATIC, g_Hinstance, NULL);

		g_ChangeValue_State = ::CreateWindowExA(NULL, "Static", "Change Value",
			WS_CHILD | WS_VISIBLE,
			160, 110, 90, 15, hWnd, (HMENU)CHANGEVALUE_STATIC, g_Hinstance, NULL);

		g_FindValue_Edit = ::CreateWindowExA(NULL, "Edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			260, 50, 300, 20, hWnd, (HMENU)FINDVALUE_EDIT, g_Hinstance, NULL);
		
		g_ValueAddress_Edit = ::CreateWindowExA(NULL, "Edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			260, 80, 300, 20, hWnd, (HMENU)VALUEADDRESS_EDIT, g_Hinstance, NULL);

		g_ChangeValue_Edit = ::CreateWindowExA(NULL, "Edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			260, 110, 300, 20, hWnd, (HMENU)CHANGEVALUE_EDIT, g_Hinstance, NULL);

		g_ShowTips_Edit = ::CreateWindowExA(NULL, "Edit", "There show tips",
			WS_CHILD | WS_VISIBLE | WS_BORDER ,
			160, 270, 400, 90, hWnd, (HMENU)CHANGEVALUE_EDIT, g_Hinstance, NULL);

		g_UpdateProcess_Button = ::CreateWindowExA(NULL, "Button", "Update",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			490, 0, 70, 25, hWnd, (HMENU)UPDATEPROCESS_BUTTON, g_Hinstance, NULL);

		g_SelectProcess_Button = ::CreateWindowExA(NULL, "Button", "Select Process",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			360, 0, 120, 25, hWnd, (HMENU)SELECTPROCESSS_BUTTON, g_Hinstance, NULL);

		g_FirstFind_Button = ::CreateWindowExA(NULL, "Button", "First Find",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			160, 190, 200, 70, hWnd, (HMENU)FIRSTFIND_BUTTON, g_Hinstance, NULL);

		g_NextFind_Button = ::CreateWindowExA(NULL, "Button", "Next Find",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			380, 190, 180, 30, hWnd, (HMENU)NEXTFIND_BUTTON, g_Hinstance, NULL);

		g_FirstAgain_Button = ::CreateWindowExA(NULL, "Button", "Again Find",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			380, 230, 180, 30, hWnd, (HMENU)FIRSTAGAIN_BUTTON, g_Hinstance, NULL);

		g_ChangeValue_Button = ::CreateWindowExA(NULL, "Button", "Change",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			160, 145, 400, 30, hWnd, (HMENU)CHANGEVALUE_BUTTON, g_Hinstance, NULL);

		::EnableWindow(g_FirstAgain_Button, FALSE);

		return 0;
	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_SELCHANGE)
		{
			SelectValueAddress();
			return 0;
		}
		switch (LOWORD(wParam))
		{
		case  UPDATEPROCESS_BUTTON:
			UpdateProcessList();
			break;
		case FIRSTFIND_BUTTON:
			//ʹ���߳�ɨ��
			::_beginthread(FirstFind, 0, 0);
			::SetWindowText(g_ShowTips_Edit, "�߳����������ڴ�,���Ե�..."); 
			break;
		case NEXTFIND_BUTTON:
			::_beginthread(NextFind, 0, 0);
			::SetWindowText(g_ShowTips_Edit, "�߳����������ڴ�,���Ե�...");
			break;
		case FIRSTAGAIN_BUTTON:
			AgainFind();
			break;
		case SELECTPROCESSS_BUTTON:
			SelectProcess();
			break;
		case CHANGEVALUE_BUTTON:
			ChangeValue();
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		::EndPaint(hWnd, &ps);
		return 0;
	case WM_CLOSE:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}