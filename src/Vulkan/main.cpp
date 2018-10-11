// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// main.cpp: DLL entry point.

#include "VkDevice.hpp"

#if defined(_WIN32)
#include "resource.h"
#include <windows.h>

#ifdef DEBUGGER_WAIT_DIALOG
static INT_PTR CALLBACK DebuggerWaitDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		GetWindowRect(GetDesktopWindow(), &rect);
		SetWindowPos(hwnd, HWND_TOP, rect.right / 2, rect.bottom / 2, 0, 0, SWP_NOSIZE);
		SetTimer(hwnd, 1, 100, NULL);
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hwnd, 0);
		}
		break;
	case WM_TIMER:
		if(IsDebuggerPresent())
		{
			EndDialog(hwnd, 0);
		}
	}

	return FALSE;
}

static void WaitForDebugger(HINSTANCE instance)
{
	if(!IsDebuggerPresent())
	{
		HRSRC dialog = FindResource(instance, MAKEINTRESOURCE(IDD_DIALOG1), RT_DIALOG);
		DLGTEMPLATE *dialogTemplate = (DLGTEMPLATE*)LoadResource(instance, dialog);
		DialogBoxIndirect(instance, dialogTemplate, NULL, DebuggerWaitDialogProc);
	}
}
#endif

void *VKAPI_PTR allocate(
	void*                                       pUserData,
	size_t                                      size,
	size_t                                      alignment,
	VkSystemAllocationScope                     allocationScope)
{
	auto *x = new uint8_t[size];
	memset(x, 0x11, size);
	return x;
}

int main()
{
	VkAllocationCallbacks callback;

	callback.pUserData = (void*)1;
	callback.pfnAllocation = allocate;
	callback.pfnFree = nullptr;
	//callback.

	auto x = sizeof(vk::Device);
	auto y = sizeof(vk::DispatchableDevice);

	vk::DispatchableDevice *device = new (&callback) vk::DispatchableDevice(nullptr, 0, nullptr);

	VkDevice handle = *device;

	vk::Device *cast = vk::DispatchableDevice::Cast(handle);

	assert(cast->virtualFunctionToForceCreatingVTable() == 77);

	//device->destroy(nullptr);

	//device->~vk::DispatchableDevice();
	//::operator delete(device);

//	delete device;

	return 0;
}
#endif
