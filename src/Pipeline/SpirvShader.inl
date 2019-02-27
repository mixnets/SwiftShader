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

#include "SpirvShader.hpp"

namespace sw
{

template<typename F>
int SpirvShader::VisitInterfaceInner(TypeID id, Decorations d, F f) const
{
    // Recursively walks variable definition and its type tree, taking into account
    // any explicit Location or Component decorations encountered; where explicit
    // Locations or Components are not specified, assigns them sequentially.
    // Collected decorations are carried down toward the leaves and across
    // siblings; Effect of decorations intentionally does not flow back up the tree.
    //
    // F is a functor to be called with the effective decoration set for every component.
    //
    // Returns the next available location, and calls f().

    // This covers the rules in Vulkan 1.1 spec, 14.1.4 Location Assignment.

    ApplyDecorationsForId(&d, id);

    auto const &obj = getType(id);
    switch (obj.definition.opcode())
    {
    case spv::OpTypePointer:
        return VisitInterfaceInner<F>(obj.definition.word(3), d, f);
    case spv::OpTypeMatrix:
        for (auto i = 0u; i < obj.definition.word(3); i++, d.Location++)
        {
            // consumes same components of N consecutive locations
            VisitInterfaceInner<F>(obj.definition.word(2), d, f);
        }
        return d.Location;
    case spv::OpTypeVector:
        for (auto i = 0u; i < obj.definition.word(3); i++, d.Component++)
        {
            // consumes N consecutive components in the same location
            VisitInterfaceInner<F>(obj.definition.word(2), d, f);
        }
        return d.Location + 1;
    case spv::OpTypeFloat:
        f(d, ATTRIBTYPE_FLOAT);
        return d.Location + 1;
    case spv::OpTypeInt:
        f(d, obj.definition.word(3) ? ATTRIBTYPE_INT : ATTRIBTYPE_UINT);
        return d.Location + 1;
    case spv::OpTypeBool:
        f(d, ATTRIBTYPE_UINT);
        return d.Location + 1;
    case spv::OpTypeStruct:
    {
        // iterate over members, which may themselves have Location/Component decorations
        for (auto i = 0u; i < obj.definition.wordCount() - 2; i++)
        {
            ApplyDecorationsForIdMember(&d, id, i);
            d.Location = VisitInterfaceInner<F>(obj.definition.word(i + 2), d, f);
            d.Component = 0;    // Implicit locations always have component=0
        }
        return d.Location;
    }
    case spv::OpTypeArray:
    {
        auto arraySize = GetConstantInt(obj.definition.word(3));
        for (auto i = 0u; i < arraySize; i++)
        {
            d.Location = VisitInterfaceInner<F>(obj.definition.word(2), d, f);
        }
        return d.Location;
    }
    default:
        // Intentionally partial; most opcodes do not participate in type hierarchies
        return 0;
    }
}

template<typename F>
void SpirvShader::VisitInterface(ObjectID id, F f) const
{
    // Walk a variable definition and call f for each component in it.
    Decorations d{};
    ApplyDecorationsForId(&d, id);

    auto def = getObject(id).definition;
    assert(def.opcode() == spv::OpVariable);
    VisitInterfaceInner<F>(def.word(1), d, f);
}

} // namespace sw
