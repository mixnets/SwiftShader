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

void SpirvShader::AnalysisPass::OpExecutionMode(InsnIterator insn)
{
    shader->ProcessExecutionMode(insn);
}

void SpirvShader::AnalysisPass::OpDecorate(InsnIterator insn)
{
    TypeOrObjectID targetId = insn.word(1);
    auto decoration = static_cast<spv::Decoration>(insn.word(2));
    shader->decorations[targetId].Apply(
            decoration,
            insn.wordCount() > 3 ? insn.word(3) : 0);

    if (decoration == spv::DecorationCentroid)
    {
        shader->modes.NeedsCentroid = true;
    }
}

void SpirvShader::AnalysisPass::OpMemberDecorate(InsnIterator insn)
{
    TypeID targetId = insn.word(1);
    auto memberIndex = insn.word(2);
    auto &d = shader->memberDecorations[targetId];
    if (memberIndex >= d.size())
        d.resize(memberIndex + 1);    // on demand; exact size would require another pass...
    auto decoration = static_cast<spv::Decoration>(insn.word(3));
    d[memberIndex].Apply(
            decoration,
            insn.wordCount() > 4 ? insn.word(4) : 0);

    if (decoration == spv::DecorationCentroid)
    {
        shader->modes.NeedsCentroid = true;
    }
}

void SpirvShader::AnalysisPass::OpDecorationGroup(InsnIterator insn)
{
    // Nothing to do here. We don't need to record the definition of the group; we'll just have
    // the bundle of decorations float around. If we were to ever walk the decorations directly,
    // we might think about introducing this as a real Object.
}

void SpirvShader::AnalysisPass::OpGroupDecorate(InsnIterator insn)
{
    auto const &srcDecorations = shader->decorations[insn.word(1)];
    for (auto i = 2u; i < insn.wordCount(); i++)
    {
        // remaining operands are targets to apply the group to.
        shader->decorations[insn.word(i)].Apply(srcDecorations);
    }
}

void SpirvShader::AnalysisPass::OpGroupMemberDecorate(InsnIterator insn)
{
    auto const &srcDecorations = shader->decorations[insn.word(1)];
    for (auto i = 2u; i < insn.wordCount(); i += 2)
    {
        // remaining operands are pairs of <id>, literal for members to apply to.
        auto &d = shader->memberDecorations[insn.word(i)];
        auto memberIndex = insn.word(i + 1);
        if (memberIndex >= d.size())
            d.resize(memberIndex + 1);    // on demand resize, see above...
        d[memberIndex].Apply(srcDecorations);
    }
}

void SpirvShader::AnalysisPass::OpTypeVoid(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeBool(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeInt(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeFloat(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeVector(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeMatrix(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeImage(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeSampler(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeSampledImage(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeArray(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeRuntimeArray(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeStruct(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypePointer(InsnIterator insn) { DeclareType(insn); }
void SpirvShader::AnalysisPass::OpTypeFunction(InsnIterator insn) { DeclareType(insn); }

void SpirvShader::AnalysisPass::OpVariable(InsnIterator insn)
{
    TypeID typeId = insn.word(1);
    ObjectID resultId = insn.word(2);
    auto storageClass = static_cast<spv::StorageClass>(insn.word(3));
    if (insn.wordCount() > 4)
        UNIMPLEMENTED("Variable initializers not yet supported");

    auto &object = shader->defs[resultId];
    object.kind = Object::Kind::Variable;
    object.definition = insn;
    object.type = typeId;
    object.pointerBase = insn.word(2);	// base is itself

    // Register builtins
    switch (storageClass)
    {
        case spv::StorageClassInput:
        case spv::StorageClassOutput:
            shader->ProcessInterfaceVariable(object);
            break;
        default:
            UNIMPLEMENTED("Unhandled storage class %d for OpVariable", (int)storageClass);
    }
}

void SpirvShader::AnalysisPass::OpConstant(InsnIterator insn)
{
    shader->CreateConstant(insn).constantValue[0] = insn.word(3);
}

void SpirvShader::AnalysisPass::OpConstantFalse(InsnIterator insn)
{
    shader->CreateConstant(insn).constantValue[0] = 0;		// represent boolean false as zero
}

void SpirvShader::AnalysisPass::OpConstantTrue(InsnIterator insn)
{
    shader->CreateConstant(insn).constantValue[0] = ~0u;	// represent boolean true as all bits set
}

void SpirvShader::AnalysisPass::OpConstantNull(InsnIterator insn)
{
    // OpConstantNull forms a constant of arbitrary type, all zeros.
    auto &object = shader->CreateConstant(insn);
    auto &objectTy = getType(object.type);
    for (auto i = 0u; i < objectTy.sizeInComponents; i++)
    {
        object.constantValue[i] = 0;
    }
}

void SpirvShader::AnalysisPass::OpConstantComposite(InsnIterator insn)
{
    auto &object = shader->CreateConstant(insn);
    auto offset = 0u;
    for (auto i = 0u; i < insn.wordCount() - 3; i++)
    {
        auto &constituent = getObject(insn.word(i + 3));
        auto &constituentTy = getType(constituent.type);
        for (auto j = 0u; j < constituentTy.sizeInComponents; j++)
            object.constantValue[offset++] = constituent.constantValue[j];
    }
}

void SpirvShader::AnalysisPass::OpCapability(InsnIterator insn)
{
    // Various capabilities will be declared, but none affect our code generation at this point.
}

void SpirvShader::AnalysisPass::OpMemoryModel(InsnIterator insn)
{
	// Memory model does not affect our code generation until we decide to do Vulkan Memory Model support.
}

// Due to preprocessing, the entrypoint and its function provide no value.
// void SpirvShader::AnalysisPass::OpEntryPoint(InsnIterator insn)
// void SpirvShader::AnalysisPass::OpFunction(InsnIterator insn)
// void SpirvShader::AnalysisPass::OpFunctionEnd(InsnIterator insn)

void SpirvShader::AnalysisPass::OpExtInstImport(InsnIterator insn)
{
    // We will only support the GLSL 450 extended instruction set, so no point in tracking the ID we assign it.
    // Valid shaders will not attempt to import any other instruction sets.
}


// These should have all been removed by preprocessing passes. If we see them here,
// our assumptions are wrong and we will probably generate wrong code.
void SpirvShader::AnalysisPass::OpFunctionParameter(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpFunctionCall(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpSpecConstant(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpSpecConstantComposite(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpSpecConstantFalse(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpSpecConstantOp(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }
void SpirvShader::AnalysisPass::OpSpecConstantTrue(InsnIterator) { UNIMPLEMENTED("This instructions should have already been lowered."); }

void SpirvShader::AnalysisPass::OpLoad(InsnIterator insn)
{
    SsaInst(insn);
}

void SpirvShader::AnalysisPass::OpAccessChain(InsnIterator insn)
{
    auto &object = SsaInst(insn);

    // interior ptr has two parts:
    // - logical base ptr, common across all lanes and known at compile time
    // - per-lane offset
    ObjectID baseId = insn.word(3);
    object.pointerBase = getObject(baseId).pointerBase;
}

SpirvShader::Object& SpirvShader::AnalysisPass::SsaInst(InsnIterator insn)
{
    TypeID typeId = insn.word(1);
    ObjectID resultId = insn.word(2);
    auto &object = shader->defs[resultId];
    object.type = typeId;
    object.kind = Object::Kind::Value;
    object.definition = insn;
    return object;
}

void SpirvShader::AnalysisPass::OpKill(InsnIterator insn)
{
    shader->modes.ContainsKill = true;
}

void SpirvShader::AnalysisPass::DeclareType(InsnIterator insn)
{
    TypeID resultId = insn.word(1);

    auto &type = shader->types[resultId];
    type.definition = insn;
    type.sizeInComponents = shader->ComputeTypeSize(insn);

    // A structure is a builtin block if it has a builtin
    // member. All members of such a structure are builtins.
    switch (insn.opcode())
    {
    case spv::OpTypeStruct:
    {
        auto d = shader->memberDecorations.find(resultId);
        if (d != shader->memberDecorations.end())
        {
            for (auto &m : d->second)
            {
                if (m.HasBuiltIn)
                {
                    type.isBuiltInBlock = true;
                    break;
                }
            }
        }
        break;
    }
    case spv::OpTypePointer:
    {
        TypeID elementTypeId = insn.word(3);
        type.element = elementTypeId;
        type.isBuiltInBlock = getType(elementTypeId).isBuiltInBlock;
        type.storageClass = static_cast<spv::StorageClass>(insn.word(2));
        break;
    }
    case spv::OpTypeVector:
    case spv::OpTypeMatrix:
    case spv::OpTypeArray:
    case spv::OpTypeRuntimeArray:
    {
        TypeID elementTypeId = insn.word(2);
        type.element = elementTypeId;
        break;
    }
    default:
        break;
    }
}

}
