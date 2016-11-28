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
			context = function->getContext();
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

					for(Ice::Inst *load = &*++store->getIterator(), *next = nullptr; load != next; next = load, load = &*++store->getIterator())
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

						replace(load, storeValue);

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

				Ice::CfgNode *singleBasicBlock = node[stores[0]];

				for(int i = 1; i < stores.size(); i++)
				{
					Ice::Inst *store = stores[i];
					if(node[store] != singleBasicBlock)
					{
						singleBasicBlock = nullptr;
						break;
					}
				}

				if(singleBasicBlock)
				{
					auto &insts = singleBasicBlock->getInsts();
					Ice::Inst *store = nullptr;
					Ice::Operand *storeValue = nullptr;

					for(Ice::Inst &inst : insts)
					{
						if(inst.isDeleted())
						{
							continue;
						}

						if(isStore(inst))
						{
							if(inst.getSrc(1) != address)
							{
								continue;
							}

							// New store found. If we had a previous one, eliminate it.
							if(store)
							{
								deleteInstruction(store);
							}

							store = &inst;
							storeValue = store->getSrc(0);
						}
						else if(isLoad(inst))
						{
							Ice::Inst *load = &inst;

							if(load->getSrc(0) != address)
							{
								continue;
							}

							replace(load, storeValue);
						}
					}

					if(loads.size() != 0)
					{

					}
				}
			}

			for(Ice::CfgNode *basicBlock : function->getNodes())
			{
				std::map<Ice::Operand*, Ice::Inst*> stores;

				for(Ice::Inst &inst : basicBlock->getInsts())
				{
					if(inst.isDeleted())
					{
						continue;
					}

					if(isStore(inst))
					{
						if(!uses[inst.getSrc(1)].areOnlyLoadStore())
						{
							continue;
						}

						// New store found. If we had a previous one, eliminate it.
						if(stores.find(inst.getSrc(1)) != stores.end())
						{
							deleteInstruction(stores[inst.getSrc(1)]);
						}

						stores[inst.getSrc(1)] = &inst;
					}
					else if(isLoad(inst))
					{
						Ice::Inst *load = &inst;

						if(stores.find(inst.getSrc(0)) == stores.end())
						{
							continue;
						}

						replace(load, stores[inst.getSrc(0)]->getSrc(0));
					}
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
							Ice::Operand *src = instruction.getSrc(i);
							uses[src].insert(src, &instruction);
						}
					}
				}
			}
		}

		static bool isLoad(const Ice::Inst &instruction)
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

		static bool isStore(const Ice::Inst &instruction)
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

		void replace(Ice::Inst *instruction, Ice::Operand *newValue)
		{
			Ice::Variable *oldValue = instruction->getDest();

			if(!newValue)
			{
				newValue = context->getConstantUndef(oldValue->getType());
			}

			for(Ice::Inst *use : uses[oldValue])
			{
				assert(!use->isDeleted());   // Should have been removed from uses already

				for(int i = 0; i < use->getSrcSize(); i++)
				{
					if(use->getSrc(i) == oldValue)
					{
						use->replaceSource(i, newValue);
					}
				}

				uses[newValue].insert(newValue, use);
			}

			uses.erase(oldValue);

			deleteInstruction(instruction);
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

			void insert(Ice::Operand *value, Ice::Inst *instruction)
			{
				push_back(instruction);

				if(isLoad(*instruction))
				{
					if(value == instruction->getSrc(0))   // Load address is first operand
					{
						loads.push_back(instruction);
					}
				}
				else if(isStore(*instruction))
				{
					if(value == instruction->getSrc(1))   // Store address is second operand
					{
						stores.push_back(instruction);
					}
				}
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

		Ice::GlobalContext *context;

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