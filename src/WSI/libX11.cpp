// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "libX11.hpp"

#include "System/SharedLibrary.hpp"

#include <memory>

#define Bool int

namespace {

template <typename FPTR>
void getFuncAddress(void *lib, const char *name, FPTR *out)
{
	*out = reinterpret_cast<FPTR>(getProcAddress(lib, name));
}

} // anonymous namespace

LibX11exports::LibX11exports(void *libX11, void *libXext, void *libXbc)
{
	getFuncAddress(libX11, "XOpenDisplay", &XOpenDisplay);
	getFuncAddress(libX11, "XGetWindowAttributes", &XGetWindowAttributes);
	getFuncAddress(libX11, "XDefaultScreenOfDisplay", &XDefaultScreenOfDisplay);
	getFuncAddress(libX11, "XWidthOfScreen", &XWidthOfScreen);
	getFuncAddress(libX11, "XHeightOfScreen", &XHeightOfScreen);
	getFuncAddress(libX11, "XPlanesOfScreen", &XPlanesOfScreen);
	getFuncAddress(libX11, "XDefaultGC", &XDefaultGC);
	getFuncAddress(libX11, "XDefaultDepth", &XDefaultDepth);
	getFuncAddress(libX11, "XMatchVisualInfo", &XMatchVisualInfo);
	getFuncAddress(libX11, "XDefaultVisual", &XDefaultVisual);
	getFuncAddress(libX11, "XSetErrorHandler", &XSetErrorHandler);
	getFuncAddress(libX11, "XSync", &XSync);
	getFuncAddress(libX11, "XCreateImage", &XCreateImage);
	getFuncAddress(libX11, "XCloseDisplay", &XCloseDisplay);
	getFuncAddress(libX11, "XPutImage", &XPutImage);
	getFuncAddress(libX11, "XDrawString", &XDrawString);

	getFuncAddress(libXext, "XShmQueryExtension", &XShmQueryExtension);
	getFuncAddress(libXext, "XShmCreateImage", &XShmCreateImage);
	getFuncAddress(libXext, "XShmAttach", &XShmAttach);
	getFuncAddress(libXext, "XShmDetach", &XShmDetach);
	getFuncAddress(libXext, "XShmPutImage", &XShmPutImage);

	getFuncAddress(libXbc, "xcb_create_gc", &xcb_create_gc);
	getFuncAddress(libXbc, "xcb_flush", &xcb_flush);
	getFuncAddress(libXbc, "xcb_free_gc", &xcb_free_gc);
	getFuncAddress(libXbc, "xcb_generate_id", &xcb_generate_id);
	getFuncAddress(libXbc, "xcb_get_geometry", &xcb_get_geometry);
	getFuncAddress(libXbc, "xcb_get_geometry_reply", &xcb_get_geometry_reply);
	getFuncAddress(libXbc, "xcb_put_image", &xcb_put_image);
}

LibX11exports *LibX11::operator->()
{
	return loadExports();
}

LibX11exports *LibX11::loadExports()
{
	static auto exports = []
	{
		auto libX11 = getProcAddress(RTLD_DEFAULT, "XOpenDisplay") ?
		              RTLD_DEFAULT : loadLibrary("libX11.so");
		auto libXext = getProcAddress(RTLD_DEFAULT, "XShmQueryExtension") ?
		              RTLD_DEFAULT : loadLibrary("libXext.so");
		auto libXcb = getProcAddress(RTLD_DEFAULT, "xcb_create_gc") ?
		              RTLD_DEFAULT : loadLibrary("libXcb.so");

		if (!libX11) { libX11 = RTLD_DEFAULT; }
		if (!libXext) { libXext = RTLD_DEFAULT; }
		if (!libXcb) { libXcb = RTLD_DEFAULT; }

		return std::unique_ptr<LibX11exports>(new LibX11exports(libX11, libXext, libXcb));
	}();

	return exports.get();
}

LibX11 libX11;
