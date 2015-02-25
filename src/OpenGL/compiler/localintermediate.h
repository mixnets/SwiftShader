//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef _LOCAL_INTERMEDIATE_INCLUDED_
#define _LOCAL_INTERMEDIATE_INCLUDED_

#include "intermediate.h"

struct TVectorFields {
    int offsets[4];
    int num;
};

//
// Set of helper functions to help parse and build the tree.
//
class TInfoSink;
class TIntermediate {
public:
    POOL_ALLOCATOR_NEW_DELETE();

    TIntermediate(TInfoSink& i) : infoSink(i) { }
    TIntermSymbol* addSymbol(int Id, const TString&, const TType&, const TSourceLoc &line);
    TIntermTyped* addBinaryMath(TOperator op, TIntermTyped* left, TIntermTyped* right, const TSourceLoc &line);
    TIntermTyped* addAssign(TOperator op, TIntermTyped* left, TIntermTyped* right, const TSourceLoc &line);
    TIntermTyped* addIndex(TOperator op, TIntermTyped* base, TIntermTyped* index, const TSourceLoc &line);
    TIntermTyped* addUnaryMath(TOperator op, TIntermNode* child, const TSourceLoc &line);
    TIntermAggregate* growAggregate(TIntermNode* left, TIntermNode* right, const TSourceLoc &line);
    TIntermAggregate* makeAggregate(TIntermNode* node, const TSourceLoc &line);
    TIntermAggregate* setAggregateOperator(TIntermNode*, TOperator, const TSourceLoc &line);
    TIntermNode*  addSelection(TIntermTyped* cond, TIntermNodePair code, const TSourceLoc &line);
    TIntermTyped* addSelection(TIntermTyped* cond, TIntermTyped* trueBlock, TIntermTyped* falseBlock, const TSourceLoc &line);
    TIntermTyped* addComma(TIntermTyped* left, TIntermTyped* right, const TSourceLoc &line);
    TIntermConstantUnion* addConstantUnion(ConstantUnion*, const TType&, const TSourceLoc &line);
    TIntermTyped* promoteConstantUnion(TBasicType, TIntermConstantUnion*);
    bool parseConstTree(TSourceLoc, TIntermNode*, ConstantUnion*, TOperator, TType, bool singleConstantParam = false);
    TIntermNode* addLoop(TLoopType, TIntermNode*, TIntermTyped*, TIntermTyped*, TIntermNode*, const TSourceLoc &line);
    TIntermBranch* addBranch(TOperator, const TSourceLoc &line);
    TIntermBranch* addBranch(TOperator, TIntermTyped*, const TSourceLoc &line);
    TIntermTyped* addSwizzle(TVectorFields&, const TSourceLoc &line);
    bool postProcess(TIntermNode*);
    void outputTree(TIntermNode*);
    
protected:
    TInfoSink& infoSink;

private:
    void operator=(TIntermediate&); // prevent assignments
};

#endif // _LOCAL_INTERMEDIATE_INCLUDED_
