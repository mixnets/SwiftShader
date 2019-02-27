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

void SpirvShader::EmitEpilogPass::OpVariable(InsnIterator insn)
{
    ObjectID resultId = insn.word(2);
    auto &object = getObject(resultId);
    auto &objectTy = getType(object.type);
    if (object.kind == Object::Kind::InterfaceVariable && objectTy.storageClass == spv::StorageClassOutput)
    {
        auto &dst = routine->getValue(resultId);
        int offset = 0;
        shader->VisitInterface(resultId,
                        [&](Decorations const &d, AttribType type) {
                            auto scalarSlot = d.Location << 2 | d.Component;
                            routine->outputs[scalarSlot] = dst[offset++];
                        });
    }
}

void SpirvShader::emitEpilog(SpirvRoutine *routine) const
{
    EmitEpilogPass(this, routine).run();
}

} // namespace sw
