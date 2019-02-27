// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include <spirv/unified1/spirv.hpp>
#include "SpirvShader.hpp"
#include "SpirvShader.inl"
#include "System/Math.hpp"
#include "Vulkan/VkDebug.hpp"
#include "Device/Config.hpp"

namespace sw
{

void SpirvShader::EmitPass::OpVariable(InsnIterator insn)
{
    ObjectID resultId = insn.word(2);
    auto &object = getObject(resultId);
    auto &objectTy = getType(object.type);
    if (object.kind == Object::Kind::InterfaceVariable && objectTy.storageClass == spv::StorageClassInput)
    {
        auto &dst = routine->getValue(resultId);
        int offset = 0;
        shader->VisitInterface(resultId,
                        [&](Decorations const &d, AttribType type) {
                            auto scalarSlot = d.Location << 2 | d.Component;
                            dst[offset++] = routine->inputs[scalarSlot];
                        });
    }
}

void SpirvShader::EmitPass::OpLoad(InsnIterator insn)
{
    ObjectID objectId = insn.word(2);
    ObjectID pointerId = insn.word(3);
    auto &object = getObject(objectId);
    auto &objectTy = getType(object.type);
    auto &pointer = getObject(pointerId);
    auto &pointerTy = getType(pointer.type);
    routine->createIntermediate(objectId, objectTy.sizeInComponents);
    auto &pointerBase = getObject(pointer.pointerBase);
    auto &pointerBaseTy = getType(pointerBase.type);

    assert(pointerTy.element == object.type);
    assert(TypeID(insn.word(1)) == object.type);

    if (pointerBaseTy.storageClass == spv::StorageClassImage ||
        pointerBaseTy.storageClass == spv::StorageClassUniform ||
        pointerBaseTy.storageClass == spv::StorageClassUniformConstant)
    {
        UNIMPLEMENTED("Descriptor-backed load not yet implemented");
    }

    auto &ptrBase = routine->getValue(pointer.pointerBase);
    auto &dst = routine->getIntermediate(objectId);

    if (pointer.kind == Object::Kind::Value)
    {
        auto offsets = As<IntL>(routine->getIntermediate(insn.word(3))[0]);
        for (auto i = 0u; i < objectTy.sizeInComponents; i++)
        {
            // i wish i had a Float,Float,Float,Float constructor here..
            FloatL v;
            for (int j = 0; j < NumLanes; j++)
            {
                Int offset = Int(i) + Extract(offsets, j);
                v = Insert(v, Extract(ptrBase[offset], j), j);
            }
            dst.emplace(i, v);
        }
    }
    else
    {
        // no divergent offsets to worry about
        for (auto i = 0u; i < objectTy.sizeInComponents; i++)
        {
            dst.emplace(i, ptrBase[i]);
        }
    }
}

void SpirvShader::EmitPass::OpStore(InsnIterator insn)
{
    ObjectID pointerId = insn.word(1);
    ObjectID objectId = insn.word(2);
    auto &object = getObject(objectId);
    auto &pointer = getObject(pointerId);
    auto &pointerTy = getType(pointer.type);
    auto &elementTy = getType(pointerTy.element);
    auto &pointerBase = getObject(pointer.pointerBase);
    auto &pointerBaseTy = getType(pointerBase.type);

    if (pointerBaseTy.storageClass == spv::StorageClassImage ||
        pointerBaseTy.storageClass == spv::StorageClassUniform ||
        pointerBaseTy.storageClass == spv::StorageClassUniformConstant)
    {
        UNIMPLEMENTED("Descriptor-backed store not yet implemented");
    }

    auto &ptrBase = routine->getValue(pointer.pointerBase);

    if (object.kind == Object::Kind::Constant)
    {
        auto src = reinterpret_cast<float *>(object.constantValue.get());

        if (pointer.kind == Object::Kind::Value)
        {
            auto offsets = As<IntL>(routine->getIntermediate(pointerId)[0]);
            for (auto i = 0u; i < elementTy.sizeInComponents; i++)
            {
                // Scattered store
                for (int j = 0; j < NumLanes; j++)
                {
                    auto dst = ptrBase[Int(i) + Extract(offsets, j)];
                    dst = Insert(dst, Float(src[i]), j);
                }
            }
        }
        else
        {
            // no divergent offsets
            for (auto i = 0u; i < elementTy.sizeInComponents; i++)
            {
                ptrBase[i] = RValue<FloatL>(src[i]);
            }
        }
    }
    else
    {
        auto &src = routine->getIntermediate(objectId);

        if (pointer.kind == Object::Kind::Value)
        {
            auto offsets = As<IntL>(routine->getIntermediate(pointerId)[0]);
            for (auto i = 0u; i < elementTy.sizeInComponents; i++)
            {
                // Scattered store
                for (int j = 0; j < NumLanes; j++)
                {
                    auto dst = ptrBase[Int(i) + Extract(offsets, j)];
                    dst = Insert(dst, Extract(src[i], j), j);
                }
            }
        }
        else
        {
            // no divergent offsets
            for (auto i = 0u; i < elementTy.sizeInComponents; i++)
            {
                ptrBase[i] = src[i];
            }
        }
    }
}

void SpirvShader::EmitPass::OpAccessChain(InsnIterator insn)
{
    TypeID typeId = insn.word(1);
    ObjectID objectId = insn.word(2);
    ObjectID baseId = insn.word(3);
    auto &object = getObject(objectId);
    auto &type = getType(typeId);
    auto &base = getObject(baseId);
    routine->createIntermediate(objectId, type.sizeInComponents);
    auto &pointerBase = getObject(object.pointerBase);
    auto &pointerBaseTy = getType(pointerBase.type);
    assert(type.sizeInComponents == 1);
    assert(base.pointerBase == object.pointerBase);

    if (pointerBaseTy.storageClass == spv::StorageClassImage ||
        pointerBaseTy.storageClass == spv::StorageClassUniform ||
        pointerBaseTy.storageClass == spv::StorageClassUniformConstant)
    {
        UNIMPLEMENTED("Descriptor-backed OpAccessChain not yet implemented");
    }
    auto &dst = routine->getIntermediate(objectId);
    dst.emplace(0, As<FloatL>(shader->WalkAccessChain(baseId, insn.wordCount() - 4, insn.wordPointer(4), routine)));
}

void SpirvShader::emit(SpirvRoutine *routine) const
{
    EmitPass(this, routine).run();
}

} // namespace sw
