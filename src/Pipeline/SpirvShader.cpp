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

#include <spirv/unified1/spirv.hpp>
#include "SpirvShader.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkBuffer.hpp"
#include "Vulkan/VkDebug.hpp"
#include "Device/Config.hpp"

namespace
{

	const char* opcodeName(spv::Op op) {
		switch(op){
		case spv::Op::OpNop: return "Nop";
		case spv::Op::OpUndef: return "Undef";
		case spv::Op::OpSourceContinued: return "SourceContinued";
		case spv::Op::OpSource: return "Source";
		case spv::Op::OpSourceExtension: return "SourceExtension";
		case spv::Op::OpName: return "Name";
		case spv::Op::OpMemberName: return "MemberName";
		case spv::Op::OpString: return "String";
		case spv::Op::OpLine: return "Line";
		case spv::Op::OpExtension: return "Extension";
		case spv::Op::OpExtInstImport: return "ExtInstImport";
		case spv::Op::OpExtInst: return "ExtInst";
		case spv::Op::OpMemoryModel: return "MemoryModel";
		case spv::Op::OpEntryPoint: return "EntryPoint";
		case spv::Op::OpExecutionMode: return "ExecutionMode";
		case spv::Op::OpCapability: return "Capability";
		case spv::Op::OpTypeVoid: return "TypeVoid";
		case spv::Op::OpTypeBool: return "TypeBool";
		case spv::Op::OpTypeInt: return "TypeInt";
		case spv::Op::OpTypeFloat: return "TypeFloat";
		case spv::Op::OpTypeVector: return "TypeVector";
		case spv::Op::OpTypeMatrix: return "TypeMatrix";
		case spv::Op::OpTypeImage: return "TypeImage";
		case spv::Op::OpTypeSampler: return "TypeSampler";
		case spv::Op::OpTypeSampledImage: return "TypeSampledImage";
		case spv::Op::OpTypeArray: return "TypeArray";
		case spv::Op::OpTypeRuntimeArray: return "TypeRuntimeArray";
		case spv::Op::OpTypeStruct: return "TypeStruct";
		case spv::Op::OpTypeOpaque: return "TypeOpaque";
		case spv::Op::OpTypePointer: return "TypePointer";
		case spv::Op::OpTypeFunction: return "TypeFunction";
		case spv::Op::OpTypeEvent: return "TypeEvent";
		case spv::Op::OpTypeDeviceEvent: return "TypeDeviceEvent";
		case spv::Op::OpTypeReserveId: return "TypeReserveId";
		case spv::Op::OpTypeQueue: return "TypeQueue";
		case spv::Op::OpTypePipe: return "TypePipe";
		case spv::Op::OpTypeForwardPointer: return "TypeForwardPointer";
		case spv::Op::OpConstantTrue: return "ConstantTrue";
		case spv::Op::OpConstantFalse: return "ConstantFalse";
		case spv::Op::OpConstant: return "Constant";
		case spv::Op::OpConstantComposite: return "ConstantComposite";
		case spv::Op::OpConstantSampler: return "ConstantSampler";
		case spv::Op::OpConstantNull: return "ConstantNull";
		case spv::Op::OpSpecConstantTrue: return "SpecConstantTrue";
		case spv::Op::OpSpecConstantFalse: return "SpecConstantFalse";
		case spv::Op::OpSpecConstant: return "SpecConstant";
		case spv::Op::OpSpecConstantComposite: return "SpecConstantComposite";
		case spv::Op::OpSpecConstantOp: return "SpecConstantOp";
		case spv::Op::OpFunction: return "Function";
		case spv::Op::OpFunctionParameter: return "FunctionParameter";
		case spv::Op::OpFunctionEnd: return "FunctionEnd";
		case spv::Op::OpFunctionCall: return "FunctionCall";
		case spv::Op::OpVariable: return "Variable";
		case spv::Op::OpImageTexelPointer: return "ImageTexelPointer";
		case spv::Op::OpLoad: return "Load";
		case spv::Op::OpStore: return "Store";
		case spv::Op::OpCopyMemory: return "CopyMemory";
		case spv::Op::OpCopyMemorySized: return "CopyMemorySized";
		case spv::Op::OpAccessChain: return "AccessChain";
		case spv::Op::OpInBoundsAccessChain: return "InBoundsAccessChain";
		case spv::Op::OpPtrAccessChain: return "PtrAccessChain";
		case spv::Op::OpArrayLength: return "ArrayLength";
		case spv::Op::OpGenericPtrMemSemantics: return "GenericPtrMemSemantics";
		case spv::Op::OpInBoundsPtrAccessChain: return "InBoundsPtrAccessChain";
		case spv::Op::OpDecorate: return "Decorate";
		case spv::Op::OpMemberDecorate: return "MemberDecorate";
		case spv::Op::OpDecorationGroup: return "DecorationGroup";
		case spv::Op::OpGroupDecorate: return "GroupDecorate";
		case spv::Op::OpGroupMemberDecorate: return "GroupMemberDecorate";
		case spv::Op::OpVectorExtractDynamic: return "VectorExtractDynamic";
		case spv::Op::OpVectorInsertDynamic: return "VectorInsertDynamic";
		case spv::Op::OpVectorShuffle: return "VectorShuffle";
		case spv::Op::OpCompositeConstruct: return "CompositeConstruct";
		case spv::Op::OpCompositeExtract: return "CompositeExtract";
		case spv::Op::OpCompositeInsert: return "CompositeInsert";
		case spv::Op::OpCopyObject: return "CopyObject";
		case spv::Op::OpTranspose: return "Transpose";
		case spv::Op::OpSampledImage: return "SampledImage";
		case spv::Op::OpImageSampleImplicitLod: return "ImageSampleImplicitLod";
		case spv::Op::OpImageSampleExplicitLod: return "ImageSampleExplicitLod";
		case spv::Op::OpImageSampleDrefImplicitLod: return "ImageSampleDrefImplicitLod";
		case spv::Op::OpImageSampleDrefExplicitLod: return "ImageSampleDrefExplicitLod";
		case spv::Op::OpImageSampleProjImplicitLod: return "ImageSampleProjImplicitLod";
		case spv::Op::OpImageSampleProjExplicitLod: return "ImageSampleProjExplicitLod";
		case spv::Op::OpImageSampleProjDrefImplicitLod: return "ImageSampleProjDrefImplicitLod";
		case spv::Op::OpImageSampleProjDrefExplicitLod: return "ImageSampleProjDrefExplicitLod";
		case spv::Op::OpImageFetch: return "ImageFetch";
		case spv::Op::OpImageGather: return "ImageGather";
		case spv::Op::OpImageDrefGather: return "ImageDrefGather";
		case spv::Op::OpImageRead: return "ImageRead";
		case spv::Op::OpImageWrite: return "ImageWrite";
		case spv::Op::OpImage: return "Image";
		case spv::Op::OpImageQueryFormat: return "ImageQueryFormat";
		case spv::Op::OpImageQueryOrder: return "ImageQueryOrder";
		case spv::Op::OpImageQuerySizeLod: return "ImageQuerySizeLod";
		case spv::Op::OpImageQuerySize: return "ImageQuerySize";
		case spv::Op::OpImageQueryLod: return "ImageQueryLod";
		case spv::Op::OpImageQueryLevels: return "ImageQueryLevels";
		case spv::Op::OpImageQuerySamples: return "ImageQuerySamples";
		case spv::Op::OpConvertFToU: return "ConvertFToU";
		case spv::Op::OpConvertFToS: return "ConvertFToS";
		case spv::Op::OpConvertSToF: return "ConvertSToF";
		case spv::Op::OpConvertUToF: return "ConvertUToF";
		case spv::Op::OpUConvert: return "UConvert";
		case spv::Op::OpSConvert: return "SConvert";
		case spv::Op::OpFConvert: return "FConvert";
		case spv::Op::OpQuantizeToF16: return "QuantizeToF16";
		case spv::Op::OpConvertPtrToU: return "ConvertPtrToU";
		case spv::Op::OpSatConvertSToU: return "SatConvertSToU";
		case spv::Op::OpSatConvertUToS: return "SatConvertUToS";
		case spv::Op::OpConvertUToPtr: return "ConvertUToPtr";
		case spv::Op::OpPtrCastToGeneric: return "PtrCastToGeneric";
		case spv::Op::OpGenericCastToPtr: return "GenericCastToPtr";
		case spv::Op::OpGenericCastToPtrExplicit: return "GenericCastToPtrExplicit";
		case spv::Op::OpBitcast: return "Bitcast";
		case spv::Op::OpSNegate: return "SNegate";
		case spv::Op::OpFNegate: return "FNegate";
		case spv::Op::OpIAdd: return "IAdd";
		case spv::Op::OpFAdd: return "FAdd";
		case spv::Op::OpISub: return "ISub";
		case spv::Op::OpFSub: return "FSub";
		case spv::Op::OpIMul: return "IMul";
		case spv::Op::OpFMul: return "FMul";
		case spv::Op::OpUDiv: return "UDiv";
		case spv::Op::OpSDiv: return "SDiv";
		case spv::Op::OpFDiv: return "FDiv";
		case spv::Op::OpUMod: return "UMod";
		case spv::Op::OpSRem: return "SRem";
		case spv::Op::OpSMod: return "SMod";
		case spv::Op::OpFRem: return "FRem";
		case spv::Op::OpFMod: return "FMod";
		case spv::Op::OpVectorTimesScalar: return "VectorTimesScalar";
		case spv::Op::OpMatrixTimesScalar: return "MatrixTimesScalar";
		case spv::Op::OpVectorTimesMatrix: return "VectorTimesMatrix";
		case spv::Op::OpMatrixTimesVector: return "MatrixTimesVector";
		case spv::Op::OpMatrixTimesMatrix: return "MatrixTimesMatrix";
		case spv::Op::OpOuterProduct: return "OuterProduct";
		case spv::Op::OpDot: return "Dot";
		case spv::Op::OpIAddCarry: return "IAddCarry";
		case spv::Op::OpISubBorrow: return "ISubBorrow";
		case spv::Op::OpUMulExtended: return "UMulExtended";
		case spv::Op::OpSMulExtended: return "SMulExtended";
		case spv::Op::OpAny: return "Any";
		case spv::Op::OpAll: return "All";
		case spv::Op::OpIsNan: return "IsNan";
		case spv::Op::OpIsInf: return "IsInf";
		case spv::Op::OpIsFinite: return "IsFinite";
		case spv::Op::OpIsNormal: return "IsNormal";
		case spv::Op::OpSignBitSet: return "SignBitSet";
		case spv::Op::OpLessOrGreater: return "LessOrGreater";
		case spv::Op::OpOrdered: return "Ordered";
		case spv::Op::OpUnordered: return "Unordered";
		case spv::Op::OpLogicalEqual: return "LogicalEqual";
		case spv::Op::OpLogicalNotEqual: return "LogicalNotEqual";
		case spv::Op::OpLogicalOr: return "LogicalOr";
		case spv::Op::OpLogicalAnd: return "LogicalAnd";
		case spv::Op::OpLogicalNot: return "LogicalNot";
		case spv::Op::OpSelect: return "Select";
		case spv::Op::OpIEqual: return "IEqual";
		case spv::Op::OpINotEqual: return "INotEqual";
		case spv::Op::OpUGreaterThan: return "UGreaterThan";
		case spv::Op::OpSGreaterThan: return "SGreaterThan";
		case spv::Op::OpUGreaterThanEqual: return "UGreaterThanEqual";
		case spv::Op::OpSGreaterThanEqual: return "SGreaterThanEqual";
		case spv::Op::OpULessThan: return "ULessThan";
		case spv::Op::OpSLessThan: return "SLessThan";
		case spv::Op::OpULessThanEqual: return "ULessThanEqual";
		case spv::Op::OpSLessThanEqual: return "SLessThanEqual";
		case spv::Op::OpFOrdEqual: return "FOrdEqual";
		case spv::Op::OpFUnordEqual: return "FUnordEqual";
		case spv::Op::OpFOrdNotEqual: return "FOrdNotEqual";
		case spv::Op::OpFUnordNotEqual: return "FUnordNotEqual";
		case spv::Op::OpFOrdLessThan: return "FOrdLessThan";
		case spv::Op::OpFUnordLessThan: return "FUnordLessThan";
		case spv::Op::OpFOrdGreaterThan: return "FOrdGreaterThan";
		case spv::Op::OpFUnordGreaterThan: return "FUnordGreaterThan";
		case spv::Op::OpFOrdLessThanEqual: return "FOrdLessThanEqual";
		case spv::Op::OpFUnordLessThanEqual: return "FUnordLessThanEqual";
		case spv::Op::OpFOrdGreaterThanEqual: return "FOrdGreaterThanEqual";
		case spv::Op::OpFUnordGreaterThanEqual: return "FUnordGreaterThanEqual";
		case spv::Op::OpShiftRightLogical: return "ShiftRightLogical";
		case spv::Op::OpShiftRightArithmetic: return "ShiftRightArithmetic";
		case spv::Op::OpShiftLeftLogical: return "ShiftLeftLogical";
		case spv::Op::OpBitwiseOr: return "BitwiseOr";
		case spv::Op::OpBitwiseXor: return "BitwiseXor";
		case spv::Op::OpBitwiseAnd: return "BitwiseAnd";
		case spv::Op::OpNot: return "Not";
		case spv::Op::OpBitFieldInsert: return "BitFieldInsert";
		case spv::Op::OpBitFieldSExtract: return "BitFieldSExtract";
		case spv::Op::OpBitFieldUExtract: return "BitFieldUExtract";
		case spv::Op::OpBitReverse: return "BitReverse";
		case spv::Op::OpBitCount: return "BitCount";
		case spv::Op::OpDPdx: return "DPdx";
		case spv::Op::OpDPdy: return "DPdy";
		case spv::Op::OpFwidth: return "Fwidth";
		case spv::Op::OpDPdxFine: return "DPdxFine";
		case spv::Op::OpDPdyFine: return "DPdyFine";
		case spv::Op::OpFwidthFine: return "FwidthFine";
		case spv::Op::OpDPdxCoarse: return "DPdxCoarse";
		case spv::Op::OpDPdyCoarse: return "DPdyCoarse";
		case spv::Op::OpFwidthCoarse: return "FwidthCoarse";
		case spv::Op::OpEmitVertex: return "EmitVertex";
		case spv::Op::OpEndPrimitive: return "EndPrimitive";
		case spv::Op::OpEmitStreamVertex: return "EmitStreamVertex";
		case spv::Op::OpEndStreamPrimitive: return "EndStreamPrimitive";
		case spv::Op::OpControlBarrier: return "ControlBarrier";
		case spv::Op::OpMemoryBarrier: return "MemoryBarrier";
		case spv::Op::OpAtomicLoad: return "AtomicLoad";
		case spv::Op::OpAtomicStore: return "AtomicStore";
		case spv::Op::OpAtomicExchange: return "AtomicExchange";
		case spv::Op::OpAtomicCompareExchange: return "AtomicCompareExchange";
		case spv::Op::OpAtomicCompareExchangeWeak: return "AtomicCompareExchangeWeak";
		case spv::Op::OpAtomicIIncrement: return "AtomicIIncrement";
		case spv::Op::OpAtomicIDecrement: return "AtomicIDecrement";
		case spv::Op::OpAtomicIAdd: return "AtomicIAdd";
		case spv::Op::OpAtomicISub: return "AtomicISub";
		case spv::Op::OpAtomicSMin: return "AtomicSMin";
		case spv::Op::OpAtomicUMin: return "AtomicUMin";
		case spv::Op::OpAtomicSMax: return "AtomicSMax";
		case spv::Op::OpAtomicUMax: return "AtomicUMax";
		case spv::Op::OpAtomicAnd: return "AtomicAnd";
		case spv::Op::OpAtomicOr: return "AtomicOr";
		case spv::Op::OpAtomicXor: return "AtomicXor";
		case spv::Op::OpPhi: return "Phi";
		case spv::Op::OpLoopMerge: return "LoopMerge";
		case spv::Op::OpSelectionMerge: return "SelectionMerge";
		case spv::Op::OpLabel: return "Label";
		case spv::Op::OpBranch: return "Branch";
		case spv::Op::OpBranchConditional: return "BranchConditional";
		case spv::Op::OpSwitch: return "Switch";
		case spv::Op::OpKill: return "Kill";
		case spv::Op::OpReturn: return "Return";
		case spv::Op::OpReturnValue: return "ReturnValue";
		case spv::Op::OpUnreachable: return "Unreachable";
		case spv::Op::OpLifetimeStart: return "LifetimeStart";
		case spv::Op::OpLifetimeStop: return "LifetimeStop";
		case spv::Op::OpGroupAsyncCopy: return "GroupAsyncCopy";
		case spv::Op::OpGroupWaitEvents: return "GroupWaitEvents";
		case spv::Op::OpGroupAll: return "GroupAll";
		case spv::Op::OpGroupAny: return "GroupAny";
		case spv::Op::OpGroupBroadcast: return "GroupBroadcast";
		case spv::Op::OpGroupIAdd: return "GroupIAdd";
		case spv::Op::OpGroupFAdd: return "GroupFAdd";
		case spv::Op::OpGroupFMin: return "GroupFMin";
		case spv::Op::OpGroupUMin: return "GroupUMin";
		case spv::Op::OpGroupSMin: return "GroupSMin";
		case spv::Op::OpGroupFMax: return "GroupFMax";
		case spv::Op::OpGroupUMax: return "GroupUMax";
		case spv::Op::OpGroupSMax: return "GroupSMax";
		case spv::Op::OpReadPipe: return "ReadPipe";
		case spv::Op::OpWritePipe: return "WritePipe";
		case spv::Op::OpReservedReadPipe: return "ReservedReadPipe";
		case spv::Op::OpReservedWritePipe: return "ReservedWritePipe";
		case spv::Op::OpReserveReadPipePackets: return "ReserveReadPipePackets";
		case spv::Op::OpReserveWritePipePackets: return "ReserveWritePipePackets";
		case spv::Op::OpCommitReadPipe: return "CommitReadPipe";
		case spv::Op::OpCommitWritePipe: return "CommitWritePipe";
		case spv::Op::OpIsValidReserveId: return "IsValidReserveId";
		case spv::Op::OpGetNumPipePackets: return "GetNumPipePackets";
		case spv::Op::OpGetMaxPipePackets: return "GetMaxPipePackets";
		case spv::Op::OpGroupReserveReadPipePackets: return "GroupReserveReadPipePackets";
		case spv::Op::OpGroupReserveWritePipePackets: return "GroupReserveWritePipePackets";
		case spv::Op::OpGroupCommitReadPipe: return "GroupCommitReadPipe";
		case spv::Op::OpGroupCommitWritePipe: return "GroupCommitWritePipe";
		case spv::Op::OpEnqueueMarker: return "EnqueueMarker";
		case spv::Op::OpEnqueueKernel: return "EnqueueKernel";
		case spv::Op::OpGetKernelNDrangeSubGroupCount: return "GetKernelNDrangeSubGroupCount";
		case spv::Op::OpGetKernelNDrangeMaxSubGroupSize: return "GetKernelNDrangeMaxSubGroupSize";
		case spv::Op::OpGetKernelWorkGroupSize: return "GetKernelWorkGroupSize";
		case spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple: return "GetKernelPreferredWorkGroupSizeMultiple";
		case spv::Op::OpRetainEvent: return "RetainEvent";
		case spv::Op::OpReleaseEvent: return "ReleaseEvent";
		case spv::Op::OpCreateUserEvent: return "CreateUserEvent";
		case spv::Op::OpIsValidEvent: return "IsValidEvent";
		case spv::Op::OpSetUserEventStatus: return "SetUserEventStatus";
		case spv::Op::OpCaptureEventProfilingInfo: return "CaptureEventProfilingInfo";
		case spv::Op::OpGetDefaultQueue: return "GetDefaultQueue";
		case spv::Op::OpBuildNDRange: return "BuildNDRange";
		case spv::Op::OpImageSparseSampleImplicitLod: return "ImageSparseSampleImplicitLod";
		case spv::Op::OpImageSparseSampleExplicitLod: return "ImageSparseSampleExplicitLod";
		case spv::Op::OpImageSparseSampleDrefImplicitLod: return "ImageSparseSampleDrefImplicitLod";
		case spv::Op::OpImageSparseSampleDrefExplicitLod: return "ImageSparseSampleDrefExplicitLod";
		case spv::Op::OpImageSparseSampleProjImplicitLod: return "ImageSparseSampleProjImplicitLod";
		case spv::Op::OpImageSparseSampleProjExplicitLod: return "ImageSparseSampleProjExplicitLod";
		case spv::Op::OpImageSparseSampleProjDrefImplicitLod: return "ImageSparseSampleProjDrefImplicitLod";
		case spv::Op::OpImageSparseSampleProjDrefExplicitLod: return "ImageSparseSampleProjDrefExplicitLod";
		case spv::Op::OpImageSparseFetch: return "ImageSparseFetch";
		case spv::Op::OpImageSparseGather: return "ImageSparseGather";
		case spv::Op::OpImageSparseDrefGather: return "ImageSparseDrefGather";
		case spv::Op::OpImageSparseTexelsResident: return "ImageSparseTexelsResident";
		case spv::Op::OpNoLine: return "NoLine";
		case spv::Op::OpAtomicFlagTestAndSet: return "AtomicFlagTestAndSet";
		case spv::Op::OpAtomicFlagClear: return "AtomicFlagClear";
		case spv::Op::OpImageSparseRead: return "ImageSparseRead";
		case spv::Op::OpSizeOf: return "SizeOf";
		case spv::Op::OpTypePipeStorage: return "TypePipeStorage";
		case spv::Op::OpConstantPipeStorage: return "ConstantPipeStorage";
		case spv::Op::OpCreatePipeFromPipeStorage: return "CreatePipeFromPipeStorage";
		case spv::Op::OpGetKernelLocalSizeForSubgroupCount: return "GetKernelLocalSizeForSubgroupCount";
		case spv::Op::OpGetKernelMaxNumSubgroups: return "GetKernelMaxNumSubgroups";
		case spv::Op::OpTypeNamedBarrier: return "TypeNamedBarrier";
		case spv::Op::OpNamedBarrierInitialize: return "NamedBarrierInitialize";
		case spv::Op::OpMemoryNamedBarrier: return "MemoryNamedBarrier";
		case spv::Op::OpModuleProcessed: return "ModuleProcessed";
		case spv::Op::OpExecutionModeId: return "ExecutionModeId";
		case spv::Op::OpDecorateId: return "DecorateId";
		case spv::Op::OpGroupNonUniformElect: return "GroupNonUniformElect";
		case spv::Op::OpGroupNonUniformAll: return "GroupNonUniformAll";
		case spv::Op::OpGroupNonUniformAny: return "GroupNonUniformAny";
		case spv::Op::OpGroupNonUniformAllEqual: return "GroupNonUniformAllEqual";
		case spv::Op::OpGroupNonUniformBroadcast: return "GroupNonUniformBroadcast";
		case spv::Op::OpGroupNonUniformBroadcastFirst: return "GroupNonUniformBroadcastFirst";
		case spv::Op::OpGroupNonUniformBallot: return "GroupNonUniformBallot";
		case spv::Op::OpGroupNonUniformInverseBallot: return "GroupNonUniformInverseBallot";
		case spv::Op::OpGroupNonUniformBallotBitExtract: return "GroupNonUniformBallotBitExtract";
		case spv::Op::OpGroupNonUniformBallotBitCount: return "GroupNonUniformBallotBitCount";
		case spv::Op::OpGroupNonUniformBallotFindLSB: return "GroupNonUniformBallotFindLSB";
		case spv::Op::OpGroupNonUniformBallotFindMSB: return "GroupNonUniformBallotFindMSB";
		case spv::Op::OpGroupNonUniformShuffle: return "GroupNonUniformShuffle";
		case spv::Op::OpGroupNonUniformShuffleXor: return "GroupNonUniformShuffleXor";
		case spv::Op::OpGroupNonUniformShuffleUp: return "GroupNonUniformShuffleUp";
		case spv::Op::OpGroupNonUniformShuffleDown: return "GroupNonUniformShuffleDown";
		case spv::Op::OpGroupNonUniformIAdd: return "GroupNonUniformIAdd";
		case spv::Op::OpGroupNonUniformFAdd: return "GroupNonUniformFAdd";
		case spv::Op::OpGroupNonUniformIMul: return "GroupNonUniformIMul";
		case spv::Op::OpGroupNonUniformFMul: return "GroupNonUniformFMul";
		case spv::Op::OpGroupNonUniformSMin: return "GroupNonUniformSMin";
		case spv::Op::OpGroupNonUniformUMin: return "GroupNonUniformUMin";
		case spv::Op::OpGroupNonUniformFMin: return "GroupNonUniformFMin";
		case spv::Op::OpGroupNonUniformSMax: return "GroupNonUniformSMax";
		case spv::Op::OpGroupNonUniformUMax: return "GroupNonUniformUMax";
		case spv::Op::OpGroupNonUniformFMax: return "GroupNonUniformFMax";
		case spv::Op::OpGroupNonUniformBitwiseAnd: return "GroupNonUniformBitwiseAnd";
		case spv::Op::OpGroupNonUniformBitwiseOr: return "GroupNonUniformBitwiseOr";
		case spv::Op::OpGroupNonUniformBitwiseXor: return "GroupNonUniformBitwiseXor";
		case spv::Op::OpGroupNonUniformLogicalAnd: return "GroupNonUniformLogicalAnd";
		case spv::Op::OpGroupNonUniformLogicalOr: return "GroupNonUniformLogicalOr";
		case spv::Op::OpGroupNonUniformLogicalXor: return "GroupNonUniformLogicalXor";
		case spv::Op::OpGroupNonUniformQuadBroadcast: return "GroupNonUniformQuadBroadcast";
		case spv::Op::OpGroupNonUniformQuadSwap: return "GroupNonUniformQuadSwap";
		case spv::Op::OpSubgroupBallotKHR: return "SubgroupBallotKHR";
		case spv::Op::OpSubgroupFirstInvocationKHR: return "SubgroupFirstInvocationKHR";
		case spv::Op::OpSubgroupAllKHR: return "SubgroupAllKHR";
		case spv::Op::OpSubgroupAnyKHR: return "SubgroupAnyKHR";
		case spv::Op::OpSubgroupAllEqualKHR: return "SubgroupAllEqualKHR";
		case spv::Op::OpSubgroupReadInvocationKHR: return "SubgroupReadInvocationKHR";
		case spv::Op::OpGroupIAddNonUniformAMD: return "GroupIAddNonUniformAMD";
		case spv::Op::OpGroupFAddNonUniformAMD: return "GroupFAddNonUniformAMD";
		case spv::Op::OpGroupFMinNonUniformAMD: return "GroupFMinNonUniformAMD";
		case spv::Op::OpGroupUMinNonUniformAMD: return "GroupUMinNonUniformAMD";
		case spv::Op::OpGroupSMinNonUniformAMD: return "GroupSMinNonUniformAMD";
		case spv::Op::OpGroupFMaxNonUniformAMD: return "GroupFMaxNonUniformAMD";
		case spv::Op::OpGroupUMaxNonUniformAMD: return "GroupUMaxNonUniformAMD";
		case spv::Op::OpGroupSMaxNonUniformAMD: return "GroupSMaxNonUniformAMD";
		case spv::Op::OpFragmentMaskFetchAMD: return "FragmentMaskFetchAMD";
		case spv::Op::OpFragmentFetchAMD: return "FragmentFetchAMD";
		case spv::Op::OpImageSampleFootprintNV: return "ImageSampleFootprintNV";
		case spv::Op::OpGroupNonUniformPartitionNV: return "GroupNonUniformPartitionNV";
		case spv::Op::OpWritePackedPrimitiveIndices4x8NV: return "WritePackedPrimitiveIndices4x8NV";
		case spv::Op::OpReportIntersectionNV: return "ReportIntersectionNV";
		case spv::Op::OpIgnoreIntersectionNV: return "IgnoreIntersectionNV";
		case spv::Op::OpTerminateRayNV: return "TerminateRayNV";
		case spv::Op::OpTraceNV: return "TraceNV";
		case spv::Op::OpTypeAccelerationStructureNV: return "TypeAccelerationStructureNV";
		case spv::Op::OpExecuteCallableNV: return "ExecuteCallableNV";
		case spv::Op::OpSubgroupShuffleINTEL: return "SubgroupShuffleINTEL";
		case spv::Op::OpSubgroupShuffleDownINTEL: return "SubgroupShuffleDownINTEL";
		case spv::Op::OpSubgroupShuffleUpINTEL: return "SubgroupShuffleUpINTEL";
		case spv::Op::OpSubgroupShuffleXorINTEL: return "SubgroupShuffleXorINTEL";
		case spv::Op::OpSubgroupBlockReadINTEL: return "SubgroupBlockReadINTEL";
		case spv::Op::OpSubgroupBlockWriteINTEL: return "SubgroupBlockWriteINTEL";
		case spv::Op::OpSubgroupImageBlockReadINTEL: return "SubgroupImageBlockReadINTEL";
		case spv::Op::OpSubgroupImageBlockWriteINTEL: return "SubgroupImageBlockWriteINTEL";
		case spv::Op::OpDecorateStringGOOGLE: return "DecorateStringGOOGLE";
		case spv::Op::OpMemberDecorateStringGOOGLE: return "MemberDecorateStringGOOGLE";
		case spv::Op::OpMax: return "Max";
		default: return "<UNKNOWN>";
		}
	}
}

namespace sw
{
	volatile int SpirvShader::serialCounter = 1;    // Start at 1, 0 is invalid shader.

	SpirvShader::SpirvShader(InsnStore const &insns)
			: insns{insns}, inputs{MAX_INTERFACE_COMPONENTS},
			  outputs{MAX_INTERFACE_COMPONENTS},
			  serialID{serialCounter++}, modes{}
	{
		// Simplifying assumptions (to be satisfied by earlier transformations)
		// - There is exactly one entrypoint in the module, and it's the one we want
		// - The only input/output OpVariables present are those used by the entrypoint

		printf("SpirvShader::SpirvShader()\n");
		for (auto insn : *this)
		{
			printf("%s\n", opcodeName(insn.opcode()));

			switch (insn.opcode())
			{
			case spv::OpExecutionMode:
				ProcessExecutionMode(insn);
				break;

			case spv::OpDecorate:
			{
				TypeOrObjectID targetId = insn.word(1);
				auto decoration = static_cast<spv::Decoration>(insn.word(2));
				decorations[targetId].Apply(
						decoration,
						insn.wordCount() > 3 ? insn.word(3) : 0);

				if (decoration == spv::DecorationCentroid)
					modes.NeedsCentroid = true;
				break;
			}

			case spv::OpMemberDecorate:
			{
				TypeID targetId = insn.word(1);
				auto memberIndex = insn.word(2);
				auto &d = memberDecorations[targetId];
				if (memberIndex >= d.size())
					d.resize(memberIndex + 1);    // on demand; exact size would require another pass...
				auto decoration = static_cast<spv::Decoration>(insn.word(3));
				d[memberIndex].Apply(
						decoration,
						insn.wordCount() > 4 ? insn.word(4) : 0);

				if (decoration == spv::DecorationCentroid)
					modes.NeedsCentroid = true;
				break;
			}

			case spv::OpDecorationGroup:
				// Nothing to do here. We don't need to record the definition of the group; we'll just have
				// the bundle of decorations float around. If we were to ever walk the decorations directly,
				// we might think about introducing this as a real Object.
				break;

			case spv::OpGroupDecorate:
			{
				auto const &srcDecorations = decorations[insn.word(1)];
				for (auto i = 2u; i < insn.wordCount(); i++)
				{
					// remaining operands are targets to apply the group to.
					decorations[insn.word(i)].Apply(srcDecorations);
				}
				break;
			}

			case spv::OpGroupMemberDecorate:
			{
				auto const &srcDecorations = decorations[insn.word(1)];
				for (auto i = 2u; i < insn.wordCount(); i += 2)
				{
					// remaining operands are pairs of <id>, literal for members to apply to.
					auto &d = memberDecorations[insn.word(i)];
					auto memberIndex = insn.word(i + 1);
					if (memberIndex >= d.size())
						d.resize(memberIndex + 1);    // on demand resize, see above...
					d[memberIndex].Apply(srcDecorations);
				}
				break;
			}

			case spv::OpTypeVoid:
			case spv::OpTypeBool:
			case spv::OpTypeInt:
			case spv::OpTypeFloat:
			case spv::OpTypeVector:
			case spv::OpTypeMatrix:
			case spv::OpTypeImage:
			case spv::OpTypeSampler:
			case spv::OpTypeSampledImage:
			case spv::OpTypeArray:
			case spv::OpTypeRuntimeArray:
			case spv::OpTypeStruct:
			case spv::OpTypePointer:
			case spv::OpTypeFunction:
			{
				TypeID resultId = insn.word(1);
				auto &type = types[resultId];
				type.definition = insn;
				type.sizeInComponents = ComputeTypeSize(insn);

				// A structure is a builtin block if it has a builtin
				// member. All members of such a structure are builtins.
				switch (insn.opcode())
				{
				case spv::OpTypeStruct:
				{
					auto d = memberDecorations.find(resultId);
					if (d != memberDecorations.end())
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
				break;
			}

			case spv::OpVariable:
			{
				TypeID typeId = insn.word(1);
				ObjectID resultId = insn.word(2);
				auto storageClass = static_cast<spv::StorageClass>(insn.word(3));
				if (insn.wordCount() > 4)
					UNIMPLEMENTED("Variable initializers not yet supported");

				auto &object = defs[resultId];
				object.kind = Object::Kind::Variable;
				object.definition = insn;
				object.type = typeId;
				object.pointerBase = insn.word(2);	// base is itself

				ASSERT(getType(typeId).storageClass == storageClass);

				// Register builtins
				switch (storageClass)
				{
					case spv::StorageClassInput:
					case spv::StorageClassOutput:
						ProcessInterfaceVariable(object);
						break;
					case spv::StorageClassUniform:
						object.kind = Object::Kind::PhysicalPointer;
						break;
					default:
						UNIMPLEMENTED("Unhandled storage class %d for OpVariable", (int)storageClass);
				}
				break;
			}

			case spv::OpConstant:
				CreateConstant(insn).constantValue[0] = insn.word(3);
				break;
			case spv::OpConstantFalse:
				CreateConstant(insn).constantValue[0] = 0;		// represent boolean false as zero
				break;
			case spv::OpConstantTrue:
				CreateConstant(insn).constantValue[0] = ~0u;	// represent boolean true as all bits set
				break;
			case spv::OpConstantNull:
			{
				// OpConstantNull forms a constant of arbitrary type, all zeros.
				auto &object = CreateConstant(insn);
				auto &objectTy = getType(object.type);
				for (auto i = 0u; i < objectTy.sizeInComponents; i++)
				{
					object.constantValue[i] = 0;
				}
				break;
			}
			case spv::OpConstantComposite:
			{
				auto &object = CreateConstant(insn);
				auto offset = 0u;
				for (auto i = 0u; i < insn.wordCount() - 3; i++)
				{
					auto &constituent = getObject(insn.word(i + 3));
					auto &constituentTy = getType(constituent.type);
					for (auto j = 0u; j < constituentTy.sizeInComponents; j++)
						object.constantValue[offset++] = constituent.constantValue[j];
				}
				break;
			}

			case spv::OpCapability:
				// Various capabilities will be declared, but none affect our code generation at this point.
			case spv::OpMemoryModel:
				// Memory model does not affect our code generation until we decide to do Vulkan Memory Model support.
			case spv::OpEntryPoint:
			case spv::OpFunction:
			case spv::OpFunctionEnd:
				// Due to preprocessing, the entrypoint and its function provide no value.
				break;
			case spv::OpExtInstImport:
				// We will only support the GLSL 450 extended instruction set, so no point in tracking the ID we assign it.
				// Valid shaders will not attempt to import any other instruction sets.
			case spv::OpName:
			case spv::OpMemberName:
			case spv::OpSource:
			case spv::OpSourceContinued:
			case spv::OpSourceExtension:
				// No semantic impact
				break;

			case spv::OpFunctionParameter:
			case spv::OpFunctionCall:
			case spv::OpSpecConstant:
			case spv::OpSpecConstantComposite:
			case spv::OpSpecConstantFalse:
			case spv::OpSpecConstantOp:
			case spv::OpSpecConstantTrue:
				// These should have all been removed by preprocessing passes. If we see them here,
				// our assumptions are wrong and we will probably generate wrong code.
				UNIMPLEMENTED("These instructions should have already been lowered.");
				break;

			case spv::OpLoad:
			case spv::OpAccessChain:
				// Instructions that yield an ssavalue.
			{
				TypeID typeId = insn.word(1);
				ObjectID resultId = insn.word(2);
				auto &object = defs[resultId];
				object.type = typeId;
				object.kind = Object::Kind::Value;
				object.definition = insn;

				if (insn.opcode() == spv::OpAccessChain)
				{
					// interior ptr has two parts:
					// - logical base ptr, common across all lanes and known at compile time
					// - per-lane offset
					ObjectID baseId = insn.word(3);
					object.pointerBase = getObject(baseId).pointerBase;
				}
				break;
			}

			case spv::OpStore:
			case spv::OpReturn:
				// Don't need to do anything during analysis pass
				break;

			case spv::OpKill:
				modes.ContainsKill = true;
				break;

			default:
				printf("Warning: ignored opcode %s\n", opcodeName(insn.opcode()));
				break;    // This is OK, these passes are intentionally partial
			}
		}
	}

	SpirvShader::Object& SpirvShader::CreateConstant(InsnIterator insn)
	{
		TypeID typeId = insn.word(1);
		ObjectID resultId = insn.word(2);
		auto &object = defs[resultId];
		auto &objectTy = getType(typeId);
		object.type = typeId;
		object.kind = Object::Kind::Constant;
		object.definition = insn;
		object.constantValue = std::unique_ptr<uint32_t[]>(new uint32_t[objectTy.sizeInComponents]);
		return object;
	}

	void SpirvShader::ProcessInterfaceVariable(Object &object)
	{
		auto &objectTy = getType(object.type);
		assert(objectTy.storageClass == spv::StorageClassInput || objectTy.storageClass == spv::StorageClassOutput);

		assert(objectTy.definition.opcode() == spv::OpTypePointer);
		auto pointeeTy = getType(objectTy.element);

		auto &builtinInterface = (objectTy.storageClass == spv::StorageClassInput) ? inputBuiltins : outputBuiltins;
		auto &userDefinedInterface = (objectTy.storageClass == spv::StorageClassInput) ? inputs : outputs;

		assert(object.definition.opcode() == spv::OpVariable);
		ObjectID resultId = object.definition.word(2);

		if (objectTy.isBuiltInBlock)
		{
			// walk the builtin block, registering each of its members separately.
			auto m = memberDecorations.find(objectTy.element);
			assert(m != memberDecorations.end());        // otherwise we wouldn't have marked the type chain
			auto &structType = pointeeTy.definition;
			auto offset = 0u;
			auto word = 2u;
			for (auto &member : m->second)
			{
				auto &memberType = getType(structType.word(word));

				if (member.HasBuiltIn)
				{
					builtinInterface[member.BuiltIn] = {resultId, offset, memberType.sizeInComponents};
				}

				offset += memberType.sizeInComponents;
				++word;
			}
			return;
		}


		auto d = decorations.find(resultId);
		if (d != decorations.end() && d->second.HasBuiltIn)
		{
			builtinInterface[d->second.BuiltIn] = {resultId, 0, pointeeTy.sizeInComponents};
		}
		else
		{
			object.kind = Object::Kind::InterfaceVariable;
			VisitInterface(resultId,
						   [&userDefinedInterface](Decorations const &d, AttribType type) {
							   // Populate a single scalar slot in the interface from a collection of decorations and the intended component type.
							   auto scalarSlot = (d.Location << 2) | d.Component;
							   assert(scalarSlot >= 0 &&
									  scalarSlot < static_cast<int32_t>(userDefinedInterface.size()));

							   auto &slot = userDefinedInterface[scalarSlot];
							   slot.Type = type;
							   slot.Flat = d.Flat;
							   slot.NoPerspective = d.NoPerspective;
							   slot.Centroid = d.Centroid;
						   });
		}
	}

	void SpirvShader::ProcessExecutionMode(InsnIterator insn)
	{
		auto mode = static_cast<spv::ExecutionMode>(insn.word(2));
		switch (mode)
		{
		case spv::ExecutionModeEarlyFragmentTests:
			modes.EarlyFragmentTests = true;
			break;
		case spv::ExecutionModeDepthReplacing:
			modes.DepthReplacing = true;
			break;
		case spv::ExecutionModeDepthGreater:
			modes.DepthGreater = true;
			break;
		case spv::ExecutionModeDepthLess:
			modes.DepthLess = true;
			break;
		case spv::ExecutionModeDepthUnchanged:
			modes.DepthUnchanged = true;
			break;
		case spv::ExecutionModeLocalSize:
			modes.LocalSizeX = insn.word(3);
			modes.LocalSizeZ = insn.word(5);
			modes.LocalSizeY = insn.word(4);
			break;
		case spv::ExecutionModeOriginUpperLeft:
			// This is always the case for a Vulkan shader. Do nothing.
			break;
		default:
			UNIMPLEMENTED("No other execution modes are permitted");
		}
	}

	uint32_t SpirvShader::ComputeTypeSize(sw::SpirvShader::InsnIterator insn)
	{
		// Types are always built from the bottom up (with the exception of forward ptrs, which
		// don't appear in Vulkan shaders. Therefore, we can always assume our component parts have
		// already been described (and so their sizes determined)
		switch (insn.opcode())
		{
		case spv::OpTypeVoid:
		case spv::OpTypeSampler:
		case spv::OpTypeImage:
		case spv::OpTypeSampledImage:
		case spv::OpTypeFunction:
		case spv::OpTypeRuntimeArray:
			// Objects that don't consume any space.
			// Descriptor-backed objects currently only need exist at compile-time.
			// Runtime arrays don't appear in places where their size would be interesting
			return 0;

		case spv::OpTypeBool:
		case spv::OpTypeFloat:
		case spv::OpTypeInt:
			// All the fundamental types are 1 component. If we ever add support for 8/16/64-bit components,
			// we might need to change this, but only 32 bit components are required for Vulkan 1.1.
			return 1;

		case spv::OpTypeVector:
		case spv::OpTypeMatrix:
			// Vectors and matrices both consume element count * element size.
			return getType(insn.word(2)).sizeInComponents * insn.word(3);

		case spv::OpTypeArray:
		{
			// Element count * element size. Array sizes come from constant ids.
			auto arraySize = GetConstantInt(insn.word(3));
			return getType(insn.word(2)).sizeInComponents * arraySize;
		}

		case spv::OpTypeStruct:
		{
			uint32_t size = 0;
			for (uint32_t i = 2u; i < insn.wordCount(); i++)
			{
				size += getType(insn.word(i)).sizeInComponents;
			}
			return size;
		}

		case spv::OpTypePointer:
			// Runtime representation of a pointer is a per-lane index.
			// Note: clients are expected to look through the pointer if they want the pointee size instead.
			return 1;

		default:
			// Some other random insn.
			UNIMPLEMENTED("Only types are supported");
		}
	}

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

	SpirvShader::IntL SpirvShader::WalkAccessChain(ObjectID id, uint32_t numIndexes, uint32_t const *indexIds, SpirvRoutine *routine) const
	{
		// TODO: think about explicit layout (UBO/SSBO) storage classes
		// TODO: avoid doing per-lane work in some cases if we can?

		int constantOffset = 0;
		IntL dynamicOffset = IntL(0);
		auto &baseObject = getObject(id);
		TypeID typeId = getType(baseObject.type).element;

		// The <base> operand is an intermediate value itself, ie produced by a previous OpAccessChain.
		// Start with its offset and build from there.
		if (baseObject.kind == Object::Kind::Value)
			dynamicOffset += As<IntL>(routine->getIntermediate(id)[0]);

		for (auto i = 0u; i < numIndexes; i++)
		{
			auto & type = getType(typeId);
			switch (type.definition.opcode())
			{
			case spv::OpTypeStruct:
			{
				int memberIndex = GetConstantInt(indexIds[i]);
				int offsetIntoStruct = 0;
				for (auto j = 0; j < memberIndex; j++) {
					auto memberType = type.definition.word(2u + j);
					offsetIntoStruct += getType(memberType).sizeInComponents;
				}
				constantOffset += offsetIntoStruct;
				typeId = type.definition.word(2u + memberIndex);
				break;
			}

			case spv::OpTypeVector:
			case spv::OpTypeMatrix:
			case spv::OpTypeArray:
			case spv::OpTypeRuntimeArray:
			{
				// TODO: Bounds checking.
				auto stride = getType(type.element).sizeInComponents;
				auto & obj = getObject(indexIds[i]);
				if (obj.kind == Object::Kind::Constant)
					constantOffset += stride * GetConstantInt(indexIds[i]);
				else
					dynamicOffset += IntL(stride) * As<IntL>(routine->getIntermediate(indexIds[i])[0]);
				typeId = type.element;
				break;
			}

			case spv::OpTypePointer:
				typeId = type.element;
				break;

			default:
				UNIMPLEMENTED("Unexpected type '%s' in WalkAccessChain", opcodeName(type.definition.opcode()));
			}
		}

		return dynamicOffset + IntL(constantOffset);
	}

	void SpirvShader::Decorations::Apply(spv::Decoration decoration, uint32_t arg)
	{
		switch (decoration)
		{
		case spv::DecorationLocation:
			HasLocation = true;
			Location = static_cast<int32_t>(arg);
			break;
		case spv::DecorationComponent:
			HasComponent = true;
			Component = arg;
			break;
		case spv::DecorationBuiltIn:
			HasBuiltIn = true;
			BuiltIn = static_cast<spv::BuiltIn>(arg);
			break;
		case spv::DecorationFlat:
			Flat = true;
			break;
		case spv::DecorationNoPerspective:
			NoPerspective = true;
			break;
		case spv::DecorationCentroid:
			Centroid = true;
			break;
		case spv::DecorationBlock:
			Block = true;
			break;
		case spv::DecorationBufferBlock:
			BufferBlock = true;
			break;
		case spv::DecorationDescriptorSet:
			DescriptorSet = arg;
			break;
		case spv::DecorationBinding:
			Binding = arg;
			break;
		default:
			// Intentionally partial, there are many decorations we just don't care about.
			break;
		}
	}

	void SpirvShader::Decorations::Apply(const sw::SpirvShader::Decorations &src)
	{
		// Apply a decoration group to this set of decorations
		if (src.HasBuiltIn)
		{
			HasBuiltIn = true;
			BuiltIn = src.BuiltIn;
		}

		if (src.HasLocation)
		{
			HasLocation = true;
			Location = src.Location;
		}

		if (src.HasComponent)
		{
			HasComponent = true;
			Component = src.Component;
		}

		if (src.DescriptorSet >= 0)
		{
			ASSERT(DescriptorSet < 0 || DescriptorSet == src.DescriptorSet);
			DescriptorSet = src.DescriptorSet;
		}

		if (src.Binding >= 0)
		{
			ASSERT(Binding < 0 || Binding == src.Binding);
			Binding = src.Binding;
		}

		Flat |= src.Flat;
		NoPerspective |= src.NoPerspective;
		Centroid |= src.Centroid;
		Block |= src.Block;
		BufferBlock |= src.BufferBlock;
	}

	void SpirvShader::ApplyDecorationsForId(Decorations *d, TypeOrObjectID id) const
	{
		auto it = decorations.find(id);
		if (it != decorations.end())
			d->Apply(it->second);
	}

	void SpirvShader::ApplyDecorationsForIdMember(Decorations *d, TypeID id, uint32_t member) const
	{
		auto it = memberDecorations.find(id);
		if (it != memberDecorations.end() && member < it->second.size())
		{
			d->Apply(it->second[member]);
		}
	}

	uint32_t SpirvShader::GetConstantInt(ObjectID id) const
	{
		// Slightly hackish access to constants very early in translation.
		// General consumption of constants by other instructions should
		// probably be just lowered to Reactor.

		// TODO: not encountered yet since we only use this for array sizes etc,
		// but is possible to construct integer constant 0 via OpConstantNull.
		auto insn = getObject(id).definition;
		assert(insn.opcode() == spv::OpConstant);
		assert(getType(insn.word(1)).definition.opcode() == spv::OpTypeInt);
		return insn.word(3);
	}

	// emit-time

	void SpirvShader::emitProlog(SpirvRoutine *routine) const
	{
		printf("SpirvShader::emitProlog()\n");
		RR_LOG("SpirvShader::emitProlog()");

		for (auto insn : *this)
		{
			switch (insn.opcode())
			{
			case spv::OpVariable:
			{
				ObjectID resultId = insn.word(2);
				auto &object = getObject(resultId);
				auto &objectTy = getType(object.type);
				auto &pointeeTy = getType(objectTy.element);
				// TODO: what to do about zero-slot objects?
				if (pointeeTy.sizeInComponents > 0)
				{
					routine->createLvalue(resultId, pointeeTy.sizeInComponents);
				}
				break;
			}
			default:
				// Nothing else produces interface variables, so can all be safely ignored.
				break;
			}
		}
	}

	void SpirvShader::emit(SpirvRoutine *routine, vk::PipelineLayout* pipelineLayout) const
	{
		printf("SpirvShader::emit()\n");
		RR_LOG("SpirvShader::emit()");

		for (auto insn : *this)
		{
			printf("%s\n", opcodeName(insn.opcode()));
			// RR_WATCH(opcodeName(insn.opcode()));

			switch (insn.opcode())
			{
			case spv::OpTypeVoid:
			case spv::OpTypeInt:
			case spv::OpTypeFloat:
			case spv::OpTypeBool:
			case spv::OpTypeVector:
			case spv::OpTypeArray:
			case spv::OpTypeRuntimeArray:
			case spv::OpTypeMatrix:
			case spv::OpTypeStruct:
			case spv::OpTypePointer:
			case spv::OpTypeFunction:
			case spv::OpExecutionMode:
			case spv::OpMemoryModel:
			case spv::OpFunction:
			case spv::OpFunctionEnd:
			case spv::OpConstant:
			case spv::OpConstantNull:
			case spv::OpConstantTrue:
			case spv::OpConstantFalse:
			case spv::OpConstantComposite:
			case spv::OpExtension:
			case spv::OpCapability:
			case spv::OpEntryPoint:
			case spv::OpExtInstImport:
			case spv::OpDecorate:
			case spv::OpMemberDecorate:
			case spv::OpGroupDecorate:
			case spv::OpGroupMemberDecorate:
			case spv::OpDecorationGroup:
			case spv::OpName:
			case spv::OpMemberName:
			case spv::OpSource:
			case spv::OpSourceContinued:
			case spv::OpSourceExtension:
				// Nothing to do at emit time. These are either fully handled at analysis time,
				// or don't require any work at all.
				break;

			case spv::OpVariable:
			{
				ObjectID resultId = insn.word(2);
				auto &object = getObject(resultId);
				auto &objectTy = getType(object.type);
				switch (object.kind)
				{
				case Object::Kind::InterfaceVariable:
					if (objectTy.storageClass == spv::StorageClassInput)
					{
						auto &dst = routine->getValue(resultId);
						int offset = 0;
						VisitInterface(resultId,
									[&](Decorations const &d, AttribType type) {
										auto scalarSlot = d.Location << 2 | d.Component;
										dst[offset++] = routine->inputs[scalarSlot];
									});
					}
					break;
				case Object::Kind::PhysicalPointer:
				{
					Decorations d{};
					ApplyDecorationsForId(&d, resultId);
					ASSERT(d.DescriptorSet >= 0);
					ASSERT(d.Binding >= 0);

					auto setLayout = pipelineLayout->getDescriptorSetLayout(d.DescriptorSet);
					size_t bindingOffset = 0;
					setLayout->getBindingInfo(d.Binding, &bindingOffset);
					// each set has a header of a VkDescriptorSetLayout.
					// TODO: Clean this up.
					bindingOffset += sizeof(VkDescriptorSetLayout);

					Pointer<Byte> set = routine->descriptorSets[d.DescriptorSet]; // DescriptorSet*
					Pointer<Byte> binding = Pointer<Byte>(set + bindingOffset); // VkDescriptorBufferInfo*
					Pointer<Byte> buffer = *Pointer<Pointer<Byte>>(binding + OFFSET(VkDescriptorBufferInfo, buffer)); // vk::Buffer*
					Pointer<Byte> data = *Pointer<Pointer<Byte>>(buffer + vk::Buffer::DataOffset); // void*
					Int offset = *Pointer<Int>(binding + OFFSET(VkDescriptorBufferInfo, offset));
					Pointer<Byte> address = data + offset;
					routine->physicalPointers[resultId] = address;
					RR_WATCH(d.DescriptorSet, d.Binding, set, binding, data, offset, address, resultId.value());
					break;
				}
				default:
					// UNIMPLEMENTED("Object kind %d not yet implemented", (int)object.kind);
					break;
				}
				break;
			}
			case spv::OpLoad:
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

				if (pointerBaseTy.storageClass == spv::StorageClassImage)
				{
					UNIMPLEMENTED("StorageClassImage load not yet implemented");
				}

				Pointer<Float> ptrBase;
				if (pointerBase.kind == Object::Kind::PhysicalPointer)
				{
					ptrBase = routine->getPhysicalPointer(pointer.pointerBase);
				}
				else
				{
					ptrBase = &routine->getValue(pointer.pointerBase)[0];
				}

				bool isUniform =
					pointerBaseTy.storageClass == spv::StorageClassUniform ||
					pointerBaseTy.storageClass == spv::StorageClassUniformConstant;

				auto &dst = routine->getIntermediate(objectId);

				if (pointer.kind == Object::Kind::Value)
				{
					auto offsets = As<IntL>(routine->getIntermediate(pointerId)[0]);
					for (auto i = 0u; i < objectTy.sizeInComponents; i++)
					{
						// i wish i had a Float,Float,Float,Float constructor here..
						// RR_WATCH(i, offsets, ptrBase, pointerBase.kind == Object::Kind::PhysicalPointer);
						FloatL v;
						for (int j = 0; j < NumLanes; j++)
						{
							Int offset = Int(i) + Extract(offsets, j);
							if (!isUniform) { offset = offset * NumLanes + j; }
							v = Insert(v, ptrBase[offset], j);
						}
						RR_WATCH(objectId.value(), offsets, As<IntL>(v));
						dst.emplace(i, v);
					}
				}
				else if (isUniform)
				{
					// no divergent offsets to worry about
					for (auto i = 0u; i < objectTy.sizeInComponents; i++)
					{
						dst.emplace(i, RValue<FloatL>(ptrBase[i]));
					}
				}
				else
				{
					// no divergent offsets to worry about
					Pointer<FloatL> ptrBaseFloatL = As<Pointer<FloatL>>(ptrBase);
					for (auto i = 0u; i < objectTy.sizeInComponents; i++)
					{
						dst.emplace(i, ptrBaseFloatL[i]);
						// RR_WATCH(i, ptrBase[i], pointerBase.kind == Object::Kind::PhysicalPointer);
					}
				}
				break;
			}
			case spv::OpAccessChain:
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
					pointerBaseTy.storageClass == spv::StorageClassUniformConstant)
				{
					UNIMPLEMENTED("Descriptor-backed OpAccessChain not yet implemented");
				}
				auto &dst = routine->getIntermediate(objectId);
				dst.emplace(0, As<FloatL>(WalkAccessChain(baseId, insn.wordCount() - 4, insn.wordPointer(4), routine)));
				break;
			}
			case spv::OpStore:
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
					pointerBaseTy.storageClass == spv::StorageClassUniformConstant)
				{
					UNIMPLEMENTED("Descriptor-backed store not yet implemented");
				}

				Pointer<Float> ptrBase;
				if (pointerBase.kind == Object::Kind::PhysicalPointer)
				{
					ptrBase = As<Pointer<Float>>(routine->getPhysicalPointer(pointer.pointerBase));
				}
				else
				{
					ptrBase = &routine->getValue(pointer.pointerBase)[0];
				}

				bool isUniform = pointerBaseTy.storageClass == spv::StorageClassUniform;

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
								Int offset = Int(i) + Extract(offsets, j);
								if (!isUniform) { offset = offset * NumLanes + j; }
								ptrBase[offset] = RValue<Float>(src[i]);
							}
						}
					}
					else
					{
						// no divergent offsets
						Pointer<FloatL> dst = As<Pointer<FloatL>>(ptrBase);
						for (auto i = 0u; i < elementTy.sizeInComponents; i++)
						{
							dst[i] = RValue<FloatL>(src[i]);
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
								Int offset = Int(i) + Extract(offsets, j);
								if (!isUniform) { offset = offset * NumLanes + j; }
								ptrBase[offset] = Extract(src[i], j);
							}
						}
					}
					else if (isUniform)
					{
						// no divergent offsets
						Pointer<FloatL> dst = As<Pointer<FloatL>>(ptrBase);
						for (auto i = 0u; i < elementTy.sizeInComponents; i++)
						{
							dst[i] = FloatL(src[i]);
						}
					}
					else
					{
						// no divergent offsets
						Pointer<FloatL> dst = As<Pointer<FloatL>>(ptrBase);
						for (auto i = 0u; i < elementTy.sizeInComponents; i++)
						{
							dst[i] = src[i];
						}
					}
				}
				break;
			}
			default:
				printf("emit: ignoring opcode %s\n", opcodeName(insn.opcode()));
				break;
			}
		}
	}

	void SpirvShader::emitEpilog(SpirvRoutine *routine) const
	{
		for (auto insn : *this)
		{
			switch (insn.opcode())
			{
			case spv::OpVariable:
			{
				ObjectID resultId = insn.word(2);
				auto &object = getObject(resultId);
				auto &objectTy = getType(object.type);
				if (object.kind == Object::Kind::InterfaceVariable && objectTy.storageClass == spv::StorageClassOutput)
				{
					auto &dst = routine->getValue(resultId);
					int offset = 0;
					VisitInterface(resultId,
								   [&](Decorations const &d, AttribType type) {
									   auto scalarSlot = d.Location << 2 | d.Component;
									   routine->outputs[scalarSlot] = dst[offset++];
								   });
				}
				break;
			}
			default:
				break;
			}
		}
	}
}
