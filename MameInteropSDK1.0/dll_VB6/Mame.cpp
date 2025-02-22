// Mame.cpp : Defines the entry point for the DLL application.
//

USES_CONVERSION;

#include "stdafx.h"
#include "Mame.h"

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <winioctl.h>

// standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wtypes.h>
#include <OLEAUTO.H>
//#include <comdef.h>
//#include <comutil.h>

// MAME output header file
typedef void running_machine;
#include "output.h"

//============================================================
//  DEBUGGING
//============================================================

// note you need to compile as a console app to have any of
// these printfs show up
#define DEBUG_VERSION		0

#if DEBUG_VERSION
#define DEBUG_PRINTF(x)		printf x
#else
#define DEBUG_PRINTF(x)
#endif


//============================================================
//  CONSTANTS
//============================================================

// window styles
#define WINDOW_STYLE						WS_OVERLAPPEDWINDOW
#define WINDOW_STYLE_EX						0


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _id_map_entry id_map_entry;
struct _id_map_entry
{
	id_map_entry *			next;
	const char *			name;
	WPARAM					id;
};

typedef int (__stdcall *MAME_START)(void);
typedef int (__stdcall *MAME_STOP)(void);
typedef int (__stdcall *MAME_COPYDATA)(int id, BSTR name);
typedef int (__stdcall *MAME_UPDATESTATE)(int id, int state);

//============================================================
//  GLOBAL VARIABLES
//============================================================

HANDLE					hThread;
HINSTANCE				hInstance;

//UNICODE
//BSTR					window_class;
//BSTR					window_name;

char					window_class[256];
char					window_name[256];

int						client_id;

HWND					mame_target;
HWND					listener_hwnd;

MAME_START				mame_start;
MAME_STOP				mame_stop;
MAME_COPYDATA			mame_copydata;
MAME_UPDATESTATE		mame_updatestate;

id_map_entry *			idmaplist;

// message IDs
UINT					om_mame_start;
UINT					om_mame_stop;
UINT					om_mame_update_state;
UINT					om_mame_register_client;
UINT					om_mame_unregister_client;
UINT					om_mame_get_id_string;

//============================================================
//  FUNCTION PROTOTYPES
//============================================================

MAME_API int __stdcall init_mame(int clientid, LPSTR name, MAME_START start, MAME_STOP stop, MAME_COPYDATA copydata, MAME_UPDATESTATE updatestate);
MAME_API int __stdcall close_mame(void);
MAME_API BSTR __stdcall map_id_to_outname(WPARAM id);

int create_window_class(void);
LRESULT CALLBACK listener_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT handle_mame_start(WPARAM wparam, LPARAM lparam);
LRESULT handle_mame_stop(WPARAM wparam, LPARAM lparam);
LRESULT handle_copydata(WPARAM wparam, LPARAM lparam);
void reset_id_to_outname_cache(void);
LRESULT handle_update_state(WPARAM wparam, LPARAM lparam);

//============================================================
//  main
//============================================================

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hInstance = (HINSTANCE) hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			if(hThread != NULL)
				CloseHandle(hThread);

			DestroyWindow(listener_hwnd);
			UnregisterClass(window_class, hInstance);
			break;
	}
	return TRUE;
}

DWORD WINAPI message_loop(LPVOID lpParam)
{
	MSG message;

	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 1;
}

BSTR T2W(LPCSTR s)
{
	OLECHAR* oleChar = NULL;
	oleChar = (OLECHAR*)calloc(strlen(s)+1, sizeof(OLECHAR));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, s, -1, oleChar, strlen(s)+1);  
	BSTR bstr = SysAllocString(oleChar);
	free(oleChar);
	oleChar = NULL;

	return bstr;
}

MAME_API int __stdcall init_mame(int clientid, LPSTR name, MAME_START start, MAME_STOP stop, MAME_COPYDATA copydata, MAME_UPDATESTATE updatestate)
{
	DWORD dwThreadId, dwThrdParam = 1;
	int exitcode = 1;
	HWND otherwnd;
	int result;
	
	client_id = clientid;
	mame_start = start;
	mame_stop = stop;
	mame_copydata = copydata;
	mame_updatestate = updatestate;

	//window_class = T2W(name);
	//window_name = T2W(name);

	strcpy(window_class, name);
	strcpy(window_name, name);

	// see if there is another instance of us running
	otherwnd = FindWindow(window_class, window_name);

	// if we had another instance, defer to it
	if (otherwnd != NULL)
		return 0;

	// create our window class
	result = create_window_class();
	if (result != 0)
		goto error;

	// create a window
	listener_hwnd = CreateWindowEx(
						WINDOW_STYLE_EX,
						window_class,
						window_name,
						WINDOW_STYLE,
						0, 0,
						1, 1,
						NULL,
						NULL,
						hInstance,
						NULL);
	if (listener_hwnd == NULL)
		goto error;

	// allocate message ids
	om_mame_start = RegisterWindowMessage(OM_MAME_START);
	if (om_mame_start == 0)
		goto error;
	om_mame_stop = RegisterWindowMessage(OM_MAME_STOP);
	if (om_mame_stop == 0)
		goto error;
	om_mame_update_state = RegisterWindowMessage(OM_MAME_UPDATE_STATE);
	if (om_mame_update_state == 0)
		goto error;
	om_mame_register_client = RegisterWindowMessage(OM_MAME_REGISTER_CLIENT);
	if (om_mame_register_client == 0)
		goto error;
	om_mame_unregister_client = RegisterWindowMessage(OM_MAME_UNREGISTER_CLIENT);
	if (om_mame_unregister_client == 0)
		goto error;
	om_mame_get_id_string = RegisterWindowMessage(OM_MAME_GET_ID_STRING);
	if (om_mame_get_id_string == 0)
		goto error;

	// see if MAME is already running
	otherwnd = FindWindow(OUTPUT_WINDOW_CLASS, OUTPUT_WINDOW_NAME);
	if (otherwnd != NULL)
		handle_mame_start((WPARAM)otherwnd, 0);

	hThread = CreateThread(NULL, 0, message_loop, &dwThrdParam, 0, &dwThreadId);

	if (hThread == NULL) 
		exitcode = 0;

error:

	return exitcode;
}

MAME_API int __stdcall close_mame(void)
{
	if(hThread != NULL)
		CloseHandle(hThread);

	DestroyWindow(listener_hwnd);
	UnregisterClass(window_class, hInstance);

	return 1;
}

//============================================================
//  create_window_class
//============================================================

int create_window_class(void)
{
	WNDCLASS wc = { 0 };

	// initialize the description of the window class
	wc.lpszClassName 	= window_class;
	wc.hInstance 		= hInstance;
	wc.lpfnWndProc		= listener_window_proc;

	// register the class; fail if we can't
	if (!RegisterClass(&wc))
		return 1;

	return 0;
}


//============================================================
//  window_proc
//============================================================

LRESULT CALLBACK listener_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// OM_MAME_START: register ourselves with the new MAME (first instance only)
	if (message == om_mame_start)
		return handle_mame_start(wparam, lparam);

	// OM_MAME_STOP: no need to unregister, just note that we've stopped caring and reset the LEDs
	else if (message == om_mame_stop)
		return handle_mame_stop(wparam, lparam);

	// OM_MAME_UPDATE_STATE: update the state of this item if we care
	else if (message == om_mame_update_state)
		return handle_update_state(wparam, lparam);

	// WM_COPYDATA: extract the string and create an ID map entry
	else if (message == WM_COPYDATA)
		return handle_copydata(wparam, lparam);

	// everything else is default
	else
		return DefWindowProc(wnd, message, wparam, lparam);
}


//============================================================
//  handle_mame_start
//============================================================

LRESULT handle_mame_start(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("mame_start (%08X)\n", (UINT32)wparam));

	mame_start();

	// make this the targeted version of MAME
	mame_target = (HWND)wparam;

	reset_id_to_outname_cache();

	// register ourselves as a client
	PostMessage(mame_target, om_mame_register_client, (WPARAM)listener_hwnd, client_id);

	// get the game name
	map_id_to_outname(0);
	return 0;
}


//============================================================
//  handle_mame_stop
//============================================================

LRESULT handle_mame_stop(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("mame_stop (%08X)\n", (UINT32)wparam));

	mame_stop();

	// ignore if this is not the instance we care about
	if (mame_target != (HWND)wparam)
		return 1;

	// clear our target out
	mame_target = NULL;
	reset_id_to_outname_cache();

	return 0;
}


//============================================================
//  handle_copydata
//============================================================

LRESULT handle_copydata(WPARAM wparam, LPARAM lparam)
{
	COPYDATASTRUCT *copydata = (COPYDATASTRUCT *)lparam;
	copydata_id_string *data = (copydata_id_string *)copydata->lpData;
	id_map_entry *entry;
	char *string;

	DEBUG_PRINTF(("wparam (%08X) lparam (%08X)\n", (UINT32)wparam, (UINT32)lparam));

	DEBUG_PRINTF(("copydata.dwData (%08X) copydata.cbData (%08X) copydata.lpData (%08X)\n", copydata->dwData, copydata->cbData, copydata->lpData));

	DEBUG_PRINTF(("sizeof(*data) (%08X) strlen(data->string) (%08X) sizeof(*data) + strlen(data->string) (%08X)\n", sizeof(*data), strlen(data->string), sizeof(*data) + strlen(data->string)));

	// ignore requests we don't care about
	if (mame_target != (HWND)wparam)
		return 1;

	// allocate memory
	entry = (id_map_entry *) malloc(sizeof(entry));
	string = (char *) malloc(strlen(data->string) + 1);

	// if all allocations worked, make a new entry
	if (entry != NULL && string != NULL)
	{
		entry->next = idmaplist;
		entry->name = string;
		entry->id = data->id;

		// copy the string and hook us into the list
		strcpy(string, data->string);
		idmaplist = entry;

		DEBUG_PRINTF(("  id %d = '%s'\n", entry->id, entry->name));

		mame_copydata(data->id, SysAllocString(T2W(entry->name)));
	}

	return 0;
}


//============================================================
//  reset_id_to_outname_cache
//============================================================

void reset_id_to_outname_cache(void)
{
	// free our ID list
	while (idmaplist != NULL)
	{
		id_map_entry *temp = idmaplist;
		idmaplist = temp->next;
		free(temp);
	}
}


//============================================================
//  map_id_to_outname
//============================================================

MAME_API BSTR __stdcall map_id_to_outname(WPARAM id)
{
	id_map_entry *entry;

	// see if we have an entry in our map
	for (entry = idmaplist; entry != NULL; entry = entry->next)
		if (entry->id == id)
			return SysAllocStringByteLen(entry->name, strlen(entry->name));

	// no entry yet; we have to ask
	SendMessage(mame_target, om_mame_get_id_string, (WPARAM)listener_hwnd, id);

	// now see if we have the entry in our map
	for (entry = idmaplist; entry != NULL; entry = entry->next)
		if (entry->id == id)
			return SysAllocStringByteLen(entry->name, strlen(entry->name));

	// if not, use an empty string
	return SysAllocString(OLESTR(""));
}


//============================================================
//  handle_update_state
//============================================================

LRESULT handle_update_state(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("update_state: id=%d state=%d\n", (UINT32)wparam, (UINT32)lparam));

	mame_updatestate((UINT32) wparam, (UINT32) lparam);

	return 0;
}
