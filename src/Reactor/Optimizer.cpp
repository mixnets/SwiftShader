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

				if(!addressUses.areOnlyLoadStore())
				{
					continue;
				}

				auto &loads = addressUses.loads;
				auto &stores = addressUses.stores;

				if(stores.size() == 0)
				{
					for(Ice::Inst *load : loads)
					{
						Ice::Variable *loadData = load->getDest();

						for(Ice::Inst *use : uses[loadData])
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

				if(stores.size() == 1)
				{
					Ice::Inst *store = stores[0];
					Ice::Operand *storeValue = store->getSrc(0);

					for(Ice::Inst *load = &node[store]->getInsts().back(); load != store; load = &*(--(*load).getIterator()))
					{
						if(load->isDeleted() || !isLoad(*load))
						{
							continue;
						}

						Ice::Operand *loadAddress = load->getSrc(0);

						if(loadAddress != address)
						{
							continue;
						}

						replace(load->getDest(), storeValue);
						deleteInstruction(load);

						for(int i = 0; i < loads.size(); i++)
						{
							if(loads[i] == load)
							{
								loads[i] = loads.back();
								loads.pop_back();
								break;
							}
						}

						for(int i = 0; i < addressUses.size(); i++)
						{
							if(addressUses[i] == load)
							{
								addressUses[i] = addressUses.back();
								addressUses.pop_back();
								break;
							}
						}

						if(addressUses.size() == 1)
						{
							assert(addressUses[0] == store);

							alloca.setDeleted();
							store->setDeleted();
							this->uses.erase(address);

							auto &valueUses = this->uses[storeValue];

							for(int i = 0; i < valueUses.size(); i++)
							{
								if(valueUses[i] == store)
								{
									valueUses[i] = valueUses.back();
									valueUses.pop_back();
									break;
								}
							}

							if(valueUses.size() == 0)
							{
								this->uses.erase(storeValue);
							}

							break;
						}
					}

					continue;
				}
			}
		}

	private:
		void analyzeUses(Ice::Cfg *function)
		{
			uses.clear();
			node.clear();
			definition.clear();

			for(Ice::CfgNode *basicBlock : function->getNodes())
			{
				for(Ice::Inst &instruction : basicBlock->getInsts())
				{
					if(instruction.isDeleted())
					{
						continue;
					}

					node[&instruction] = basicBlock;
					definition[instruction.getDest()] = &instruction;

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

							valueUses.push_back(&instruction);

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

		void replace(Ice::Operand *oldValue, Ice::Operand *newValue)
		{
			for(Ice::Inst *use : uses[oldValue])
			{
				if(use->isDeleted())
				{
					continue;
				}

				bool hit = false;
				for(int i = 0; i < use->getSrcSize(); i++)
				{
					if(use->getSrc(i) == oldValue)
					{
						use->replaceSource(i, newValue);
						hit = true;
					}
				}

				if(hit)
				{
					uses[newValue].push_back(use);

					if(llvm::isa<Ice::InstLoad>(use) || (llvm::isa<Ice::InstIntrinsicCall>(use) && llvm::cast<Ice::InstIntrinsicCall>(use)->getIntrinsicInfo().ID == Ice::Intrinsics::LoadSubVector))
					{
						if(newValue == use->getSrc(0))
						{
							uses[newValue].loads.push_back(use);
						}
					}
					else if(llvm::isa<Ice::InstStore>(use) || (llvm::isa<Ice::InstIntrinsicCall>(use) && llvm::cast<Ice::InstIntrinsicCall>(use)->getIntrinsicInfo().ID == Ice::Intrinsics::StoreSubVector))
					{
						if(newValue == use->getSrc(1))
						{
							uses[newValue].stores.push_back(use);
						}
					}
				}
			}

			uses.erase(oldValue);

			if(Ice::Variable *var = llvm::dyn_cast<Ice::Variable>(oldValue))
			{
				deleteInstruction(definition[var]);
			}
		}

		void deleteInstruction(Ice::Inst *instruction)
		{
			if(instruction->isDeleted())
			{
				return;
			}

			instruction->setDeleted();

			for(int i = 0; i < instruction->getSrcSize(); i++)
			{
				Ice::Operand *src = instruction->getSrc(i);

				auto &srcEntry = uses.find(src);

				if(srcEntry != uses.end())
				{
					auto &srcUses = srcEntry->second;

					srcUses.erase(instruction);

					if(srcUses.size() == 0)
					{
						uses.erase(srcEntry);

						if(Ice::Variable *var = llvm::dyn_cast<Ice::Variable>(src))
						{
							deleteInstruction(definition[var]);
						}
					}
				}
			}
		}

		struct Uses : std::vector<Ice::Inst*>
		{
			std::vector<Ice::Inst*> loads;
			std::vector<Ice::Inst*> stores;

			bool areOnlyLoadStore()
			{
				return size() == (loads.size() + stores.size());
			}

			void erase(Ice::Inst *instruction)
			{
				auto &uses = *this;

				for(int i = 0; i < uses.size(); i++)
				{
					if(uses[i] == instruction)
					{
						uses[i] = back();
						pop_back();

						for(int i = 0; i < loads.size(); i++)
						{
							if(loads[i] == instruction)
							{
								loads[i] = loads.back();
								loads.pop_back();
								break;
							}
						}

						for(int i = 0; i < stores.size(); i++)
						{
							if(stores[i] == instruction)
							{
								stores[i] = stores.back();
								stores.pop_back();
								break;
							}
						}

						break;
					}
				}
			}
		};

		std::map<Ice::Operand*, Uses> uses;
		std::map<Ice::Inst*, Ice::CfgNode*> node;
		std::map<Ice::Variable*, Ice::Inst*> definition;
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