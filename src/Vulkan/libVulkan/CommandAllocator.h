// Copyright 2017 The NXT Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BACKEND_COMMAND_ALLOCATOR_H_
#define BACKEND_COMMAND_ALLOCATOR_H_

#include <cstdint>
#include <cstddef>
#include <vector>

namespace backend {

    // Allocation for command buffers should be fast. To avoid doing an allocation per command
    // or to avoid copying commands when reallocing, we use a linear allocator in a growing set
    // of large memory blocks. We also use this to have the format to be (u32 commandId, command),
    // so that iteration over the commands is easy.

    // Usage of the allocator and iterator:
    //     CommandAllocator allocator;
    //     DrawCommand* cmd = allocator.Allocate<DrawCommand>(CommandType::Draw);
    //     // Fill command
    //     // Repeat allocation and filling commands
    //
    //     CommandIterator commands(allocator);
    //     CommandType type;
    //     void* command;
    //     while(commands.NextCommandId(&e)) {
    //         switch(e) {
    //              case CommandType::Draw:
    //                  DrawCommand* draw = commands.NextCommand<DrawCommand>();
    //                  // Do the draw
    //                  break;
    //              // other cases
    //         }
    //     }

    // Note that you need to extract the commands from the CommandAllocator before destroying it
    // and must tell the CommandIterator when the allocated commands have been processed for
    // deletion.

    // These are the lists of blocks, should not be used directly, only through CommandAllocator
    // and CommandIterator
    struct BlockDef {
        size_t size;
        uint8_t* block;
    };
    using CommandBlocks = std::vector<BlockDef>;

    class CommandAllocator;

    // TODO(cwallez@chromium.org): prevent copy for both iterator and allocator
    class CommandIterator {
        public:
            CommandIterator();
            ~CommandIterator();

            CommandIterator(CommandIterator&& other);
            CommandIterator& operator=(CommandIterator&& other);

            CommandIterator(CommandAllocator&& allocator);
            CommandIterator& operator=(CommandAllocator&& allocator);

            template<typename E>
            bool NextCommandId(E* commandId) {
                return NextCommandId(reinterpret_cast<uint32_t*>(commandId));
            }
            template<typename T>
            T* NextCommand() {
                return reinterpret_cast<T*>(NextCommand(sizeof(T), alignof(T)));
            }
            template<typename T>
            T* NextData(size_t count) {
                return reinterpret_cast<T*>(NextData(sizeof(T) * count, alignof(T)));
            }

            // Needs to be called if iteration was stopped early.
            void Reset();

            void DataWasDestroyed();

        private:
            bool IsEmpty() const;

            bool NextCommandId(uint32_t* commandId);
            void* NextCommand(size_t commandSize, size_t commandAlignment);
            void* NextData(size_t dataSize, size_t dataAlignment);

            CommandBlocks blocks;
            uint8_t* currentPtr = nullptr;
            size_t currentBlock = 0;
            // Used to avoid a special case for empty iterators.
            uint32_t endOfBlock;
            bool dataWasDestroyed = false;
    };

    class CommandAllocator {
        public:
            CommandAllocator();
            ~CommandAllocator();

            template<typename T, typename E>
            T* Allocate(E commandId) {
                static_assert(sizeof(E) == sizeof(uint32_t), "");
                static_assert(alignof(E) == alignof(uint32_t), "");
                return reinterpret_cast<T*>(Allocate(static_cast<uint32_t>(commandId), sizeof(T), alignof(T)));
            }

            template<typename T>
            T* AllocateData(size_t count) {
                return reinterpret_cast<T*>(AllocateData(sizeof(T) * count, alignof(T)));
            }

        private:
            friend CommandIterator;
            CommandBlocks&& AcquireBlocks();

            uint8_t* Allocate(uint32_t commandId, size_t commandSize, size_t commandAlignment);
            uint8_t* AllocateData(size_t dataSize, size_t dataAlignment);
            bool GetNewBlock(size_t minimumSize);

            CommandBlocks blocks;
            size_t lastAllocationSize = 2048;

            // Pointers to the current range of allocation in the block. Guaranteed to allow
            // for at least one uint32_t is not nullptr, so that the special EndOfBlock command id
            // can always be written.
            // Nullptr iff the blocks were moved out.
            uint8_t* currentPtr = nullptr;
            uint8_t* endPtr = nullptr;

            // Data used for the block range at initialization so that the first call to Allocate
            // sees there is not enough space and calls GetNewBlock. This avoids having to special
            // case the initialization in Allocate.
            uint32_t dummyEnum[1] = {0};
    };

}

#endif // BACKEND_COMMAND_ALLOCATOR_H_
