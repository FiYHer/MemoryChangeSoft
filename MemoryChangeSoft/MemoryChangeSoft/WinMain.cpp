


#include "MemorySearch.h"
#include <process.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <commctrl.h>

//列表控件
#define LISTBOX_ADDRESS 5000

//第一次搜索内存按钮
#define FIRSTFIND_BUTTON 6001

//选择进程按钮
#define SELECTPROCESSS_BUTTON 6002

//显示进程的下拉式编辑框
#define SHOWPROCESS_COMBOBOX 6003

//显示第一次查找的静态文字
#define FINDVALUE_STATIC 6004

//显示改变数值的静态文字
#define CHANGEVALUE_STATIC 6005

//第一次查找的编辑框
#define FINDVALUE_EDIT 6006

//改变数值的编辑框
#define CHANGEVALUE_EDIT 6007

//下一次查找按钮
#define NEXTFIND_BUTTON 6008

//重新查找按钮
#define FIRSTAGAIN_BUTTON 6009

//显示操作提示的编辑框
#define SHOWTIPS_EDIT 6010

//更新进程列表按钮
#define UPDATEPROCESS_BUTTON 6011

//修改数据
#define CHANGEVALUE_BUTTON 6012

//数值地址静态文本
#define VALUEADDRESS_STATIC 6013

//数值地址编辑框
#define VALUEADDRESS_EDIT 6014

//让按钮具有XP风格
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
	//先清除所有的数据
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
		//添加字符串
		SendMessage(g_ShowProcess_ComboBox, CB_ADDSTRING, 0, (LPARAM)szMessage);
		bRet = ::Process32Next(hSnap, &Pro);
	}
	::CloseHandle(hSnap);

	//获取进程得数量
	UINT uCount = 0;
	uCount = ::SendMessage(g_ShowProcess_ComboBox, CB_GETCOUNT, 0, 0);

	//更换提示字符串
	wsprintf(szMessage, "获取进程成功,当前进程数量为 %d", uCount);
	::SetWindowText(g_ShowTips_Edit, szMessage);
}

VOID SelectProcess()
{
	//获取当前选择的进程
	CHAR Temp[100] = { 0 };
	CHAR szNum[10] = { 0 };
	::SendMessage(g_ShowProcess_ComboBox, WM_GETTEXT, 100, (LPARAM)Temp);

	if (::strlen(Temp) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "请选择一个进程进行搜索...");
		return;
	}

	//获取进程得ID
	CHAR* pPos = ::strstr(Temp, "-");
	::strncpy(szNum, Temp+abs(pPos-Temp)+1, ::strlen(Temp)-abs(pPos-Temp));
	
	//转化为数字
	INT ProcessID = 0;
	::sscanf(szNum, "%d", &ProcessID);

	//设置进程的ID
	if (g_CE.SetTargetProcessID((DWORD)ProcessID))
	{
		::wsprintf(Temp, "成功打开进程 %d", ProcessID);
		g_SelectProcess = TRUE;
	}
	else
	{
		::wsprintf(Temp,"无法打开进程 %d，请检查是否为系统进程",ProcessID);
	}
	::SetWindowText(g_ShowTips_Edit, Temp);
}

VOID FirstFind(LPVOID)
{
	//获取需要查找的数值
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_FindValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "请输入一个数值进行搜索...");
		return;
	}

	//清除原来编辑框的数据
	::SetWindowText(g_FindValue_Edit, NULL);

	//查看是否选择了一个进程进行操作
	if (!g_SelectProcess)
	{
		::SetWindowText(g_ShowTips_Edit, "请选择一个进程进行搜索...");
		return;
	}

	//设置按钮的状态
	::EnableWindow(g_FirstAgain_Button, TRUE);
	::EnableWindow(g_FirstFind_Button, FALSE);

	//转化为整形
	DWORD Value = 0;
	::sscanf(szNum, "%d", &Value);

	//第一次查找
	INT AddressNum = g_CE.FirstMemoryValueFind(Value);
	if (AddressNum < 0)
	{
		::SetWindowText(g_ShowTips_Edit, "发生错误，无法查找");
		return;
	}

	//先清空原来的数据
	::SendMessage(g_ShowAddress_ListBox, LB_RESETCONTENT, 0, 0);

	//添加新的数据
	CHAR szAddr[100] = { 0 };
	INT Index = 0;
	for (list<DWORD>::iterator it = g_CE.m_AddressList.begin();
		it != g_CE.m_AddressList.end();it++,Index++)
	{
		::wsprintf(szAddr, "0x%08x", *it);
		::SendMessage(g_ShowAddress_ListBox, LB_ADDSTRING, 0, (LPARAM)szAddr);
	}

	::wsprintf(szAddr, "内存搜索完毕,当前搜索到%d个地址",Index);
	::SetWindowText(g_ShowTips_Edit, szAddr);
}

VOID NextFind(LPVOID)
{
	//获取需要查找的数值
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_FindValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "请输入一个数值进行搜索...");
		return;
	}

	//先清空原来的列表数据
	::SendMessage(g_ShowAddress_ListBox, LB_RESETCONTENT, 0, 0);

	//清空原来的编辑框数据
	::SetWindowText(g_FindValue_Edit, NULL);

	//转化为整形
	INT Value = 0;
	::sscanf(szNum, "%d", &Value);

	//再次搜索，查看还有多少个地址是一样的数值
	INT ValueNum = g_CE.NextMemoryValueFind(Value);
	if (ValueNum < 0)
	{
		::SetWindowText(g_ShowAddress_ListBox,"发生错误,无法查找");
		return;
	}

	//添加新的数据
	CHAR szAddr[100] = { 0 };
	INT Index = 0;
	for (list<DWORD>::iterator it = g_CE.m_AddressList.begin();
		it != g_CE.m_AddressList.end(); it++,Index++)
	{
		::wsprintf(szAddr, "0x%08x", *it);
		::SendMessage(g_ShowAddress_ListBox, LB_ADDSTRING, 0, (LPARAM)szAddr);
	}

	//如果还剩下一个地址的话，那就是这个地址了
	if (Index == 1)
	{
		::wsprintf(szAddr, "查找找该数值的地址,可以进行数据修改");
		::EnableWindow(g_NextFind_Button, FALSE);
	}
	else
		::wsprintf(szAddr, "内存搜索完毕,当前搜索到%d个地址", Index);
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
		::SetWindowText(g_ShowTips_Edit, "请选择一个进程进行搜索...");
		return;
	}

	//获取需要查找的数值
	CHAR szNum[100] = { 0 };
	::GetWindowText(g_ChangeValue_Edit, szNum, 100);
	if (::strlen(szNum) == NULL)
	{
		::SetWindowText(g_ShowTips_Edit, "请输入一个数值进行搜索...");
		return;
	}

	//清空修改值的编辑框
	::SetWindowText(g_ChangeValue_Edit, NULL);

	//转化为整形
	INT Value = 0;
	::sscanf(szNum, "%d", &Value);

	//获取当前数值地址编辑框的地址
	CHAR szAddress[100] = { 0 };
	CHAR szShowTips[100] = { 0 };
	::GetWindowText(g_ValueAddress_Edit, szAddress, 100);
	if (::strlen(szAddress) == NULL)
	{
		//那就是只有一个地址的情况了
		if (g_CE.m_AddressList.size() == 1 && g_CE.MemoryValueChange(*g_CE.m_AddressList.begin(), Value))
			::wsprintf(szShowTips, "对地址[%08x]数值修改成功",*g_CE.m_AddressList.begin());
		else
			::wsprintf(szShowTips, "对地址[%08x]数值修改失败", *g_CE.m_AddressList.begin());
	}
	else
	{
		//有很多地址的情况
		//先把数值地址转化出来
		DWORD dwAddress = NULL;
		CHAR szTemp[20] = { 0 };
		::strncpy(szTemp, szAddress + 2, 20);
		::sscanf(szTemp, "%x", &dwAddress);

		if (g_CE.MemoryValueChange(dwAddress, Value))
			::wsprintf(szShowTips, "对地址[%s]数值修改成功", szAddress);
		else
			::wsprintf(szShowTips, "对地址[%s]数值修改失败", szAddress);
	}
	::SetWindowText(g_ShowTips_Edit, szShowTips);
}

VOID SelectValueAddress()
{
	//获取地址列表框的地址数量
	INT Num = ::SendMessage(g_ShowAddress_ListBox, LB_GETCOUNT, 0, 0);
	if (Num == NULL)
		return;

	//获取选中的地址
	CHAR szAddress[100] = { 0 };
	INT Index = ::SendMessage(g_ShowAddress_ListBox, LB_GETCURSEL, 0, 0);
	::SendMessage(g_ShowAddress_ListBox, LB_GETTEXT, Index, (LPARAM)szAddress);

	//发送到数值地址编辑框
	::SetWindowText(g_ValueAddress_Edit, szAddress);

	//更新提示框
	CHAR szBuf[100] = { 0 };
	::wsprintf(szBuf, "选中数值地址 %s", szAddress);
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
			//使用线程扫描
			::_beginthread(FirstFind, 0, 0);
			::SetWindowText(g_ShowTips_Edit, "线程正在搜索内存,请稍等..."); 
			break;
		case NEXTFIND_BUTTON:
			::_beginthread(NextFind, 0, 0);
			::SetWindowText(g_ShowTips_Edit, "线程正在搜索内存,请稍等...");
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