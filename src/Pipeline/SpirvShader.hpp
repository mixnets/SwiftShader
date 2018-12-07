// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_SpirvShader_hpp
#define sw_SpirvShader_hpp

#include "System/Types.hpp"
#include "System/Debug.hpp"

#include <string>
#include <vector>
#include <cstdint>
#include <spirv/unified1/spirv.hpp>

namespace sw
{
    /* TODO: consider sharing copy of instruction store with something closer to the API */

    class SpirvShader
    {
    public:
        using InsnStore = std::vector<uint32_t>;
        InsnStore insns;

        /* Pseudo-iterator over SPIRV instructions, barely good enough for range-based-for. */
        class InsnIterator
        {
            InsnStore::const_iterator iter;
            InsnStore::const_iterator zero;     /* for determining offset */

        public:
            spv::Op opcode() const { return static_cast<spv::Op>(*iter & spv::OpCodeMask); }
            uint32_t wordCount() const { return *iter >> spv::WordCountShift; }
            uint32_t offset() const { return static_cast<uint32_t>(iter - zero); }
            uint32_t word(uint32_t n) const { ASSERT(n < wordCount()); return iter[n]; }

            bool operator != (InsnIterator const & other) const { return iter == other.iter; }
            InsnIterator operator* () const { return *this; }
            InsnIterator& operator++ () { iter += wordCount(); return *this; }
            InsnIterator const operator++ (int) { InsnIterator ret{*this}; iter += wordCount(); return ret; }
            InsnIterator(InsnIterator const & other) = default;
            InsnIterator(InsnStore::const_iterator iter, InsnStore::const_iterator zero) : iter{iter}, zero{zero} {}
        };

        /* range-based-for interface */
        InsnIterator begin() const { return InsnIterator{insns.cbegin(), insns.cbegin()}; }
        InsnIterator end() const { return InsnIterator{insns.cend(), insns.cbegin()}; }

        int getSerialID() const { return serialID; }

        explicit SpirvShader(InsnStore const & insns);

    private:
        const int serialID;
        static volatile int serialCounter;
    };
}

#endif  // sw_SpirvShader_hpp