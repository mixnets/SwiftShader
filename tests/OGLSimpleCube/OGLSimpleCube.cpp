/******************************************************************************

@File			OGLSimpleTriangle.cpp

@Title			OpenGL Simple cube application

@Version		1.0

@Platform		Windows

@Description	Basic window with a cube drawn in it, using libGL (opengl32).
Inspired by http://www.cs.rit.edu/~ncs/Courses/570/UserGuide/OpenGLonWin-11.html

******************************************************************************/
#include "stdafx.h"
#include <windows.h>

#include <gl\GL.h>
#pragma comment (lib, "opengl32.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

const char *className = "OpenGL";
char *windowName = "OpenGL Cube";
int winX = 0, winY = 0;
int winWidth = 300, winHeight = 300;
int listIndex = 1;

HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;

void init(void)
{
	// Set viewing projection
	glMatrixMode(GL_PROJECTION);
	glFrustum(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 3.0F);

	// Position viewer
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(0.0F, 0.0F, -2.0F);

	// Position object
	glRotatef(30.0F, 1.0F, 0.0F, 0.0F);
	glRotatef(30.0F, 0.0F, 1.0F, 0.0F);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);

	// Display list
	glNewList(listIndex, GL_COMPILE);
	glBegin(GL_QUADS);
	glNormal3f(0.0F, 0.0F, 1.0F);
	glColor3f(1, 1, 1);
	glVertex3f(0.5F, 0.5F, 0.5F);
	glColor3f(1, 1, 0);
	glVertex3f(-0.5F, 0.5F, 0.5F);
	glColor3f(1, 0, 0);
	glVertex3f(-0.5F, -0.5F, 0.5F);
	glColor3f(0, 0, 0);
	glVertex3f(0.5F, -0.5F, 0.5F);

	glNormal3f(0.0F, 0.0F, -1.0F);
	glColor3f(0, 0, 1);
	glVertex3f(-0.5F, -0.5F, -0.5F);
	glColor3f(0, 0, 0);
	glVertex3f(-0.5F, 0.5F, -0.5F);
	glColor3f(0, 1, 0);
	glVertex3f(0.5F, 0.5F, -0.5F);
	glColor3f(0, 1, 0);
	glVertex3f(0.5F, -0.5F, -0.5F);

	glNormal3f(0.0F, 1.0F, 0.0F);
	glColor3f(1, 1, 1);
	glVertex3f(0.5F, 0.5F, 0.5F);
	glColor3f(0, 1, 1);
	glVertex3f(0.5F, 0.5F, -0.5F);
	glColor3f(0, 1, 0);
	glVertex3f(-0.5F, 0.5F, -0.5F);
	glColor3f(0, 0, 0);
	glVertex3f(-0.5F, 0.5F, 0.5F);

	glNormal3f(0.0F, -1.0F, 0.0F);
	glColor3f(0, 1, 0);
	glVertex3f(-0.5F, -0.5F, -0.5F);
	glColor3f(1, 1, 0);
	glVertex3f(0.5F, -0.5F, -0.5F);
	glColor3f(1, 1, 1);
	glVertex3f(0.5F, -0.5F, 0.5F);
	glColor3f(1, 1, 1);
	glVertex3f(-0.5F, -0.5F, 0.5F);

	glNormal3f(1.0F, 0.0F, 0.0F);
	glColor3f(1, 1, 1);
	glVertex3f(0.5F, 0.5F, 0.5F);
	glColor3f(1, 0, 1);
	glVertex3f(0.5F, -0.5F, 0.5F);
	glColor3f(0, 0, 1);
	glVertex3f(0.5F, -0.5F, -0.5F);
	glColor3f(0, 0, 0);
	glVertex3f(0.5F, 0.5F, -0.5F);

	glNormal3f(-1.0F, 0.0F, 0.0F);
	glColor3f(0, 0, 1);
	glVertex3f(-0.5F, -0.5F, -0.5F);
	glColor3f(0, 1, 1);
	glVertex3f(-0.5F, -0.5F, 0.5F);
	glColor3f(1, 1, 1);
	glVertex3f(-0.5F, 0.5F, 0.5F);
	glColor3f(1, 1, 1);
	glVertex3f(-0.5F, 0.5F, -0.5F);
	glEnd();
	glEndList();
}

void redraw(void)
{
	// Clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Display list to draw six faces of a cube
	glCallList(listIndex);

	SwapBuffers(hDC);
}

void resize(void)
{
	// Set viewport to cover the window
	glViewport(0, 0, winWidth, winHeight);
}

void setupPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size
		1,                              // Version
		PFD_SUPPORT_OPENGL |
		PFD_DRAW_TO_WINDOW |
		PFD_DOUBLEBUFFER,               // Support double-buffering
		PFD_TYPE_RGBA,                  // Color type
		16,                             // Prefered color depth
		0, 0, 0, 0, 0, 0,               // Color bits (ignored)
		0,                              // No alpha buffer
		0,                              // Alpha bits (ignored)
		0,                              // No accumulation buffer
		0, 0, 0, 0,                     // Accum bits (ignored)
		16,                             // Depth buffer
		0,                              // No stencil buffer
		0,                              // No auxiliary buffers
		PFD_MAIN_PLANE,                 // Main layer
		0,                              // Reserved
		0, 0, 0,                        // No layer, visible, damage masks
	};
	int pixelFormat;

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if(pixelFormat == 0) {
		MessageBox(WindowFromDC(hDC), L"ChoosePixelFormat failed.", L"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}

	if(SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
		MessageBox(WindowFromDC(hDC), L"SetPixelFormat failed.", L"Error",
			MB_ICONERROR | MB_OK);
		exit(1);
	}
}

void setupPalette(HDC hDC)
{
	int pixelFormat = GetPixelFormat(hDC);
	PIXELFORMATDESCRIPTOR pfd;
	LOGPALETTE* pPal;
	int paletteSize;

	DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if(pfd.dwFlags & PFD_NEED_PALETTE) {
		paletteSize = 1 << pfd.cColorBits;
	}
	else {
		return;
	}

	pPal = (LOGPALETTE*)
		malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
	pPal->palVersion = 0x300;
	pPal->palNumEntries = paletteSize;

	// Build a simple RGB color palette
	{
		int redMask = (1 << pfd.cRedBits) - 1;
		int greenMask = (1 << pfd.cGreenBits) - 1;
		int blueMask = (1 << pfd.cBlueBits) - 1;
		int i;

		for(i = 0; i<paletteSize; ++i) {
			pPal->palPalEntry[i].peRed =
				(((i >> pfd.cRedShift) & redMask) * 255) / redMask;
			pPal->palPalEntry[i].peGreen =
				(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
			pPal->palPalEntry[i].peBlue =
				(((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
			pPal->palPalEntry[i].peFlags = 0;
		}
	}

	hPalette = CreatePalette(pPal);
	free(pPal);

	if(hPalette) {
		SelectPalette(hDC, hPalette, FALSE);
		RealizePalette(hDC);
	}
}

int WinMain(__in HINSTANCE hCurrentInst, __in_opt HINSTANCE hPreviousInst, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	WNDCLASS wndClass;
	HWND hWnd;
	MSG msg;

	// Register window class
	wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hCurrentInst;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)BLACK_BRUSH;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"OpenGL cube";
	RegisterClass(&wndClass);

	// Create window
	hWnd = CreateWindow(
		L"OpenGL cube", L"OpenGL",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		winX, winY, winWidth, winHeight,
		NULL, NULL, hCurrentInst, NULL);

	// Display window
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// Process messages
	while(GetMessage(&msg, NULL, 0, 0) == TRUE) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_CREATE:
		// Initialize OpenGL rendering
		hDC = GetDC(hWnd);
		setupPixelFormat(hDC);
		setupPalette(hDC);
		hGLRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hGLRC);
		init();
		return 0;
	case WM_DESTROY:
		// Finish OpenGL rendering
		if(hGLRC) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hGLRC);
		}
		if(hPalette) {
			DeleteObject(hPalette);
		}
		ReleaseDC(hWnd, hDC);
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		// Track window size changes
		if(hGLRC) {
			winWidth = (int)LOWORD(lParam);
			winHeight = (int)HIWORD(lParam);
			resize();
			return 0;
		}
	case WM_PALETTECHANGED:
		// Realize palette if this is *not* the current window
		if(hGLRC && hPalette && (HWND)wParam != hWnd) {
			UnrealizeObject(hPalette);
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
			redraw();
			break;
		}
		break;
	case WM_QUERYNEWPALETTE:
		// Realize palette if this is the current window
		if(hGLRC && hPalette) {
			UnrealizeObject(hPalette);
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
			redraw();
			return TRUE;
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		if(hGLRC) {
			redraw();
		}
		EndPaint(hWnd, &ps);
		return 0;
	}
	break;
	case WM_CHAR:
		// Handle keyboard input
		switch((int)wParam) {
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			return 0;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}