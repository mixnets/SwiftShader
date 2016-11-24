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

#include "Optimizer.hpp"

#include "src/IceCfg.h"
#include "src/IceCfgNode.h"

#include <map>
#include <vector>

namespace
{
	class Optimizer
	{
	public:
		void optimize(Ice::Cfg *function)
		{
			analyzeUses(function);

			Ice::CfgNode *entryBlock = function->getEntryNode();

			for(Ice::Inst &alloca : entryBlock->getInsts())
			{
				if(alloca.isDeleted())
				{
					continue;
				}

				if(!llvm::isa<Ice::InstAlloca>(alloca))
				{
					break;   // Allocas are all at the top
				}

				Ice::Operand *address = alloca.getDest();
				auto addressEntry = uses.find(address);

				if(addressEntry == uses.end())
				{
					alloca.setDeleted();

					continue;
				}
				
				auto &addressUses = addressEntry->second;

				if(addressUses.stores.empty() && addressUses.allUsesAreLoads())
				{
					for(Ice::Inst *load : addressUses.loads)
					{
						Ice::Variable *loadData = load->getDest();

						for(Ice::Inst *use : uses[loadData].operands)
						{
							for(int i = 0; i < use->getSrcSize(); i++)
							{
								if(use->getSrc(i) == loadData)
								{
									auto *undef = function->getContext()->getConstantUndef(loadData->getType());
									
									use->replaceSource(i, undef);
								}
							}
						}

						uses.erase(loadData);

						load->setDeleted();
					}

					alloca.setDeleted();
					uses.erase(addressEntry);

					continue;
				}
			}
		}

	private:
		void analyzeUses(Ice::Cfg *function)
		{
			uses.clear();

			for(Ice::CfgNode *basicBlock : function->getNodes())
			{
				for(Ice::Inst &instruction : basicBlock->getInsts())
				{
					if(instruction.isDeleted())
					{
						continue;
					}

					for(int i = 0; i < instruction.getSrcSize(); i++)
					{
						int unique = 0;
						for(; unique < i; unique++)
						{
							if(instruction.getSrc(i) == instruction.getSrc(unique))
							{
								break;
							}
						}

						if(i == unique)
						{
							auto &valueUses = uses[instruction.getSrc(i)];

							valueUses.operands.push_back(&instruction);

							if(i == 0 && isLoad(instruction))   // Load address is first operand
							{
								valueUses.loads.push_back(&instruction);
							}
							else if(i == 1 && isStore(instruction))   // Store address is second operand
							{
								valueUses.stores.push_back(&instruction);
							}
						}
					}
				}
			}
		}

		bool isLoad(const Ice::Inst &instruction)
		{
			if(llvm::isa<Ice::InstLoad>(&instruction))
			{
				return true;
			}

			if(auto intrinsicCall = llvm::dyn_cast<Ice::InstIntrinsicCall>(&instruction))
			{
				return intrinsicCall->getIntrinsicInfo().ID == Ice::Intrinsics::LoadSubVector;
			}

			return false;
		}

		bool isStore(const Ice::Inst &instruction)
		{
			if(llvm::isa<Ice::InstStore>(&instruction))
			{
				return true;
			}

			if(auto intrinsicCall = llvm::dyn_cast<Ice::InstIntrinsicCall>(&instruction))
			{
				return intrinsicCall->getIntrinsicInfo().ID == Ice::Intrinsics::StoreSubVector;
			}

			return false;
		}

		struct Uses
		{
			std::vector<Ice::Inst*> operands;
			std::vector<Ice::Inst*> loads;
			std::vector<Ice::Inst*> stores;

			bool allUsesAreLoads()
			{
				return operands.size() == loads.size();
			}
		};

		std::map<Ice::Operand*, Uses> uses;
	};
}

namespace sw
{
	
	void optimize(Ice::Cfg *function)
	{
		Optimizer optimizer;

		optimizer.optimize(function);
	}
}