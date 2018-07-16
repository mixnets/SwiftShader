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

// Shader.cpp: Implements the Shader class and its  derived classes
// VertexShader and FragmentShader. Implements GL shader objects and related
// functionality. [OpenGL ES 2.0.24] section 2.10 page 24 and section 3.8 page 84.

#include "Shader.h"

#include "main.h"
#include "utilities.h"

#include "glslang/Public/ShaderLang.h"
#include "StandAlone/ResourceLimits.h"
#include "SPIRV/GlslangToSpv.h"
#include "OGLCompilersDLL/InitializeDll.h"
#include "SPIRV/disassemble.h"

#include <string>
#include <algorithm>
#include <array>

namespace es2
{
bool Shader::compilerInitialized = false;

Shader::Shader(ResourceManager *manager, GLuint handle) : mHandle(handle), mResourceManager(manager)
{
	mSource = nullptr;

	clear();

	mRefCount = 0;
	mDeleteStatus = false;
}

Shader::~Shader()
{
	delete[] mSource;
}

GLuint Shader::getName() const
{
	return mHandle;
}

void Shader::setSource(GLsizei count, const char *const *string, const GLint *length)
{
	delete[] mSource;
	int totalLength = 0;

	for(int i = 0; i < count; i++)
	{
		if(length && length[i] >= 0)
		{
			totalLength += length[i];
		}
		else
		{
			totalLength += (int)strlen(string[i]);
		}
	}

	mSource = new char[totalLength + 1];
	char *code = mSource;

	for(int i = 0; i < count; i++)
	{
		int stringLength;

		if(length && length[i] >= 0)
		{
			stringLength = length[i];
		}
		else
		{
			stringLength = (int)strlen(string[i]);
		}

		strncpy(code, string[i], stringLength);
		code += stringLength;
	}

	mSource[totalLength] = '\0';
}

size_t Shader::getInfoLogLength() const
{
	if(infoLog.empty())
	{
		return 0;
	}
	else
	{
	   return infoLog.size() + 1;
	}
}

void Shader::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLogOut)
{
	int index = 0;

	if(bufSize > 0)
	{
		if(!infoLog.empty())
		{
			index = std::min(bufSize - 1, (GLsizei)infoLog.size());
			memcpy(infoLogOut, infoLog.c_str(), index);
		}

		infoLogOut[index] = '\0';
	}

	if(length)
	{
		*length = index;
	}
}

size_t Shader::getSourceLength() const
{
	if(!mSource)
	{
		return 0;
	}
	else
	{
	   return strlen(mSource) + 1;
	}
}

void Shader::getSource(GLsizei bufSize, GLsizei *length, char *source)
{
	int index = 0;

	if(bufSize > 0)
	{
		if(mSource)
		{
			index = std::min(bufSize - 1, (int)strlen(mSource));
			memcpy(source, mSource, index);
		}

		source[index] = '\0';
	}

	if(length)
	{
		*length = index;
	}
}

sh::TranslatorASM *Shader::createCompiler(GLenum shaderType)
{
	if(!compilerInitialized)
	{
		sh::InitCompilerGlobals();
		compilerInitialized = true;
	}

	sh::TranslatorASM *assembler = new sh::TranslatorASM(this, shaderType);

	sh::ShBuiltInResources resources;
	resources.MaxVertexAttribs = MAX_VERTEX_ATTRIBS;
	resources.MaxVertexUniformVectors = MAX_VERTEX_UNIFORM_VECTORS;
	resources.MaxVaryingVectors = MAX_VARYING_VECTORS;
	resources.MaxVertexTextureImageUnits = MAX_VERTEX_TEXTURE_IMAGE_UNITS;
	resources.MaxCombinedTextureImageUnits = MAX_COMBINED_TEXTURE_IMAGE_UNITS;
	resources.MaxTextureImageUnits = MAX_TEXTURE_IMAGE_UNITS;
	resources.MaxFragmentUniformVectors = MAX_FRAGMENT_UNIFORM_VECTORS;
	resources.MaxDrawBuffers = MAX_DRAW_BUFFERS;
	resources.MaxVertexOutputVectors = MAX_VERTEX_OUTPUT_VECTORS;
	resources.MaxFragmentInputVectors = MAX_FRAGMENT_INPUT_VECTORS;
	resources.MinProgramTexelOffset = MIN_PROGRAM_TEXEL_OFFSET;
	resources.MaxProgramTexelOffset = MAX_PROGRAM_TEXEL_OFFSET;
	resources.OES_standard_derivatives = 1;
	resources.OES_fragment_precision_high = 1;
	resources.OES_EGL_image_external = 1;
	resources.EXT_draw_buffers = 1;
	resources.ARB_texture_rectangle = 1;
	resources.MaxCallStackDepth = 64;
	assembler->Init(resources);

	return assembler;
}

void Shader::clear()
{
	infoLog.clear();

	varyings.clear();
	activeUniforms.clear();
	activeAttributes.clear();
}

void Shader::compile()
{
	clear();

	createShader();
	sh::TranslatorASM *compiler = createCompiler(getType());

	// Ensure we don't pass a nullptr source to the compiler
	const char *source = "\0";
	if(mSource)
	{
		source = mSource;
	}

	bool success = compiler->compile(&source, 1, sh::SH_OBJECT_CODE);

	if(false)
	{
		static int serial = 1;

		if(false)
		{
			char buffer[256];
			sprintf(buffer, "shader-input-%d-%d.txt", getName(), serial);
			FILE *file = fopen(buffer, "wt");
			fprintf(file, "%s", mSource);
			fclose(file);
		}

		getShader()->print("shader-output-%d-%d.txt", getName(), serial);

		serial++;
	}

	shaderVersion = compiler->getShaderVersion();
	infoLog += compiler->getInfoSink().info.c_str();

	if(!success)
	{
		deleteShader();

		TRACE("\n%s", infoLog.c_str());
	}

	delete compiler;

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	std::string vertexSource = mSource;
//	std::string fragmentSource = glFragmentShader->getTranslatedSource(glContext);

	// Parse attribute locations and replace them in the vertex shader.
	// See corresponding code in OutputVulkanGLSL.cpp.
	// TODO(jmadill): Also do the same for ESSL 3 fragment outputs.
	//for (const sh::Attribute &attribute : programState.getAttributes())
	//{
	//	// Warning: If we endup supporting ES 3.0 shaders and up, Program::linkAttributes is going
	//	// to bring us all attributes in this list instead of only the active ones.
	//	ASSERT(attribute.active);

	//	std::string locationString = "location = " + Str(attribute.location);
	//	InsertLayoutSpecifierString(&vertexSource, attribute.name, locationString);
	//	InsertQualifierSpecifierString(&vertexSource, attribute.name, "in");
	//}

	// The attributes in the programState could have been filled with active attributes only
	// depending on the shader version. If there is inactive attributes left, we have to remove
	// their @@ QUALIFIER and @@ LAYOUT markers.
	//for (const sh::Attribute &attribute : glVertexShader->getAllAttributes(glContext))
	//{
	//	if (attribute.active)
	//	{
	//		continue;
	//	}

	//	InsertLayoutSpecifierString(&vertexSource, attribute.name, "");
	//	InsertQualifierSpecifierString(&vertexSource, attribute.name, "");
	//}

	//// Assign varying locations.
	//for (const gl::PackedVaryingRegister &varyingReg : resources.varyingPacking.getRegisterList())
	//{
	//	const auto &varying = *varyingReg.packedVarying;

	//	std::string locationString = "location = " + Str(varyingReg.registerRow);
	//	if (varyingReg.registerColumn > 0)
	//	{
	//		ASSERT(!varying.varying->isStruct());
	//		ASSERT(!gl::IsMatrixType(varying.varying->type));
	//		locationString += ", component = " + Str(varyingReg.registerColumn);
	//	}

	//	InsertLayoutSpecifierString(&vertexSource, varying.varying->name, locationString);
	//	InsertLayoutSpecifierString(&fragmentSource, varying.varying->name, locationString);

	//	ASSERT(varying.interpolation == sh::INTERPOLATION_SMOOTH);
	//	InsertQualifierSpecifierString(&vertexSource, varying.varying->name, "out");
	//	InsertQualifierSpecifierString(&fragmentSource, varying.varying->name, "in");
	//}

	//// Remove all the markers for unused varyings.
	//for (const std::string &varyingName : resources.varyingPacking.getInactiveVaryingNames())
	//{
	//	EraseLayoutAndQualifierStrings(&vertexSource, &fragmentSource, varyingName);
	//}

	//// Bind the default uniforms for vertex and fragment shaders.
	//// See corresponding code in OutputVulkanGLSL.cpp.
	//std::stringstream searchStringBuilder;
	//searchStringBuilder << "@@ DEFAULT-UNIFORMS-SET-BINDING @@";
	//std::string searchString = searchStringBuilder.str();

	//std::string vertexDefaultUniformsBinding = "set = 0, binding = 0";
	//std::string fragmentDefaultUniformsBinding = "set = 0, binding = 1";

	//angle::ReplaceSubstring(&vertexSource, searchString, vertexDefaultUniformsBinding);
	//angle::ReplaceSubstring(&fragmentSource, searchString, fragmentDefaultUniformsBinding);

	//// Assign textures to a descriptor set and binding.
	//int textureCount = 0;
	//const auto &uniforms = programState.getUniforms();
	//for (unsigned int uniformIndex : programState.getSamplerUniformRange())
	//{
	//	const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];
	//	std::string setBindingString = "set = 1, binding = " + Str(textureCount);

	//	// Samplers in structs are extracted and renamed.
	//	const std::string samplerName = GetMappedSamplerName(samplerUniform.name);

	//	ASSERT(samplerUniform.isActive(gl::ShaderType::Vertex) ||
	//		samplerUniform.isActive(gl::ShaderType::Fragment));
	//	if (samplerUniform.isActive(gl::ShaderType::Vertex))
	//	{
	//		InsertLayoutSpecifierString(&vertexSource, samplerName, setBindingString);
	//	}
	//	InsertQualifierSpecifierString(&vertexSource, samplerName, kUniformQualifier);

	//	if (samplerUniform.isActive(gl::ShaderType::Fragment))
	//	{
	//		InsertLayoutSpecifierString(&fragmentSource, samplerName, setBindingString);
	//	}
	//	InsertQualifierSpecifierString(&fragmentSource, samplerName, kUniformQualifier);

	//	textureCount++;
	//}

	//// Start the unused sampler bindings at something ridiculously high.
	//constexpr int kBaseUnusedSamplerBinding = 100;
	//int unusedSamplerBinding = kBaseUnusedSamplerBinding;

	//for (const gl::UnusedUniform &unusedUniform : resources.unusedUniforms)
	//{
	//	if (unusedUniform.isSampler)
	//	{
	//		// Samplers in structs are extracted and renamed.
	//		std::string uniformName = GetMappedSamplerName(unusedUniform.name);

	//		std::stringstream layoutStringStream;

	//		layoutStringStream << "set = 0, binding = " << unusedSamplerBinding++;

	//		std::string layoutString = layoutStringStream.str();

	//		InsertLayoutSpecifierString(&vertexSource, uniformName, layoutString);
	//		InsertLayoutSpecifierString(&fragmentSource, uniformName, layoutString);

	//		InsertQualifierSpecifierString(&vertexSource, uniformName, kUniformQualifier);
	//		InsertQualifierSpecifierString(&fragmentSource, uniformName, kUniformQualifier);
	//	}
	//	else
	//	{
	//		EraseLayoutAndQualifierStrings(&vertexSource, &fragmentSource, unusedUniform.name);
	//	}
	//}

	//// Substitute layout and qualifier strings for the driver uniforms block.
	//constexpr char kDriverBlockLayoutString[] = "set = 2, binding = 0";
	//constexpr char kDriverBlockName[] = "ANGLEUniforms";
	//InsertLayoutSpecifierString(&vertexSource, kDriverBlockName, kDriverBlockLayoutString);
	//InsertLayoutSpecifierString(&fragmentSource, kDriverBlockName, kDriverBlockLayoutString);

	//InsertQualifierSpecifierString(&vertexSource, kDriverBlockName, kUniformQualifier);
	//InsertQualifierSpecifierString(&fragmentSource, kDriverBlockName, kUniformQualifier);

//	std::array<const char *, 2> strings = { { vertexSource.c_str(), fragmentSource.c_str() } };
//	std::array<int, 2> lengths = {
//		{ static_cast<int>(vertexSource.length()), static_cast<int>(fragmentSource.length()) } };

//glslang::InitProcess();
ShInitialize();

	std::array<const char *, 1> strings = { { vertexSource.c_str()} };
	std::array<int, 1> lengths = {
		{ static_cast<int>(vertexSource.length()) } };

////	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = EShMsgDefault;// static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

	glslang::TShader vertexShader(EShLangFragment);
	vertexShader.setStringsWithLengths(&strings[0], &lengths[0], 1);
	vertexShader.setEntryPoint("main");

	TBuiltInResource builtInResources(glslang::DefaultTBuiltInResource);
	//GetBuiltInResourcesFromCaps(glCaps, &builtInResources);

	bool vertexResult =
		vertexShader.parse(&builtInResources, 100, EEsProfile, false, false, messages);
	if (!vertexResult)
	{
		/*return gl::InternalError() << "Internal error parsing Vulkan vertex shader:\n"
			<< vertexShader.getInfoLog() << "\n"
			<< vertexShader.getInfoDebugLog() << "\n";*/
	}

	/*glslang::TShader fragmentShader(EShLangFragment);
	fragmentShader.setStringsWithLengths(&strings[1], &lengths[1], 1);
	fragmentShader.setEntryPoint("main");
	bool fragmentResult =
		fragmentShader.parse(&builtInResources, 450, ECoreProfile, false, false, messages);
	if (!fragmentResult)
	{
		return gl::InternalError() << "Internal error parsing Vulkan fragment shader:\n"
			<< fragmentShader.getInfoLog() << "\n"
			<< fragmentShader.getInfoDebugLog() << "\n";
	}*/

	glslang::TProgram program;
	program.addShader(&vertexShader);
	//program.addShader(&fragmentShader);
	bool linkResult = program.link(messages);
	if (!linkResult)
	{
		//return gl::InternalError() << "Internal error linking Vulkan shaders:\n"
		//	<< program.getInfoLog() << "\n";
	}

	glslang::TIntermediate *vertexStage = program.getIntermediate(EShLangFragment);
//	glslang::TIntermediate *fragmentStage = program.getIntermediate(EShLangFragment);

	std::vector<uint32_t> vertexCodeOut;

	glslang::GlslangToSpv(*vertexStage, vertexCodeOut);
	//glslang::GlslangToSpv(*fragmentStage, *fragmentCodeOut);

	std::ostringstream out;

	spv::Disassemble(out, vertexCodeOut);
	std::string s = out.str();
	const char *a = s.c_str();

	assert(a);
}

bool Shader::isCompiled()
{
	return getShader() != 0;
}

void Shader::addRef()
{
	mRefCount++;
}

void Shader::release()
{
	mRefCount--;

	if(mRefCount == 0 && mDeleteStatus)
	{
		mResourceManager->deleteShader(mHandle);
	}
}

unsigned int Shader::getRefCount() const
{
	return mRefCount;
}

bool Shader::isFlaggedForDeletion() const
{
	return mDeleteStatus;
}

void Shader::flagForDeletion()
{
	mDeleteStatus = true;
}

void Shader::releaseCompiler()
{
	sh::FreeCompilerGlobals();
	compilerInitialized = false;
}

// true if varying x has a higher priority in packing than y
bool Shader::compareVarying(const glsl::Varying &x, const glsl::Varying &y)
{
	if(x.type == y.type)
	{
		return x.size() > y.size();
	}

	switch(x.type)
	{
	case GL_FLOAT_MAT4: return true;
	case GL_FLOAT_MAT2:
		switch(y.type)
		{
		case GL_FLOAT_MAT4: return false;
		case GL_FLOAT_MAT2: return true;
		case GL_FLOAT_VEC4: return true;
		case GL_FLOAT_MAT3: return true;
		case GL_FLOAT_VEC3: return true;
		case GL_FLOAT_VEC2: return true;
		case GL_FLOAT:      return true;
		default: UNREACHABLE(y.type);
		}
		break;
	case GL_FLOAT_VEC4:
		switch(y.type)
		{
		case GL_FLOAT_MAT4: return false;
		case GL_FLOAT_MAT2: return false;
		case GL_FLOAT_VEC4: return true;
		case GL_FLOAT_MAT3: return true;
		case GL_FLOAT_VEC3: return true;
		case GL_FLOAT_VEC2: return true;
		case GL_FLOAT:      return true;
		default: UNREACHABLE(y.type);
		}
		break;
	case GL_FLOAT_MAT3:
		switch(y.type)
		{
		case GL_FLOAT_MAT4: return false;
		case GL_FLOAT_MAT2: return false;
		case GL_FLOAT_VEC4: return false;
		case GL_FLOAT_MAT3: return true;
		case GL_FLOAT_VEC3: return true;
		case GL_FLOAT_VEC2: return true;
		case GL_FLOAT:      return true;
		default: UNREACHABLE(y.type);
		}
		break;
	case GL_FLOAT_VEC3:
		switch(y.type)
		{
		case GL_FLOAT_MAT4: return false;
		case GL_FLOAT_MAT2: return false;
		case GL_FLOAT_VEC4: return false;
		case GL_FLOAT_MAT3: return false;
		case GL_FLOAT_VEC3: return true;
		case GL_FLOAT_VEC2: return true;
		case GL_FLOAT:      return true;
		default: UNREACHABLE(y.type);
		}
		break;
	case GL_FLOAT_VEC2:
		switch(y.type)
		{
		case GL_FLOAT_MAT4: return false;
		case GL_FLOAT_MAT2: return false;
		case GL_FLOAT_VEC4: return false;
		case GL_FLOAT_MAT3: return false;
		case GL_FLOAT_VEC3: return false;
		case GL_FLOAT_VEC2: return true;
		case GL_FLOAT:      return true;
		default: UNREACHABLE(y.type);
		}
		break;
	case GL_FLOAT: return false;
	default: UNREACHABLE(x.type);
	}

	return false;
}

VertexShader::VertexShader(ResourceManager *manager, GLuint handle) : Shader(manager, handle)
{
	vertexShader = 0;
}

VertexShader::~VertexShader()
{
	delete vertexShader;
}

GLenum VertexShader::getType() const
{
	return GL_VERTEX_SHADER;
}

int VertexShader::getSemanticIndex(const std::string &attributeName) const
{
	if(!attributeName.empty())
	{
		for(const auto &attribute : activeAttributes)
		{
			if(attribute.name == attributeName)
			{
				return attribute.registerIndex;
			}
		}
	}

	return -1;
}

sw::Shader *VertexShader::getShader() const
{
	return vertexShader;
}

sw::VertexShader *VertexShader::getVertexShader() const
{
	return vertexShader;
}

void VertexShader::createShader()
{
	delete vertexShader;
	vertexShader = new sw::VertexShader();
}

void VertexShader::deleteShader()
{
	delete vertexShader;
	vertexShader = nullptr;
}

FragmentShader::FragmentShader(ResourceManager *manager, GLuint handle) : Shader(manager, handle)
{
	pixelShader = 0;
}

FragmentShader::~FragmentShader()
{
	delete pixelShader;
}

GLenum FragmentShader::getType() const
{
	return GL_FRAGMENT_SHADER;
}

sw::Shader *FragmentShader::getShader() const
{
	return pixelShader;
}

sw::PixelShader *FragmentShader::getPixelShader() const
{
	return pixelShader;
}

void FragmentShader::createShader()
{
	delete pixelShader;
	pixelShader = new sw::PixelShader();
}

void FragmentShader::deleteShader()
{
	delete pixelShader;
	pixelShader = nullptr;
}

}
