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

#ifndef sw_Renderer_hpp
#define sw_Renderer_hpp

#include "VertexProcessor.hpp"
#include "PixelProcessor.hpp"
#include "SetupProcessor.hpp"
#include "Plane.hpp"
#include "Primitive.hpp"
#include "Blitter.hpp"
#include "Device/Config.hpp"
#include "Vulkan/VkDescriptorSet.hpp"

#include "Yarn/Pool.hpp"
#include "Yarn/TicketQueue.hpp"

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

namespace vk
{
	class DescriptorSet;
	class Device;
	class Query;
}

namespace sw
{
	struct DrawCall;
	class PixelShader;
	class VertexShader;
	struct Task;
	class TaskEvents;
	class Resource;
	class Renderer;
	struct Constants;

	static const int BatchSize = 128;
	static const int ClusterCount = 16;
	using TriangleBatch = std::array<Triangle, BatchSize>;
	using PrimitiveBatch = std::array<Primitive, BatchSize>;

	struct DrawData
	{
		const Constants *constants;

		vk::DescriptorSet::Bindings descriptorSets = {};
		vk::DescriptorSet::DynamicOffsets descriptorDynamicOffsets = {};

		const void *input[MAX_INTERFACE_COMPONENTS / 4];
		unsigned int stride[MAX_INTERFACE_COMPONENTS / 4];
		const void *indices;

		int instanceID;
		int baseVertex;
		float lineWidth;

		PixelProcessor::Stencil stencil[2];   // clockwise, counterclockwise
		PixelProcessor::Factor factor;
		unsigned int occlusion[ClusterCount];   // Number of pixels passing depth test

		float4 Wx16;
		float4 Hx16;
		float4 X0x16;
		float4 Y0x16;
		float4 halfPixelX;
		float4 halfPixelY;
		float viewportHeight;
		float slopeDepthBias;
		float depthRange;
		float depthNear;

		unsigned int *colorBuffer[RENDERTARGETS];
		int colorPitchB[RENDERTARGETS];
		int colorSliceB[RENDERTARGETS];
		float *depthBuffer;
		int depthPitchB;
		int depthSliceB;
		unsigned char *stencilBuffer;
		int stencilPitchB;
		int stencilSliceB;

		int scissorX0;
		int scissorX1;
		int scissorY0;
		int scissorY1;

		float4 a2c0;
		float4 a2c1;
		float4 a2c2;
		float4 a2c3;

		PushConstantStorage pushConstants;
	};

	struct DrawCall
	{
		struct BatchData
		{
			static constexpr int MaxInstances = 8;
			using Pool = yarn::FixedSizePool<BatchData, MaxInstances, yarn::PoolPolicy::Preserve>;

			yarn::Loan<DrawCall> draw;
			TriangleBatch triangles;
			PrimitiveBatch primitives;
			VertexTask vertexTask;
			unsigned int id;
			unsigned int firstPrimitive;
			unsigned int numPrimitives;
			int numVisible;
		};

		static constexpr int MaxInstances = 16;
		using Pool = yarn::FixedSizePool<DrawCall, MaxInstances, yarn::PoolPolicy::Preserve>;
		using SetupFunction = int(*)(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count);

		DrawCall();
		~DrawCall();

		static void run(const yarn::Loan<DrawCall>& draw);
		static void processVertices(BatchData* batch);
		static void processPrimitives(BatchData* batch);
		static void processPixels(BatchData* batch);
		void setup();
		void teardown();

		int id;

		BatchData::Pool *batchDataPool;
		unsigned int numPrimitives;
		unsigned int numPrimitivesPerBatch;
		unsigned int numBatches;
		unsigned int numBatchWorkers;
		std::vector<yarn::Ticket> batchTickets;

		VkPrimitiveTopology topology;
		VkIndexType indexType;

		std::shared_ptr<Routine> vertexRoutine;
		std::shared_ptr<Routine> setupRoutine;
		std::shared_ptr<Routine> pixelRoutine;

		VertexProcessor::RoutinePointer vertexPointer;
		SetupProcessor::RoutinePointer setupPointer;
		PixelProcessor::RoutinePointer pixelPointer;

		SetupFunction setupPrimitives;
		SetupProcessor::State setupState;

		vk::ImageView *renderTarget[RENDERTARGETS];
		vk::ImageView *depthBuffer;
		vk::ImageView *stencilBuffer;
		TaskEvents *events;

		std::list<vk::Query*> queries;

		DrawData *data;

		static void processPrimitiveVertices(
				unsigned int triangleIndicesOut[BatchSize + 1][3],
				const void *primitiveIndices,
				VkIndexType indexType,
				unsigned int start,
				unsigned int triangleCount,
				VkPrimitiveTopology topology);

		static int setupTriangles(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count);
		static int setupLines(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count);
		static int setupPoints(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count);

		static bool setupLine(Primitive &primitive, Triangle &triangle, const DrawCall &draw);
		static bool setupPoint(Primitive &primitive, Triangle &triangle, const DrawCall &draw);
	};

	class Renderer : public VertexProcessor, public PixelProcessor, public SetupProcessor
	{
	public:
		Renderer(vk::Device* device);

		virtual ~Renderer();

		bool hasQueryOfType(VkQueryType type) const;

		void draw(const sw::Context* context, VkIndexType indexType, unsigned int count, int baseVertex, TaskEvents *events, bool update = true);

		// Viewport & Clipper
		void setViewport(const VkViewport &viewport);
		void setScissor(const VkRect2D &scissor);

		void addQuery(vk::Query *query);
		void removeQuery(vk::Query *query);

		void advanceInstanceAttributes(Stream* inputs);

		void synchronize();

	private:
		VkViewport viewport;
		VkRect2D scissor;

		DrawCall::Pool drawCallPool;
		DrawCall::BatchData::Pool batchDataPool;

		std::atomic<int> nextDrawID = {0};

		std::list<vk::Query*> queries;
		yarn::Ticket::Queue tickets;

		VertexProcessor::State vertexState;
		SetupProcessor::State setupState;
		PixelProcessor::State pixelState;

		std::shared_ptr<Routine> vertexRoutine;
		std::shared_ptr<Routine> setupRoutine;
		std::shared_ptr<Routine> pixelRoutine;

		vk::Device* device;
	};

}

#endif   // sw_Renderer_hpp
