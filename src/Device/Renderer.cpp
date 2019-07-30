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

#include "Renderer.hpp"

#include "Clipper.hpp"
#include "Primitive.hpp"
#include "Polygon.hpp"
#include "Reactor/Reactor.hpp"
#include "Pipeline/Constants.hpp"
#include "System/Memory.hpp"
#include "System/Half.hpp"
#include "System/Math.hpp"
#include "System/Timer.hpp"
#include "Vulkan/VkConfig.h"
#include "Vulkan/VkDebug.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkFence.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkQueryPool.hpp"
#include "Pipeline/SpirvShader.hpp"
#include "Vertex.hpp"

#include "Yarn/Defer.hpp"
#include "Yarn/Trace.hpp"
#include "Yarn/WaitGroup.hpp"

#undef max

#ifndef NDEBUG
unsigned int minPrimitives = 1;
unsigned int maxPrimitives = 1 << 21;
#endif

namespace sw
{
	template<typename T>
	inline bool setBatchIndices(unsigned int batch[128][3], VkPrimitiveTopology topology, T indices, unsigned int start, unsigned int triangleCount)
	{
		switch(topology)
		{
		case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		{
			auto index = start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index];
				batch[i][1] = indices[index];
				batch[i][2] = indices[index];

				index += 1;
			}
			break;
		}
		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		{
			auto index = 2 * start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + 0];
				batch[i][1] = indices[index + 1];
				batch[i][2] = indices[index + 1];

				index += 2;
			}
			break;
		}
		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		{
			auto index = start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + 0];
				batch[i][1] = indices[index + 1];
				batch[i][2] = indices[index + 1];

				index += 1;
			}
			break;
		}
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		{
			auto index = 3 * start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + 0];
				batch[i][1] = indices[index + 1];
				batch[i][2] = indices[index + 2];

				index += 3;
			}
			break;
		}
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		{
			auto index = start;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + 0];
				batch[i][1] = indices[index + ((start + i) & 1) + 1];
				batch[i][2] = indices[index + (~(start + i) & 1) + 1];

				index += 1;
			}
			break;
		}
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		{
			auto index = start + 1;
			for(unsigned int i = 0; i < triangleCount; i++)
			{
				batch[i][0] = indices[index + 0];
				batch[i][1] = indices[index + 1];
				batch[i][2] = indices[0];

				index += 1;
			}
			break;
		}
		default:
			ASSERT(false);
			return false;
		}

		return true;
	}

	DrawCall::DrawCall()
	{
		data = (DrawData*)allocate(sizeof(DrawData));
		data->constants = &constants;
	}

	DrawCall::~DrawCall()
	{
		deallocate(data);
	}

	Renderer::Renderer(vk::Device* device) : device(device)
	{
		VertexProcessor::setRoutineCacheSize(1024);
		PixelProcessor::setRoutineCacheSize(1024);
		SetupProcessor::setRoutineCacheSize(1024);
	}

	Renderer::~Renderer()
	{
		tickets.take().wait();
	}

	bool Renderer::hasQueryOfType(VkQueryType type) const
	{
		for(auto query : queries)
		{
			if(query->getType() == type)
			{
				return true;
			}
		}

		return false;
	}

	void Renderer::draw(const sw::Context* context, VkIndexType indexType, unsigned int count, int baseVertex, TaskEvents *events, bool update)
	{
		if(count == 0) { return; }

		auto id = nextDrawID++;
		SCOPED_EVENT("draw %d", id);

		#ifndef NDEBUG
		{
			unsigned int minPrimitives = 1;
			unsigned int maxPrimitives = 1 << 21;
			if(count < minPrimitives || count > maxPrimitives)
			{
				return;
			}
		}
		#endif

		int ms = context->sampleCount;

		if(!context->multiSampleMask)
		{
			return;
		}

		yarn::Pool<sw::DrawCall>::Loan draw;
		{
			SCOPED_EVENT("drawCallPool.borrow()");
			draw = drawCallPool.borrow();
		}
		draw->id = id;

		if(update)
		{
			SCOPED_EVENT("update");
			vertexState = VertexProcessor::update(context);
			setupState = SetupProcessor::update(context);
			pixelState = PixelProcessor::update(context);

			vertexRoutine = VertexProcessor::routine(vertexState, context->pipelineLayout, context->vertexShader, context->descriptorSets);
			setupRoutine = SetupProcessor::routine(setupState);
			pixelRoutine = PixelProcessor::routine(pixelState, context->pipelineLayout, context->pixelShader, context->descriptorSets);
		}

		DrawCall::SetupFunction setupPrimitives = nullptr;

		if(context->isDrawTriangle())
		{
			setupPrimitives = &DrawCall::setupTriangles;
		}
		else if(context->isDrawLine())
		{
			setupPrimitives = &DrawCall::setupLines;
		}
		else   // Point draw
		{
			setupPrimitives = &DrawCall::setupPoints;
		}

		DrawData *data = draw->data;

		if(queries.size() != 0)
		{
			for(auto &query : queries)
			{
				draw->queries.push_back(query);
			}
		}

		draw->batchDataPool = &batchDataPool;
		draw->numPrimitives = count;
		draw->numPrimitivesPerBatch = BatchSize / ms;
		draw->numBatches = (count + draw->numPrimitivesPerBatch - 1) / draw->numPrimitivesPerBatch;
		draw->numBatchWorkers = std::min<unsigned int>(draw->numBatches, DrawCall::BatchData::MaxInstances);
		draw->numBatchWorkers = std::min<unsigned int>(draw->numBatchWorkers, yarn::Scheduler::get()->getWorkerThreadCount());
		draw->batchTickets = tickets.takeN(draw->numBatches);
		draw->topology = context->topology;
		draw->indexType = indexType;

		draw->vertexRoutine = vertexRoutine;
		draw->setupRoutine = setupRoutine;
		draw->pixelRoutine = pixelRoutine;
		draw->vertexPointer = (VertexProcessor::RoutinePointer)vertexRoutine->getEntry();
		draw->setupPointer = (SetupProcessor::RoutinePointer)setupRoutine->getEntry();
		draw->pixelPointer = (PixelProcessor::RoutinePointer)pixelRoutine->getEntry();
		draw->setupPrimitives = setupPrimitives;
		draw->setupState = setupState;

		data->descriptorSets = context->descriptorSets;
		data->descriptorDynamicOffsets = context->descriptorDynamicOffsets;

		for(int i = 0; i < MAX_INTERFACE_COMPONENTS/4; i++)
		{
			data->input[i] = context->input[i].buffer;
			data->stride[i] = context->input[i].vertexStride;
		}

		data->indices = context->indexBuffer;

		if(context->vertexShader->hasBuiltinInput(spv::BuiltInInstanceIndex))
		{
			data->instanceID = context->instanceID;
		}

		data->baseVertex = baseVertex;

		if(pixelState.stencilActive)
		{
			data->stencil[0].set(context->frontStencil.reference, context->frontStencil.compareMask, context->frontStencil.writeMask);
			data->stencil[1].set(context->backStencil.reference, context->backStencil.compareMask, context->backStencil.writeMask);
		}

		data->lineWidth = context->lineWidth;

		data->factor = factor;

		if(pixelState.alphaToCoverage)
		{
			if(ms == 4)
			{
				data->a2c0 = replicate(0.2f);
				data->a2c1 = replicate(0.4f);
				data->a2c2 = replicate(0.6f);
				data->a2c3 = replicate(0.8f);
			}
			else if(ms == 2)
			{
				data->a2c0 = replicate(0.25f);
				data->a2c1 = replicate(0.75f);
			}
			else ASSERT(false);
		}

		if(pixelState.occlusionEnabled)
		{
			for(int cluster = 0; cluster < ClusterCount; cluster++)
			{
				data->occlusion[cluster] = 0;
			}
		}

		// Viewport
		{
			float W = 0.5f * viewport.width;
			float H = 0.5f * viewport.height;
			float X0 = viewport.x + W;
			float Y0 = viewport.y + H;
			float N = viewport.minDepth;
			float F = viewport.maxDepth;
			float Z = F - N;

			if(context->isDrawTriangle())
			{
				N += context->depthBias;
			}

			data->Wx16 = replicate(W * 16);
			data->Hx16 = replicate(H * 16);
			data->X0x16 = replicate(X0 * 16 - 8);
			data->Y0x16 = replicate(Y0 * 16 - 8);
			data->halfPixelX = replicate(0.5f / W);
			data->halfPixelY = replicate(0.5f / H);
			data->viewportHeight = abs(viewport.height);
			data->slopeDepthBias = context->slopeDepthBias;
			data->depthRange = Z;
			data->depthNear = N;
		}

		// Target
		{
			for(int index = 0; index < RENDERTARGETS; index++)
			{
				draw->renderTarget[index] = context->renderTarget[index];

				if(draw->renderTarget[index])
				{
					data->colorBuffer[index] = (unsigned int*)context->renderTarget[index]->getOffsetPointer({0, 0, 0}, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
					data->colorPitchB[index] = context->renderTarget[index]->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
					data->colorSliceB[index] = context->renderTarget[index]->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
				}
			}

			draw->depthBuffer = context->depthBuffer;
			draw->stencilBuffer = context->stencilBuffer;

			if(draw->depthBuffer)
			{
				data->depthBuffer = (float*)context->depthBuffer->getOffsetPointer({0, 0, 0}, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0);
				data->depthPitchB = context->depthBuffer->rowPitchBytes(VK_IMAGE_ASPECT_DEPTH_BIT, 0);
				data->depthSliceB = context->depthBuffer->slicePitchBytes(VK_IMAGE_ASPECT_DEPTH_BIT, 0);
			}

			if(draw->stencilBuffer)
			{
				data->stencilBuffer = (unsigned char*)context->stencilBuffer->getOffsetPointer({0, 0, 0}, VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0);
				data->stencilPitchB = context->stencilBuffer->rowPitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
				data->stencilSliceB = context->stencilBuffer->slicePitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
			}
		}

		// Scissor
		{
			data->scissorX0 = scissor.offset.x;
			data->scissorX1 = scissor.offset.x + scissor.extent.width;
			data->scissorY0 = scissor.offset.y;
			data->scissorY1 = scissor.offset.y + scissor.extent.height;
		}

		// Push constants
		{
			data->pushConstants = context->pushConstants;
		}

		draw->events = events;

		DrawCall::run(draw);
	}

	void DrawCall::setup()
	{
		for(auto &query : queries)
		{
			query->start();
		}

		if(events)
		{
			events->start();
		}
	}

	void DrawCall::teardown()
	{
		if(events)
		{
			events->finish();
			events = nullptr;
		}

		for(auto &query : queries)
		{
			switch(query->getType())
			{
			case VK_QUERY_TYPE_OCCLUSION:
				for(int cluster = 0; cluster < ClusterCount; cluster++)
				{
					query->add(data->occlusion[cluster]);
				}
				break;
			default:
				break;
			}

			query->finish();
		}
		queries.clear();

		vertexRoutine.reset();
		setupRoutine.reset();
		pixelRoutine.reset();
	}

	void DrawCall::run(const yarn::Loan<DrawCall>& draw)
	{
		draw->setup();

		auto finally = yarn::make_shared_finally([draw]
		{
			SCOPED_EVENT("draw %d -- FINISH", draw->id);
			draw->teardown();
		});

		for (unsigned int batchWorker = 0; batchWorker < draw->numBatchWorkers; batchWorker++)
		{
			yarn::Pool<sw::DrawCall::BatchData>::Loan batch;

			{
				SCOPED_EVENT("batchDataPool->borrow()");
				batch = draw->batchDataPool->borrow();
				batch->draw = draw;
			}

			yarn::schedule([batchWorker, batch, finally] {
				defer(batch->draw.reset());

				auto const draw = batch->draw.get();
				auto const numPrimitives = draw->numPrimitives;
				auto const numPrimitivesPerBatch = draw->numPrimitivesPerBatch;
				auto const numBatches = draw->numBatches;
				auto const numBatchWorkers = draw->numBatchWorkers;

				for (auto id = batchWorker; id < numBatches; id += numBatchWorkers)
				{
					batch->id = id;
					batch->firstPrimitive = batch->id * numPrimitivesPerBatch;
					batch->numPrimitives = std::min(batch->firstPrimitive + numPrimitivesPerBatch, numPrimitives) - batch->firstPrimitive;

					auto ticket = std::move(draw->batchTickets[id]);

					processVertices(batch.get());

					if (draw->setupState.rasterizerDiscard)
					{
						continue;
					}

					processPrimitives(batch.get());

					if (batch->numVisible <= 0)
					{
						continue;
					}

					{
						SCOPED_EVENT("BARRIER");
						ticket.wait();
					}

					processPixels(batch.get());
				}
			});
		}
	}

	void DrawCall::processVertices(BatchData* batch)
	{
		auto draw = batch->draw.get();
		SCOPED_EVENT("draw %d, batch %d", draw->id, batch);

		unsigned int triangleIndices[BatchSize + 1][3];  // One extra for SIMD width overrun. TODO: Adjust to dynamic batch size.
		{
			SCOPED_EVENT("processPrimitiveVertices");
			processPrimitiveVertices(
				triangleIndices,
				draw->data->indices,
				draw->indexType,
				batch->firstPrimitive,
				batch->numPrimitives,
				draw->topology);
		}

		auto& vertexTask = batch->vertexTask;
		vertexTask.primitiveStart = batch->firstPrimitive;
		vertexTask.vertexCount = batch->numPrimitives * 3;
		if (vertexTask.vertexCache.drawCall != draw->id)
		{
			vertexTask.vertexCache.clear();
			vertexTask.vertexCache.drawCall = draw->id;
		}

		{
			SCOPED_EVENT("VERTEX");
			draw->vertexPointer(&batch->triangles.front().v0, &triangleIndices[0][0], &vertexTask, draw->data);
		}
	}

	void DrawCall::processPrimitives(BatchData* batch)
	{
		SCOPED_EVENT("PRIMITIVES");
		auto draw = batch->draw.get();
		batch->numVisible = draw->setupPrimitives(&batch->triangles, &batch->primitives, draw, batch->numPrimitives);
	}

	void DrawCall::processPixels(BatchData* batch)
	{
		SCOPED_EVENT("PIXEL");
		auto draw = batch->draw.get();
		if (draw->topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
		{
			yarn::WaitGroup wg(ClusterCount);
			for (int cluster = 0; cluster < ClusterCount; cluster++)
			{
				yarn::schedule([&, cluster] {
					defer(wg.done());
					auto draw = batch->draw.get();
					SCOPED_EVENT("draw %d, batch %d, cluster %d", draw->id, batch->id, cluster);
					draw->pixelPointer(&batch->primitives.front(), batch->numVisible, cluster, draw->data);
				});
			}
			SCOPED_EVENT("WAIT CLUSTER");
			wg.wait();
		}
		else
		{
			for (int cluster = 0; cluster < ClusterCount; cluster++)
			{
				draw->pixelPointer(&batch->primitives.front(), batch->numVisible, cluster, draw->data);
			}
		}
	}

	void Renderer::synchronize()
	{
		SCOPED_EVENT("synchronize");
		tickets.take().wait();
		device->updateSamplingRoutineConstCache();
	}

	void DrawCall::processPrimitiveVertices(
		unsigned int triangleIndicesOut[BatchSize + 1][3],
		const void *primitiveIndices,
		VkIndexType indexType,
		unsigned int start,
		unsigned int triangleCount,
		VkPrimitiveTopology topology)
	{
		if(!primitiveIndices)
		{
			struct LinearIndex
			{
				unsigned int operator[](unsigned int i) { return i; }
			};

			if(!setBatchIndices(triangleIndicesOut, topology, LinearIndex(), start, triangleCount))
			{
				return;
			}
		}
		else
		{
			switch(indexType)
			{
			case VK_INDEX_TYPE_UINT16:
				if(!setBatchIndices(triangleIndicesOut, topology, static_cast<const uint16_t*>(primitiveIndices), start, triangleCount))
				{
					return;
				}
				break;
			case VK_INDEX_TYPE_UINT32:
				if(!setBatchIndices(triangleIndicesOut, topology, static_cast<const uint32_t*>(primitiveIndices), start, triangleCount))
				{
					return;
				}
				break;
			break;
			default:
				ASSERT(false);
				return;
			}
		}

		// Repeat the last index to allow for SIMD width overrun.
		triangleIndicesOut[triangleCount][0] = triangleIndicesOut[triangleCount - 1][2];
		triangleIndicesOut[triangleCount][1] = triangleIndicesOut[triangleCount - 1][2];
		triangleIndicesOut[triangleCount][2] = triangleIndicesOut[triangleCount - 1][2];
	}

	int DrawCall::setupTriangles(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count)
	{
		Triangle *triangle = &triangleBatch->front();
		Primitive *primitive = &primitiveBatch->front();
		auto &state = drawCall->setupState;
		auto setupRoutine = drawCall->setupPointer;

		int ms = state.multiSample;
		const DrawData *data = drawCall->data;
		int visible = 0;

		for(int i = 0; i < count; i++, triangle++)
		{
			Vertex &v0 = triangle->v0;
			Vertex &v1 = triangle->v1;
			Vertex &v2 = triangle->v2;

			if((v0.clipFlags & v1.clipFlags & v2.clipFlags) == Clipper::CLIP_FINITE)
			{
				Polygon polygon(&v0.position, &v1.position, &v2.position);

				int clipFlagsOr = v0.clipFlags | v1.clipFlags | v2.clipFlags;

				if(clipFlagsOr != Clipper::CLIP_FINITE)
				{
					if(!Clipper::Clip(polygon, clipFlagsOr, *drawCall))
					{
						continue;
					}
				}

				if(setupRoutine(primitive, triangle, &polygon, data))
				{
					primitive += ms;
					visible++;
				}
			}
		}

		return visible;
	}

	int DrawCall::setupLines(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count)
	{
		Triangle *triangle = &triangleBatch->front();
		Primitive *primitive = &primitiveBatch->front();
		auto &state = drawCall->setupState;

		int visible = 0;
		int ms = state.multiSample;

		for(int i = 0; i < count; i++)
		{
			if(setupLine(*primitive, *triangle, *drawCall))
			{
				primitive += ms;
				visible++;
			}

			triangle++;
		}

		return visible;
	}

	int DrawCall::setupPoints(TriangleBatch *triangleBatch, PrimitiveBatch *primitiveBatch, const DrawCall *drawCall, int count)
	{
		Triangle *triangle = &triangleBatch->front();
		Primitive *primitive = &primitiveBatch->front();
		auto &state = drawCall->setupState;

		int visible = 0;
		int ms = state.multiSample;

		for(int i = 0; i < count; i++)
		{
			if(setupPoint(*primitive, *triangle, *drawCall))
			{
				primitive += ms;
				visible++;
			}

			triangle++;
		}

		return visible;
	}

	bool DrawCall::setupLine(Primitive &primitive, Triangle &triangle, const DrawCall &draw)
	{
		const SetupProcessor::RoutinePointer &setupRoutine = draw.setupPointer;
		const DrawData &data = *draw.data;
		auto &state = draw.setupState;

		float lineWidth = data.lineWidth;

		Vertex &v0 = triangle.v0;
		Vertex &v1 = triangle.v1;

		const float4 &P0 = v0.position;
		const float4 &P1 = v1.position;

		if(P0.w <= 0 && P1.w <= 0)
		{
			return false;
		}

		const float W = data.Wx16[0] * (1.0f / 16.0f);
		const float H = data.Hx16[0] * (1.0f / 16.0f);

		float dx = W * (P1.x / P1.w - P0.x / P0.w);
		float dy = H * (P1.y / P1.w - P0.y / P0.w);

		if(dx == 0 && dy == 0)
		{
			return false;
		}

		if(state.multiSample > 1)   // Rectangle
		{
			float4 P[4];
			int C[4];

			P[0] = P0;
			P[1] = P1;
			P[2] = P1;
			P[3] = P0;

			float scale = lineWidth * 0.5f / sqrt(dx*dx + dy*dy);

			dx *= scale;
			dy *= scale;

			float dx0h = dx * P0.w / H;
			float dy0w = dy * P0.w / W;

			float dx1h = dx * P1.w / H;
			float dy1w = dy * P1.w / W;

			P[0].x += -dy0w;
			P[0].y += +dx0h;
			C[0] = Clipper::ComputeClipFlags(P[0]);

			P[1].x += -dy1w;
			P[1].y += +dx1h;
			C[1] = Clipper::ComputeClipFlags(P[1]);

			P[2].x += +dy1w;
			P[2].y += -dx1h;
			C[2] = Clipper::ComputeClipFlags(P[2]);

			P[3].x += +dy0w;
			P[3].y += -dx0h;
			C[3] = Clipper::ComputeClipFlags(P[3]);

			if((C[0] & C[1] & C[2] & C[3]) == Clipper::CLIP_FINITE)
			{
				Polygon polygon(P, 4);

				int clipFlagsOr = C[0] | C[1] | C[2] | C[3];

				if(clipFlagsOr != Clipper::CLIP_FINITE)
				{
					if(!Clipper::Clip(polygon, clipFlagsOr, draw))
					{
						return false;
					}
				}

				return setupRoutine(&primitive, &triangle, &polygon, &data);
			}
		}
		else   // Diamond test convention
		{
			float4 P[8];
			int C[8];

			P[0] = P0;
			P[1] = P0;
			P[2] = P0;
			P[3] = P0;
			P[4] = P1;
			P[5] = P1;
			P[6] = P1;
			P[7] = P1;

			float dx0 = lineWidth * 0.5f * P0.w / W;
			float dy0 = lineWidth * 0.5f * P0.w / H;

			float dx1 = lineWidth * 0.5f * P1.w / W;
			float dy1 = lineWidth * 0.5f * P1.w / H;

			P[0].x += -dx0;
			C[0] = Clipper::ComputeClipFlags(P[0]);

			P[1].y += +dy0;
			C[1] = Clipper::ComputeClipFlags(P[1]);

			P[2].x += +dx0;
			C[2] = Clipper::ComputeClipFlags(P[2]);

			P[3].y += -dy0;
			C[3] = Clipper::ComputeClipFlags(P[3]);

			P[4].x += -dx1;
			C[4] = Clipper::ComputeClipFlags(P[4]);

			P[5].y += +dy1;
			C[5] = Clipper::ComputeClipFlags(P[5]);

			P[6].x += +dx1;
			C[6] = Clipper::ComputeClipFlags(P[6]);

			P[7].y += -dy1;
			C[7] = Clipper::ComputeClipFlags(P[7]);

			if((C[0] & C[1] & C[2] & C[3] & C[4] & C[5] & C[6] & C[7]) == Clipper::CLIP_FINITE)
			{
				float4 L[6];

				if(dx > -dy)
				{
					if(dx > dy)   // Right
					{
						L[0] = P[0];
						L[1] = P[1];
						L[2] = P[5];
						L[3] = P[6];
						L[4] = P[7];
						L[5] = P[3];
					}
					else   // Down
					{
						L[0] = P[0];
						L[1] = P[4];
						L[2] = P[5];
						L[3] = P[6];
						L[4] = P[2];
						L[5] = P[3];
					}
				}
				else
				{
					if(dx > dy)   // Up
					{
						L[0] = P[0];
						L[1] = P[1];
						L[2] = P[2];
						L[3] = P[6];
						L[4] = P[7];
						L[5] = P[4];
					}
					else   // Left
					{
						L[0] = P[1];
						L[1] = P[2];
						L[2] = P[3];
						L[3] = P[7];
						L[4] = P[4];
						L[5] = P[5];
					}
				}

				Polygon polygon(L, 6);

				int clipFlagsOr = C[0] | C[1] | C[2] | C[3] | C[4] | C[5] | C[6] | C[7];

				if(clipFlagsOr != Clipper::CLIP_FINITE)
				{
					if(!Clipper::Clip(polygon, clipFlagsOr, draw))
					{
						return false;
					}
				}

				return setupRoutine(&primitive, &triangle, &polygon, &data);
			}
		}

		return false;
	}

	bool DrawCall::setupPoint(Primitive &primitive, Triangle &triangle, const DrawCall &draw)
	{
		const SetupProcessor::RoutinePointer &setupRoutine = draw.setupPointer;
		const DrawData &data = *draw.data;

		Vertex &v = triangle.v0;

		float pSize = v.pointSize;

		pSize = clamp(pSize, 1.0f, static_cast<float>(vk::MAX_POINT_SIZE));

		float4 P[4];
		int C[4];

		P[0] = v.position;
		P[1] = v.position;
		P[2] = v.position;
		P[3] = v.position;

		const float X = pSize * P[0].w * data.halfPixelX[0];
		const float Y = pSize * P[0].w * data.halfPixelY[0];

		P[0].x -= X;
		P[0].y += Y;
		C[0] = Clipper::ComputeClipFlags(P[0]);

		P[1].x += X;
		P[1].y += Y;
		C[1] = Clipper::ComputeClipFlags(P[1]);

		P[2].x += X;
		P[2].y -= Y;
		C[2] = Clipper::ComputeClipFlags(P[2]);

		P[3].x -= X;
		P[3].y -= Y;
		C[3] = Clipper::ComputeClipFlags(P[3]);

		Polygon polygon(P, 4);

		if((C[0] & C[1] & C[2] & C[3]) == Clipper::CLIP_FINITE)
		{
			int clipFlagsOr = C[0] | C[1] | C[2] | C[3];

			if(clipFlagsOr != Clipper::CLIP_FINITE)
			{
				if(!Clipper::Clip(polygon, clipFlagsOr, draw))
				{
					return false;
				}
			}

			triangle.v1 = triangle.v0;
			triangle.v2 = triangle.v0;

			triangle.v1.projected.x += iround(16 * 0.5f * pSize);
			triangle.v2.projected.y -= iround(16 * 0.5f * pSize) * (data.Hx16[0] > 0.0f ? 1 : -1);   // Both Direct3D and OpenGL expect (0, 0) in the top-left corner
			return setupRoutine(&primitive, &triangle, &polygon, &data);
		}

		return false;
	}

	void Renderer::addQuery(vk::Query *query)
	{
		queries.push_back(query);
	}

	void Renderer::removeQuery(vk::Query *query)
	{
		queries.remove(query);
	}

	void Renderer::advanceInstanceAttributes(Stream* inputs)
	{
		for(uint32_t i = 0; i < vk::MAX_VERTEX_INPUT_BINDINGS; i++)
		{
			auto &attrib = inputs[i];
			if (attrib.count && attrib.instanceStride)
			{
				// Under the casts: attrib.buffer += attrib.instanceStride
				attrib.buffer = (void const *)((uintptr_t)attrib.buffer + attrib.instanceStride);
			}
		}
	}

	void Renderer::setViewport(const VkViewport &viewport)
	{
		this->viewport = viewport;
	}

	void Renderer::setScissor(const VkRect2D &scissor)
	{
		this->scissor = scissor;
	}

}
