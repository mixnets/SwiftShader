// Copyright 2017 The SwiftShader Authors. All Rights Reserved.
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

// TODO: Debug macros of the GLSL compiler clash with core SwiftShader's.
// They should not be exposed through the interface headers above.
#undef ASSERT
#undef UNIMPLEMENTED

#include <cassert>
#include <cstdint>
#include <memory>

namespace {

// TODO(cwallez@google.com): Like in ANGLE, disable most of the pool allocator for fuzzing
// This is a helper class to make sure all the resources used by the compiler are initialized
class ScopedPoolAllocatorAndTLS
{
public:
	ScopedPoolAllocatorAndTLS()
	{
		InitializeParseContextIndex();
		InitializePoolIndex();
		SetGlobalPoolAllocator(&allocator);
	}
	~ScopedPoolAllocatorAndTLS()
	{
		SetGlobalPoolAllocator(nullptr);
		FreePoolIndex();
		FreeParseContextIndex();
	}

private:
	TPoolAllocator allocator;
};

// Trivial implementation of the glsl::Shader interface that fakes being an API-level
// shader object.
class FakeVS : public glsl::Shader
{
public:
	FakeVS(sw::VertexShader *bytecode)
	    : bytecode(bytecode)
	{
	}

	sw::Shader *getShader() const override
	{
		return bytecode;
	}
	sw::VertexShader *getVertexShader() const override
	{
		return bytecode;
	}

private:
	sw::VertexShader *bytecode;
};

// The fuzzer will specify a list of templates and mutations.
// A template is specified via a number representing the possible pieces of
// boilerplate code available to the fuzzer. A template is a list of code
// blocks.
//
// A code block is a minimal chunk of code with a list of inputs and outputs.
// Each input and output has a type. Each input can be represented as either a
// variable, a code block, or raw data. Raw data is only valid if the input is a
// non-struct non-pointer type. Each output is a variable. Either list can be
// empty. All potential base codeblocks are enumerated in a list. The fuzzer
// chooses from them via an integer.
//
// A mutation is a modification to the last specified template. A mutation does
// one of the following things:
//	* Inserts a new codeblock to a specified index in the template
//	* Deletes the codeblock from the specific index in the template
//	* Reorders two codeblocks
//	* Modifies the inputs to the codeblock at the specified index in the template
//
// The list of templates and mutations must begin with a template.
// When a new template is specified within the list, all mutations following
// apply only to it, until the next template is specified.

/******  Node definitions ******/
// These are how the fuzzer organizes its input.
// We interpret the random data as headers specifying whether the next data is
// a template or a mutation on the current active template.

class Node
{
	// Can either be:
	//  * Template
	//  * Mutation
	NodeType nodeType;
};

class Template : public Node
{
	// Index into global TemplateObject List
	int idx;
};

//// Mutations
// A mutation modifies the current active template by either inserting a new
// codeblock, deleting a codeblock, or reordering codeblocks.

enum MutationType
{
	MutInsertion,
	MutDeletion,
	MutReorder
};

class Mutation : public Node
{
	// Can either be:
	//  * Insertion
	//  * Deletion (Doesn't have its own class)
	//  * Reorder
	MutationType mutationType;

	// Index into the first codeblock in the active template affected by this
	// mutation
	int templateCBIndex1;
};

class Mut_Insert : public Mutation
// Adds a new code block to the active template
{
	// Index into the global CodeBlockObject list
	int globalCodeblockIndex;
};

class Mut_Reorder : Public Mutation
{
	// Index of the 2nd code block in the active template to swap CBidx1 with.
	int templateCBIndex2;
};

/****** Object definitions ******/
// These represent the actual API code that we will run.
// A template object is a list of codeblocks, and each codeblock is an
// unchangeable set of C++ code that has several inputs and outputs.

class TemplateObject
{
	std::vector<Codeblock> codeBlocks;
};

class Codeblock
{
	// Index into the global CodeblockObject table.
	int definition;

	// Index into the global Codeblock list. I.e. when this codeBlock is actually
	// used.
	int useIdx;
};

enum DataTypeEnum
{
	DT_Float,
	DT_Int32,
	DT_Uint32
};

struct DataType
{
	size_t size;
	DataTypeEnum name;
};

class Input
{
	DataType type;
	bool isVariable;

	// If this Input is a variable, then *(int *)data is an integer index into
	// the global VariableObject table.
	// Otherwise, data is N-bits of raw data filled in randomly, where N is the
	// size of the DataType.
	void *data;
	friend std::ostream &operator<<(std::ostream &os, const Input &obj);
}

struct Variable
{
	// Index into the VariableObject memory pool
	int idx;
}

struct VariableObject
{
	std::string name;
	DataType type;

	// Index of the codeblock that creates this variable.
	// No codeblock prior to this index may reference this variable.
	int cbIdx;

	// data that is currently within the variable.
	void *data;

	friend std::ostream &operator<<(std::ostream &os, const Input &obj);
}

class CodeBlockObject
{
	// Name of the code block
	std::string name;

	std::vector<Input> inputs;
	std::vector<Variable> outputs;

	CodeBlockObject(std::string name)
	    : name(name)
	{}
	void (*execute)(std::vector<Input> inputs, std::vector<Variable> outputs);

	// Makes the codeblock read its inputs, perform its functionality, then
	// write to its outputs.
	void run()
	{
		execute(inputs, outputs);
	}

	friend std::ostream &operator<<(std::ostream &os, const CodeBlockObject &obj);
}

class NodeList
{
	size_t count;
	std::vector<Node> nodes;
	NodeList()
	    : count = 0
	{}
};

}  // anonymous namespace

struct State
{
	NodeList n;
	std::vector<CodeBlockObject> codeblocks;
	std::vector<TemplateObject> templates;
}

void
init()
{
	CodeBlockObject
	    TemplateObject first;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	// Data layout:
	//
	// byte n: Number of nodes
	// {
	//   byte: Type of node {0 = Template, 1 = Mutation}
	//   [Template]
	//   int: Index into template list
	//   [Mutation]
	//   byte: Mutation type {0 = Insertion, 1 = Deletion, 2 = Reorder}
	//   int: Index into codeblock list within current active template
	//		[Insertion]
	//		int: Index of codeblock to insert from global codeblock list
	//		[Deletion]
	//		[Reorder]
	//		int Index into codeblock list within current active template
	// } [n]
	//
	// char data[] // Data used for creating random variables and data.
	//
	const size_t headerSize = 1 + 4;

	if(size <= headerSize)
	{
		return 0;
	}

	init();

	std::unique_ptr<ScopedPoolAllocatorAndTLS> allocatorAndTLS(new ScopedPoolAllocatorAndTLS);
	std::unique_ptr<sw::VertexShader> shader(new sw::VertexShader);
	std::unique_ptr<FakeVS> fakeVS(new FakeVS(shader.get()));

	std::unique_ptr<TranslatorASM> glslCompiler(new TranslatorASM(fakeVS.get(), GL_VERTEX_SHADER));

	// TODO(cwallez@google.com): have a function to init to default values somewhere
	ShBuiltInResources resources;
	resources.MaxVertexAttribs = sw::MAX_VERTEX_INPUTS;
	resources.MaxVertexUniformVectors = sw::VERTEX_UNIFORM_VECTORS - 3;
	resources.MaxVaryingVectors = MIN(sw::MAX_VERTEX_OUTPUTS, sw::MAX_VERTEX_INPUTS);
	resources.MaxVertexTextureImageUnits = sw::VERTEX_TEXTURE_IMAGE_UNITS;
	resources.MaxCombinedTextureImageUnits = sw::TEXTURE_IMAGE_UNITS + sw::VERTEX_TEXTURE_IMAGE_UNITS;
	resources.MaxTextureImageUnits = sw::TEXTURE_IMAGE_UNITS;
	resources.MaxFragmentUniformVectors = sw::FRAGMENT_UNIFORM_VECTORS - 3;
	resources.MaxDrawBuffers = sw::RENDERTARGETS;
	resources.MaxVertexOutputVectors = 16;   // ???
	resources.MaxFragmentInputVectors = 15;  // ???
	resources.MinProgramTexelOffset = sw::MIN_PROGRAM_TEXEL_OFFSET;
	resources.MaxProgramTexelOffset = sw::MAX_PROGRAM_TEXEL_OFFSET;
	resources.OES_standard_derivatives = 1;
	resources.OES_fragment_precision_high = 1;
	resources.OES_EGL_image_external = 1;
	resources.OES_EGL_image_external_essl3 = 1;
	resources.EXT_draw_buffers = 1;
	resources.ARB_texture_rectangle = 1;
	resources.MaxCallStackDepth = 16;

	glslCompiler->Init(resources);

	const char *glslSource = reinterpret_cast<const char *>(data + kHeaderSize);
	if(!glslCompiler->compile(&glslSource, 1, SH_OBJECT_CODE))
	{
		return 0;
	}

	std::unique_ptr<sw::VertexShader> bytecodeShader(new sw::VertexShader(fakeVS->getVertexShader()));

	sw::VertexProcessor::State state;

	state.textureSampling = bytecodeShader->containsTextureSampling();
	state.positionRegister = bytecodeShader->getPositionRegister();
	state.pointSizeRegister = bytecodeShader->getPointSizeRegister();

	state.preTransformed = (data[0] & 0x01) != 0;
	state.superSampling = (data[0] & 0x02) != 0;

	state.transformFeedbackQueryEnabled = (data[0] & 0x08) != 0;
	state.transformFeedbackEnabled = (data[0] & 0x10) != 0;
	state.verticesPerPrimitive = 1 + ((data[0] & 0x20) != 0) + ((data[0] & 0x40) != 0);

	if((data[0] & 0x80) != 0)  // Unused/reserved.
	{
		return 0;
	}

	constexpr int MAX_ATTRIBUTE_COMPONENTS = 4;

	struct Stream
	{
		uint8_t count : BITS(MAX_ATTRIBUTE_COMPONENTS);
		bool normalized : 1;
		uint8_t reserved : 8 - BITS(MAX_ATTRIBUTE_COMPONENTS) - 1;
	};

	for(int i = 0; i < sw::MAX_VERTEX_INPUTS; i++)
	{
		sw::StreamType type = (sw::StreamType)data[1 + 2 * i + 0];
		Stream stream = (Stream &)data[1 + 2 * i + 1];

		if(type > sw::STREAMTYPE_LAST) return 0;
		if(stream.count > MAX_ATTRIBUTE_COMPONENTS) return 0;
		if(stream.reserved != 0) return 0;

		state.input[i].type = type;
		state.input[i].count = stream.count;
		state.input[i].normalized = stream.normalized;
		state.input[i].attribType = bytecodeShader->getAttribType(i);
	}

	for(unsigned int i = 0; i < sw::VERTEX_TEXTURE_IMAGE_UNITS; i++)
	{
		// TODO
		//	if(bytecodeShader->usesSampler(i))
		//	{
		//		state.samplerState[i] = context->sampler[sw::TEXTURE_IMAGE_UNITS + i].samplerState();
		//	}

		for(int j = 0; j < 32; j++)
		{
			if(data[1 + 2 * sw::MAX_VERTEX_INPUTS + 32 * i + j] != 0)
			{
				return 0;
			}
		}
	}

	for(int i = 0; i < sw::MAX_VERTEX_OUTPUTS; i++)
	{
		state.output[i].xWrite = bytecodeShader->getOutput(i, 0).active();
		state.output[i].yWrite = bytecodeShader->getOutput(i, 1).active();
		state.output[i].zWrite = bytecodeShader->getOutput(i, 2).active();
		state.output[i].wWrite = bytecodeShader->getOutput(i, 3).active();
	}

	sw::VertexProgram program(state, bytecodeShader.get());
	program.generate();

	auto routine = program("VertexRoutine");
	assert(routine);
	const void *entry = routine->getEntry();
	assert(entry);
	(void)entry;

	return 0;
}

#if defined(FUZZER_STANDALONE_REPRODUCE)
int main(int argc, char *argv[])
{
	FILE *file = fopen("clusterfuzz-testcase", "r");

	fseek(file, 0L, SEEK_END);
	long numbytes = ftell(file);
	fseek(file, 0L, SEEK_SET);
	uint8_t *buffer = (uint8_t *)calloc(numbytes, sizeof(uint8_t));
	fread(buffer, sizeof(char), numbytes, file);
	fclose(file);

	while(true)
	{
		LLVMFuzzerTestOneInput(buffer, numbytes);
	}

	free(buffer);

	return 0;
}
#endif
