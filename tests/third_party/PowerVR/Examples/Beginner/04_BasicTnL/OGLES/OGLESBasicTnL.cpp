/******************************************************************************

 @File         OGLESBasicTnL.cpp

 @Title        Shows basic transformations and lighting

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited.

 @Platform     Independant

 @Description  Shows basic transformations and lighting

******************************************************************************/
#include "PVRShell.h"
#include <math.h>
#include <stdio.h>

#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#else
#include <GLES/gl.h>
#endif

/******************************************************************************
 Defines
******************************************************************************/
// Size of the texture we create
const int g_i32TexSize = 128;

/*!****************************************************************************
 Class implementing the PVRShell functions.
******************************************************************************/
class OGLESBasicTnL : public PVRShell
{
	// Texture handle
	GLuint	m_ui32Texture;

	// Angle to rotate the triangle
	float	m_fAngle;

	// Vertex Buffer Object (VBO) handle
	GLuint	m_ui32Vbo;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();
};


/*!****************************************************************************
 @Function		InitApplication
 @Return		bool		true if no error occured
 @Description	Code in InitApplication() will be called by PVRShell once per
				run, before the rendering context is created.
				Used to initialize variables that are not dependant on it
				(e.g. external modules, loading meshes, etc.)
				If the rendering context is lost, InitApplication() will
				not be called again.
******************************************************************************/
bool OGLESBasicTnL::InitApplication()
{
	m_fAngle = 0;
	return true;
}

/*!****************************************************************************
 @Function		QuitApplication
 @Return		bool		true if no error occured
 @Description	Code in QuitApplication() will be called by PVRShell once per
				run, just before exiting the program.
				If the rendering context is lost, QuitApplication() will
				not be called.
******************************************************************************/
bool OGLESBasicTnL::QuitApplication()
{
    return true;
}

/*!****************************************************************************
 @Function		InitView
 @Return		bool		true if no error occured
 @Description	Code in InitView() will be called by PVRShell upon
				initialization or after a change in the rendering context.
				Used to initialize variables that are dependant on the rendering
				context (e.g. textures, vertex buffers, etc.)
******************************************************************************/

// BT.601 YUV to RGB reference
//  R = (Y - 16) * 1.164              - V * -1.596
//  G = (Y - 16) * 1.164 - U *  0.391 - V *  0.813
//  B = (Y - 16) * 1.164 - U * -2.018

// Y contribution to R,G,B.  Scale and bias.
// TODO(fbarchard): Consider moving constants into a common header.
const int YG = 18997; /* round(1.164 * 64 * 256 * 256 / 257) */
const int YGB  = -1160; /* 1.164 * 64 * -16 + 64 / 2 */

// U and V contributions to R,G,B.
const int UB = -128; /* max(-128, round(-2.018 * 64)) */
const int UG = 25; /* round(0.391 * 64) */
const int VG = 52; /* round(0.813 * 64) */
const int VR = -102; /* round(-1.596 * 64) */

// Bias values to subtract 16 from Y and 128 from U and V.
const int BB = (UB * 128 + YGB);
const int BG = (UG * 128 + VG * 128 + YGB);
const int BR = (VR * 128 + YGB);

typedef unsigned char uint8;
typedef int int32;
typedef unsigned int uint32;

uint8 Clamp(int32 x)
{
	if(x < 0)
	{
		return 0;
	}
	if(x > 255)
	{
		return 255;
	}
	return x;
}

uint8 Clamp0(int32 x)
{
	if(x < 0)
	{
		return 0;
	}
	if(x > 255)
	{
		return 255;
	}
	return x;
}

uint8 Clamp01(int32 x)
{
	if(x < 0)
	{
		return 0;
	}
	if(x > 255)
	{
		return 255;
	}
	return x;
}

// C reference code that mimics the YUV assembly.
static __inline void YuvPixel(uint8 y, uint8 u, uint8 v,
                              volatile uint8* b, volatile uint8* g, volatile uint8* r) {
  uint32 y1 = (uint32)(y * 0x0101 * YG) >> 16;
  *b = Clamp01((int32)(-(u * UB) + y1 + BB) >> 6);
  *g = Clamp01((int32)(-(v * VG + u * UG) + y1 + BG) >> 6);
  *r = Clamp01((int32)(-(v * VR)+ y1 + BR) >> 6);
}

inline unsigned int align(unsigned int value, unsigned int alignment)
{
	return ((value + alignment - 1) / alignment) * alignment;
}

bool OGLESBasicTnL::InitView()
{
	// Sets the clear color
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);

	// Enables texturing
	glEnable(GL_TEXTURE_2D);

	/*
		Creates the texture.
		Please refer to the training course "Texturing" for a detailed explanation.
	*/
	/*
	volatile uint8 x;

	for(int y = 0; y < 256; y++)
		for(int u = 0; u < 256; u++)
			for(int v = 0; v < 256; v++)
	YuvPixel(y, u, v, &x, &x, &x);
	*/
	volatile unsigned int width = 176;
	volatile unsigned int height = 144;

	unsigned int YStride = align(width, 16);
	unsigned int YSize = YStride * height;
	unsigned int CStride = align(YStride / 2, 16);
 	unsigned int CSize = CStride * height / 2;

	glGenTextures(1, &m_ui32Texture);
	glBindTexture(GL_TEXTURE_2D, m_ui32Texture);
	GLubyte* pTexData = new GLubyte[YSize + 2 * CSize];

	FILE *file = fopen("carphone001.yuv", "rb");
	//fread(pTexData, 1, 176 * 144, file);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 176, 144, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pTexData);
	//fread(pTexData, 1, 176 / 2 * 144 / 2, file);
	//glTexImage2D(GL_TEXTURE_2D, 1, GL_LUMINANCE, 176 / 2, 144 / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pTexData);
	//fread(pTexData, 1, 176 / 2 * 144 / 2, file);
	//glTexImage2D(GL_TEXTURE_2D, 2, GL_LUMINANCE, 176 / 2, 144 / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pTexData);
	for(int i = 0; i < height; i++) fread(pTexData + i * YStride, 1, width, file);
	for(int i = 0; i < height / 2; i++) fread(pTexData + YSize + CSize + i * CStride, 1, width / 2, file);
	for(int i = 0; i < height / 2; i++) fread(pTexData + YSize + i * CStride, 1, width / 2, file);
	glTexImage2D(GL_TEXTURE_2D, 0, 0x32315659, 176, 144, 0, 0x32315659, GL_UNSIGNED_BYTE, pTexData);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 176, 144, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pTexData);
	fclose(file);

	//unsigned int HAL_PIXEL_FORMAT_YV12   = 0x32315659;

	//unsigned int check = 'YV12';

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	delete[] pTexData;

	// Enables lighting and light 0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/*
		Specifies the light direction.
		If the 4th component is 0, it's a parallel light (the case here).
		If the 4th component is not 0, it's a point light.
	*/
	float aLightPosition[] = {0.0f,0.0f,1.0f,0.0f};

	/*
		Assigns the light direction to the light number 0.
		This function allows you to set also the ambiant, diffuse,
		specular, emission colors of the light as well as attenuation parameters.
		We keep the other parameters to their default value in this demo.
	*/
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, aLightPosition);

	// Create VBO for the triangle from our data

	// Interleaved vertex data
	float afVertices[] = { -0.5f,-0.5f,0.0f, // Position
							0.0f,1.0f,			  // UV
							0,0,1,			  // Normal
							0.5f,-0.5f,0.0f,
							1.0f,1.0f,
							0,0,1,
							-0.5f,0.5f,0.0f,
							0.0f,0.0f,
							0,0,1,
							0.5f,0.5f,0.0f,
							1.0f,0.0f,
							0,0,1};

	glGenBuffers(1, &m_ui32Vbo);

	unsigned int uiSize = 4 * (sizeof(float) * 8); // 3 vertices * stride (8 verttypes per vertex)

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	// Set the buffer's data
	glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);

	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		ReleaseView
 @Return		bool		true if no error occured
 @Description	Code in ReleaseView() will be called by PVRShell when the
				application quits or before a change in the rendering context.
******************************************************************************/
bool OGLESBasicTnL::ReleaseView()
{
	// Frees the texture
	glDeleteTextures(1, &m_ui32Texture);

	return true;
}

/*!****************************************************************************
 @Function		RenderScene
 @Return		bool		true if no error occured
 @Description	Main rendering loop function of the program. The shell will
				call this function every frame.
				eglSwapBuffers() will be performed by PVRShell automatically.
				PVRShell will also manage important OS events.
				Will also manage relevent OS events. The user has access to
				these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLESBasicTnL::RenderScene()
{
	// Clears the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Model view matrix
	float aModelView[] =
	{
		(float)cos(m_fAngle), 0, (float)sin(m_fAngle), 0,
		0, 1, 0, 0,
		-(float)sin(m_fAngle), 0, (float)cos(m_fAngle),	0,
		0, 0, 0, 1
	};

	// Sets the matrix mode to modify the Model View matrix
	glMatrixMode(GL_MODELVIEW);

	// Loads our matrix into OpenGL Model View matrix
	glLoadMatrixf(aModelView);

	// Increments the angle of the view
//	m_fAngle += .02f;

	/*
		Draw a triangle.
		Please refer to the training course IntroducingPVRShell for a detailed explanation.
	*/

	// bind the VBO for the triangle
	glBindBuffer(GL_ARRAY_BUFFER, m_ui32Vbo);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,sizeof(float) * 8, 0);

	// Pass the texture coordinates data
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,sizeof(float) * 8, (unsigned char*) (sizeof(float) * 3));

	// Pass the normals data
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT,sizeof(float) * 8, (unsigned char*) (sizeof(float) * 5));

	// Draws a non-indexed triangle array
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// unbind the vertex buffer as we don't need it bound anymore
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

/*!****************************************************************************
 @Function		NewDemo
 @Return		PVRShell*		The demo supplied by the user
 @Description	This function must be implemented by the user of the shell.
				The user should return its PVRShell object defining the
				behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLESBasicTnL();
}

/******************************************************************************
 End of file (OGLESBasicTnL.cpp)
******************************************************************************/

