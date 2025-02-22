// MameInterop.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (__stdcall *MAME_START)(void);
typedef int (__stdcall *MAME_STOP)(void);
typedef int (__stdcall *MAME_COPYDATA)(int id, const char *name);
typedef int (__stdcall *MAME_UPDATESTATE)(int id, int state);

MAME_START mame_start_ptr;
MAME_STOP mame_stop_ptr;
MAME_COPYDATA mame_copydata_ptr;
MAME_UPDATESTATE mame_updatestate_ptr;

int __stdcall mame_start(void);
int __stdcall mame_stop(void);
int __stdcall mame_copydata(int id, const char *name);
int __stdcall mame_updatestate(int id, int state);

extern int __stdcall init_mame(int clientid, const char * name, MAME_START start, MAME_STOP stop, MAME_COPYDATA copydata, MAME_UPDATESTATE updatestate);
extern int __stdcall close_mame(void);
extern LPCSTR __stdcall map_id_to_outname(WPARAM id);

typedef struct _id_map_entry id_map_entry;
struct _id_map_entry
{
	id_map_entry *			next;
	const char *			name;
	WPARAM					id;
};

void AppendTextToEditCtrl(HWND hWnd, LPCTSTR lpString);
void reset_id_to_outname_cache(void);
const char * get_name_from_id(int id);
void add_map_entry(id_map_entry *entry, int id, char *name);

id_map_entry * idmaplist;

HWND hEdit;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void AppendTextToEditCtrl(HWND hWnd, LPCTSTR pszText);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "MameInterop";
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	hWnd = CreateWindow("MameInterop", "MameInterop", WS_OVERLAPPEDWINDOW,
	0, 0, 512, 512, NULL, NULL, hInstance, NULL);

	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
	WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 
	8, 8, 488, 464, hWnd, NULL, GetModuleHandle(NULL), NULL);
	
	if(hEdit == NULL)
		MessageBox(hWnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	mame_start_ptr = mame_start;
	mame_stop_ptr = mame_stop;
	mame_copydata_ptr = mame_copydata;
	mame_updatestate_ptr = mame_updatestate;

	init_mame(1, "Test", mame_start_ptr, mame_stop_ptr, mame_copydata_ptr, mame_updatestate_ptr);

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			reset_id_to_outname_cache();
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam); 
	}
	return 0;
}

void AppendTextToEditCtrl(HWND hWnd, LPCTSTR lpString)
{
	int iPrevLen = GetWindowTextLength(hWnd);
	SendMessage(hWnd, EM_SETSEL, iPrevLen, iPrevLen);
	SendMessage(hWnd, EM_REPLACESEL, 0, (LONG) lpString);
}

void reset_id_to_outname_cache(void)
{
	while (idmaplist != NULL)
	{
		id_map_entry *temp = idmaplist;
		idmaplist = temp->next;
		free(temp);
	}
}

const char * get_name_from_id(int id)
{
	while (idmaplist != NULL)
	{
		id_map_entry *temp = idmaplist;

		if(temp->id == (WPARAM) id)
			return temp->name;

		idmaplist = temp->next;
	}

	return NULL;
}

void add_map_entry(id_map_entry *entry, int id, char *name)
{
	char *string;

	entry = (id_map_entry *) malloc(sizeof(entry));
	string = (char *) malloc(strlen(name) + 1);

	if (entry != NULL && string != NULL)
	{
		entry->next = idmaplist;
		entry->name = string;
		entry->id = id;

		strcpy(string, name);
		idmaplist = entry;
	}
}

int __stdcall mame_start(void)
{
	AppendTextToEditCtrl(hEdit, TEXT("mame_start\r\n"));

	return 1;
}

int __stdcall mame_stop(void)
{
	reset_id_to_outname_cache();

	AppendTextToEditCtrl(hEdit, TEXT("mame_stop\r\n"));

	return 1;
}

int __stdcall mame_copydata(int id, const char *name)
{
	char buf[256];

	sprintf(buf, "id %d = '%s'\r\n", id, name);

	AppendTextToEditCtrl(hEdit, buf);

	return 1;
}

int __stdcall mame_updatestate(int id, int state)
{
	char buf[256];

	LPCSTR name = get_name_from_id(id);
		
	if(name == NULL)
		name = map_id_to_outname(id);

	sprintf(buf, "updatestate: id=%d (%s) state=%d\r\n", id, name, state);
	AppendTextToEditCtrl(hEdit, buf);

	return 1;
}
