LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CLANG := true
LOCAL_MODULE := libswiftshader_common
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	Common/CPUID.cpp \
	Common/Configurator.cpp \
	Common/DebugAndroid.cpp \
	Common/GrallocAndroid.cpp \
	Common/Half.cpp \
	Common/Math.cpp \
	Common/Memory.cpp \
	Common/Resource.cpp \
	Common/Socket.cpp \
	Common/Thread.cpp \
	Common/Timer.cpp

LOCAL_SRC_FILES += \
	Main/Config.cpp \
	Main/FrameBuffer.cpp \
	Main/FrameBufferAndroid.cpp \
	Main/Logo.cpp \
	Main/Register.cpp \
	Main/SwiftConfig.cpp \
	Main/crc.cpp \
	Main/serialvalid.cpp \

LOCAL_SRC_FILES += \
	Reactor/Nucleus.cpp \
	Reactor/Routine.cpp \
	Reactor/RoutineManager.cpp

LOCAL_SRC_FILES += \
	Renderer/Blitter.cpp \
	Renderer/Clipper.cpp \
	Renderer/Color.cpp \
	Renderer/Context.cpp \
	Renderer/Matrix.cpp \
	Renderer/PixelProcessor.cpp \
	Renderer/Plane.cpp \
	Renderer/Point.cpp \
	Renderer/QuadRasterizer.cpp \
	Renderer/Rasterizer.cpp \
	Renderer/Renderer.cpp \
	Renderer/Sampler.cpp \
	Renderer/SetupProcessor.cpp \
	Renderer/Surface.cpp \
	Renderer/TextureStage.cpp \
	Renderer/Vector.cpp \
	Renderer/VertexProcessor.cpp \

LOCAL_SRC_FILES += \
	Shader/Constants.cpp \
	Shader/PixelRoutine.cpp \
	Shader/PixelShader.cpp \
	Shader/SamplerCore.cpp \
	Shader/SetupRoutine.cpp \
	Shader/Shader.cpp \
	Shader/ShaderCore.cpp \
	Shader/VertexPipeline.cpp \
	Shader/VertexProgram.cpp \
	Shader/VertexRoutine.cpp \
	Shader/VertexShader.cpp \

LOCAL_SRC_FILES += \
	OpenGL/common/AndroidCommon.cpp \
	OpenGL/common/Image.cpp \
	OpenGL/common/NameSpace.cpp \
	OpenGL/common/Object.cpp \
	OpenGL/common/MatrixStack.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/Analysis/AliasAnalysis.cpp \
	LLVM/lib/Analysis/AliasSetTracker.cpp \
	LLVM/lib/Analysis/BasicAliasAnalysis.cpp \
	LLVM/lib/Analysis/BranchProbabilityInfo.cpp \
	LLVM/lib/Analysis/CaptureTracking.cpp \
	LLVM/lib/Analysis/ConstantFolding.cpp \
	LLVM/lib/Analysis/DebugInfo.cpp \
	LLVM/lib/Analysis/DIBuilder.cpp \
	LLVM/lib/Analysis/InstructionSimplify.cpp \
	LLVM/lib/Analysis/IVUsers.cpp \
	LLVM/lib/Analysis/Loads.cpp \
	LLVM/lib/Analysis/LoopInfo.cpp \
	LLVM/lib/Analysis/LoopPass.cpp \
	LLVM/lib/Analysis/MemoryBuiltins.cpp \
	LLVM/lib/Analysis/MemoryDependenceAnalysis.cpp \
	LLVM/lib/Analysis/NoAliasAnalysis.cpp \
	LLVM/lib/Analysis/PathNumbering.cpp \
	LLVM/lib/Analysis/PHITransAddr.cpp \
	LLVM/lib/Analysis/ProfileInfo.cpp \
	LLVM/lib/Analysis/ScalarEvolution.cpp \
	LLVM/lib/Analysis/ScalarEvolutionExpander.cpp \
	LLVM/lib/Analysis/ScalarEvolutionNormalization.cpp \
	LLVM/lib/Analysis/TypeBasedAliasAnalysis.cpp \
	LLVM/lib/Analysis/ValueTracking.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/CodeGen/SelectionDAG/DAGCombiner.cpp \
	LLVM/lib/CodeGen/SelectionDAG/FastISel.cpp \
	LLVM/lib/CodeGen/SelectionDAG/FunctionLoweringInfo.cpp \
	LLVM/lib/CodeGen/SelectionDAG/InstrEmitter.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeDAG.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeFloatTypes.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeIntegerTypes.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeTypes.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeTypesGeneric.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeVectorOps.cpp \
	LLVM/lib/CodeGen/SelectionDAG/LegalizeVectorTypes.cpp \
	LLVM/lib/CodeGen/SelectionDAG/ScheduleDAGFast.cpp \
	LLVM/lib/CodeGen/SelectionDAG/ScheduleDAGList.cpp \
	LLVM/lib/CodeGen/SelectionDAG/ScheduleDAGRRList.cpp \
	LLVM/lib/CodeGen/SelectionDAG/ScheduleDAGSDNodes.cpp \
	LLVM/lib/CodeGen/SelectionDAG/SelectionDAG.cpp \
	LLVM/lib/CodeGen/SelectionDAG/SelectionDAGBuilder.cpp \
	LLVM/lib/CodeGen/SelectionDAG/SelectionDAGISel.cpp \
	LLVM/lib/CodeGen/SelectionDAG/SelectionDAGPrinter.cpp \
	LLVM/lib/CodeGen/SelectionDAG/TargetLowering.cpp \
	LLVM/lib/CodeGen/SelectionDAG/TargetSelectionDAGInfo.cpp \
	LLVM/lib/CodeGen/AggressiveAntiDepBreaker.cpp \
	LLVM/lib/CodeGen/AllocationOrder.cpp \
	LLVM/lib/CodeGen/Analysis.cpp \
	LLVM/lib/CodeGen/BranchFolding.cpp \
	LLVM/lib/CodeGen/CalcSpillWeights.cpp \
	LLVM/lib/CodeGen/CallingConvLower.cpp \
	LLVM/lib/CodeGen/CodeGen.cpp \
	LLVM/lib/CodeGen/CodePlacementOpt.cpp \
	LLVM/lib/CodeGen/CriticalAntiDepBreaker.cpp \
	LLVM/lib/CodeGen/DeadMachineInstructionElim.cpp \
	LLVM/lib/CodeGen/DwarfEHPrepare.cpp \
	LLVM/lib/CodeGen/EdgeBundles.cpp \
	LLVM/lib/CodeGen/ELFCodeEmitter.cpp \
	LLVM/lib/CodeGen/ELFWriter.cpp \
	LLVM/lib/CodeGen/ExecutionDepsFix.cpp \
	LLVM/lib/CodeGen/ExpandISelPseudos.cpp \
	LLVM/lib/CodeGen/ExpandPostRAPseudos.cpp \
	LLVM/lib/CodeGen/GCMetadata.cpp \
	LLVM/lib/CodeGen/GCStrategy.cpp \
	LLVM/lib/CodeGen/IfConversion.cpp \
	LLVM/lib/CodeGen/InlineSpiller.cpp \
	LLVM/lib/CodeGen/InterferenceCache.cpp \
	LLVM/lib/CodeGen/IntrinsicLowering.cpp \
	LLVM/lib/CodeGen/LatencyPriorityQueue.cpp \
	LLVM/lib/CodeGen/LexicalScopes.cpp \
	LLVM/lib/CodeGen/LiveDebugVariables.cpp \
	LLVM/lib/CodeGen/LiveIntervalAnalysis.cpp \
	LLVM/lib/CodeGen/LiveInterval.cpp \
	LLVM/lib/CodeGen/LiveIntervalUnion.cpp \
	LLVM/lib/CodeGen/LiveRangeCalc.cpp \
	LLVM/lib/CodeGen/LiveRangeEdit.cpp \
	LLVM/lib/CodeGen/LiveStackAnalysis.cpp \
	LLVM/lib/CodeGen/LiveVariables.cpp \
	LLVM/lib/CodeGen/LLVMTargetMachine.cpp \
	LLVM/lib/CodeGen/LocalStackSlotAllocation.cpp \
	LLVM/lib/CodeGen/MachineBasicBlock.cpp \
	LLVM/lib/CodeGen/MachineBlockFrequencyInfo.cpp \
	LLVM/lib/CodeGen/MachineBranchProbabilityInfo.cpp \
	LLVM/lib/CodeGen/MachineCSE.cpp \
	LLVM/lib/CodeGen/MachineDominators.cpp \
	LLVM/lib/CodeGen/MachineFunctionAnalysis.cpp \
	LLVM/lib/CodeGen/MachineFunction.cpp \
	LLVM/lib/CodeGen/MachineFunctionPass.cpp \
	LLVM/lib/CodeGen/MachineFunctionPrinterPass.cpp \
	LLVM/lib/CodeGen/MachineInstr.cpp \
	LLVM/lib/CodeGen/MachineLICM.cpp \
	LLVM/lib/CodeGen/MachineLoopInfo.cpp \
	LLVM/lib/CodeGen/MachineLoopRanges.cpp \
	LLVM/lib/CodeGen/MachineModuleInfo.cpp \
	LLVM/lib/CodeGen/MachineModuleInfoImpls.cpp \
	LLVM/lib/CodeGen/MachinePassRegistry.cpp \
	LLVM/lib/CodeGen/MachineRegisterInfo.cpp \
	LLVM/lib/CodeGen/MachineSink.cpp \
	LLVM/lib/CodeGen/MachineSSAUpdater.cpp \
	LLVM/lib/CodeGen/MachineVerifier.cpp \
	LLVM/lib/CodeGen/ObjectCodeEmitter.cpp \
	LLVM/lib/CodeGen/OcamlGC.cpp \
	LLVM/lib/CodeGen/OptimizePHIs.cpp \
	LLVM/lib/CodeGen/Passes.cpp \
	LLVM/lib/CodeGen/PeepholeOptimizer.cpp \
	LLVM/lib/CodeGen/PHIElimination.cpp \
	LLVM/lib/CodeGen/PHIEliminationUtils.cpp \
	LLVM/lib/CodeGen/PostRASchedulerList.cpp \
	LLVM/lib/CodeGen/ProcessImplicitDefs.cpp \
	LLVM/lib/CodeGen/PrologEpilogInserter.cpp \
	LLVM/lib/CodeGen/PseudoSourceValue.cpp \
	LLVM/lib/CodeGen/RegAllocBasic.cpp \
	LLVM/lib/CodeGen/RegAllocFast.cpp \
	LLVM/lib/CodeGen/RegAllocGreedy.cpp \
	LLVM/lib/CodeGen/RegAllocLinearScan.cpp \
	LLVM/lib/CodeGen/RegAllocPBQP.cpp \
	LLVM/lib/CodeGen/RegisterClassInfo.cpp \
	LLVM/lib/CodeGen/RegisterCoalescer.cpp \
	LLVM/lib/CodeGen/RegisterScavenging.cpp \
	LLVM/lib/CodeGen/RenderMachineFunction.cpp \
	LLVM/lib/CodeGen/ScheduleDAG.cpp \
	LLVM/lib/CodeGen/ScheduleDAGEmit.cpp \
	LLVM/lib/CodeGen/ScheduleDAGInstrs.cpp \
	LLVM/lib/CodeGen/ScheduleDAGPrinter.cpp \
	LLVM/lib/CodeGen/ScoreboardHazardRecognizer.cpp \
	LLVM/lib/CodeGen/ShadowStackGC.cpp \
	LLVM/lib/CodeGen/ShrinkWrapping.cpp \
	LLVM/lib/CodeGen/SjLjEHPrepare.cpp \
	LLVM/lib/CodeGen/SlotIndexes.cpp \
	LLVM/lib/CodeGen/Spiller.cpp \
	LLVM/lib/CodeGen/SpillPlacement.cpp \
	LLVM/lib/CodeGen/SplitKit.cpp \
	LLVM/lib/CodeGen/Splitter.cpp \
	LLVM/lib/CodeGen/StackProtector.cpp \
	LLVM/lib/CodeGen/StackSlotColoring.cpp \
	LLVM/lib/CodeGen/StrongPHIElimination.cpp \
	LLVM/lib/CodeGen/TailDuplication.cpp \
	LLVM/lib/CodeGen/TargetInstrInfoImpl.cpp \
	LLVM/lib/CodeGen/TargetLoweringObjectFileImpl.cpp \
	LLVM/lib/CodeGen/TwoAddressInstructionPass.cpp \
	LLVM/lib/CodeGen/UnreachableBlockElim.cpp \
	LLVM/lib/CodeGen/VirtRegMap.cpp \
	LLVM/lib/CodeGen/VirtRegRewriter.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/ExecutionEngine/JIT/Intercept.cpp \
	LLVM/lib/ExecutionEngine/JIT/JIT.cpp \
	LLVM/lib/ExecutionEngine/JIT/JITDebugRegisterer.cpp \
	LLVM/lib/ExecutionEngine/JIT/JITDwarfEmitter.cpp \
	LLVM/lib/ExecutionEngine/JIT/JITEmitter.cpp \
	LLVM/lib/ExecutionEngine/JIT/JITMemoryManager.cpp \
	LLVM/lib/ExecutionEngine/JIT/OProfileJITEventListener.cpp \
	LLVM/lib/ExecutionEngine/ExecutionEngine.cpp \
	LLVM/lib/ExecutionEngine/TargetSelect.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/MC/ELFObjectWriter.cpp \
	LLVM/lib/MC/MachObjectWriter.cpp \
	LLVM/lib/MC/MCAsmBackend.cpp \
	LLVM/lib/MC/MCAsmInfoCOFF.cpp \
	LLVM/lib/MC/MCAsmInfo.cpp \
	LLVM/lib/MC/MCAsmInfoDarwin.cpp \
	LLVM/lib/MC/MCAsmStreamer.cpp \
	LLVM/lib/MC/MCAssembler.cpp \
	LLVM/lib/MC/MCAtom.cpp \
	LLVM/lib/MC/MCCodeEmitter.cpp \
	LLVM/lib/MC/MCCodeGenInfo.cpp \
	LLVM/lib/MC/MCContext.cpp \
	LLVM/lib/MC/MCDisassembler.cpp \
	LLVM/lib/MC/MCDwarf.cpp \
	LLVM/lib/MC/MCELF.cpp \
	LLVM/lib/MC/MCELFObjectTargetWriter.cpp \
	LLVM/lib/MC/MCELFStreamer.cpp \
	LLVM/lib/MC/MCExpr.cpp \
	LLVM/lib/MC/MCInst.cpp \
	LLVM/lib/MC/MCInstPrinter.cpp \
	LLVM/lib/MC/MCInstrAnalysis.cpp \
	LLVM/lib/MC/MCLabel.cpp \
	LLVM/lib/MC/MCLoggingStreamer.cpp \
	LLVM/lib/MC/MCMachObjectTargetWriter.cpp \
	LLVM/lib/MC/MCMachOStreamer.cpp \
	LLVM/lib/MC/MCModule.cpp \
	LLVM/lib/MC/MCNullStreamer.cpp \
	LLVM/lib/MC/MCObjectFileInfo.cpp \
	LLVM/lib/MC/MCObjectStreamer.cpp \
	LLVM/lib/MC/MCObjectWriter.cpp \
	LLVM/lib/MC/MCPureStreamer.cpp \
	LLVM/lib/MC/MCSectionCOFF.cpp \
	LLVM/lib/MC/MCSection.cpp \
	LLVM/lib/MC/MCSectionELF.cpp \
	LLVM/lib/MC/MCSectionMachO.cpp \
	LLVM/lib/MC/MCStreamer.cpp \
	LLVM/lib/MC/MCSubtargetInfo.cpp \
	LLVM/lib/MC/MCSymbol.cpp \
	LLVM/lib/MC/MCTargetAsmLexer.cpp \
	LLVM/lib/MC/MCValue.cpp \
	LLVM/lib/MC/MCWin64EH.cpp \
	LLVM/lib/MC/SubtargetFeature.cpp \
	LLVM/lib/MC/WinCOFFObjectWriter.cpp \
	LLVM/lib/MC/WinCOFFStreamer.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/Support/Allocator.cpp \
	LLVM/lib/Support/APFloat.cpp \
	LLVM/lib/Support/APInt.cpp \
	LLVM/lib/Support/APSInt.cpp \
	LLVM/lib/Support/Atomic.cpp \
	LLVM/lib/Support/BlockFrequency.cpp \
	LLVM/lib/Support/BranchProbability.cpp \
	LLVM/lib/Support/circular_raw_ostream.cpp \
	LLVM/lib/Support/CommandLine.cpp \
	LLVM/lib/Support/ConstantRange.cpp \
	LLVM/lib/Support/CrashRecoveryContext.cpp \
	LLVM/lib/Support/DAGDeltaAlgorithm.cpp \
	LLVM/lib/Support/DataExtractor.cpp \
	LLVM/lib/Support/Debug.cpp \
	LLVM/lib/Support/DeltaAlgorithm.cpp \
	LLVM/lib/Support/Disassembler.cpp \
	LLVM/lib/Support/Dwarf.cpp \
	LLVM/lib/Support/DynamicLibrary.cpp \
	LLVM/lib/Support/Errno.cpp \
	LLVM/lib/Support/ErrorHandling.cpp \
	LLVM/lib/Support/FileUtilities.cpp \
	LLVM/lib/Support/FoldingSet.cpp \
	LLVM/lib/Support/FormattedStream.cpp \
	LLVM/lib/Support/GraphWriter.cpp \
	LLVM/lib/Support/Host.cpp \
	LLVM/lib/Support/IncludeFile.cpp \
	LLVM/lib/Support/IntEqClasses.cpp \
	LLVM/lib/Support/IntervalMap.cpp \
	LLVM/lib/Support/IsInf.cpp \
	LLVM/lib/Support/IsNAN.cpp \
	LLVM/lib/Support/ManagedStatic.cpp \
	LLVM/lib/Support/MemoryBuffer.cpp \
	LLVM/lib/Support/Memory.cpp \
	LLVM/lib/Support/MemoryObject.cpp \
	LLVM/lib/Support/Mutex.cpp \
	LLVM/lib/Support/Path.cpp \
	LLVM/lib/Support/PathV2.cpp \
	LLVM/lib/Support/PluginLoader.cpp \
	LLVM/lib/Support/PrettyStackTrace.cpp \
	LLVM/lib/Support/Process.cpp \
	LLVM/lib/Support/Program.cpp \
	LLVM/lib/Support/raw_os_ostream.cpp \
	LLVM/lib/Support/raw_ostream.cpp \
	LLVM/lib/Support/RWMutex.cpp \
	LLVM/lib/Support/SearchForAddressOfSpecialSymbol.cpp \
	LLVM/lib/Support/Signals.cpp \
	LLVM/lib/Support/SmallPtrSet.cpp \
	LLVM/lib/Support/SmallVector.cpp \
	LLVM/lib/Support/SourceMgr.cpp \
	LLVM/lib/Support/Statistic.cpp \
	LLVM/lib/Support/StringExtras.cpp \
	LLVM/lib/Support/StringMap.cpp \
	LLVM/lib/Support/StringPool.cpp \
	LLVM/lib/Support/StringRef.cpp \
	LLVM/lib/Support/system_error.cpp \
	LLVM/lib/Support/SystemUtils.cpp \
	LLVM/lib/Support/TargetRegistry.cpp \
	LLVM/lib/Support/Threading.cpp \
	LLVM/lib/Support/ThreadLocal.cpp \
	LLVM/lib/Support/Timer.cpp \
	LLVM/lib/Support/TimeValue.cpp \
	LLVM/lib/Support/ToolOutputFile.cpp \
	LLVM/lib/Support/Triple.cpp \
	LLVM/lib/Support/Twine.cpp \
	LLVM/lib/Support/Valgrind.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/Target/X86/InstPrinter/X86ATTInstPrinter.cpp \
	LLVM/lib/Target/X86/InstPrinter/X86InstComments.cpp \
	LLVM/lib/Target/X86/InstPrinter/X86IntelInstPrinter.cpp \
	LLVM/lib/Target/X86/MCTargetDesc/X86AsmBackend.cpp \
	LLVM/lib/Target/X86/MCTargetDesc/X86MachObjectWriter.cpp \
	LLVM/lib/Target/X86/MCTargetDesc/X86MCAsmInfo.cpp \
	LLVM/lib/Target/X86/MCTargetDesc/X86MCCodeEmitter.cpp \
	LLVM/lib/Target/X86/MCTargetDesc/X86MCTargetDesc.cpp \
	LLVM/lib/Target/X86/TargetInfo/X86TargetInfo.cpp \
	LLVM/lib/Target/X86/Utils/X86ShuffleDecode.cpp \
	LLVM/lib/Target/X86/X86CodeEmitter.cpp \
	LLVM/lib/Target/X86/X86ELFWriterInfo.cpp \
	LLVM/lib/Target/X86/X86FastISel.cpp \
	LLVM/lib/Target/X86/X86FloatingPoint.cpp \
	LLVM/lib/Target/X86/X86FrameLowering.cpp \
	LLVM/lib/Target/X86/X86InstrInfo.cpp \
	LLVM/lib/Target/X86/X86ISelDAGToDAG.cpp \
	LLVM/lib/Target/X86/X86ISelLowering.cpp \
	LLVM/lib/Target/X86/X86JITInfo.cpp \
	LLVM/lib/Target/X86/X86RegisterInfo.cpp \
	LLVM/lib/Target/X86/X86SelectionDAGInfo.cpp \
	LLVM/lib/Target/X86/X86Subtarget.cpp \
	LLVM/lib/Target/X86/X86TargetMachine.cpp \
	LLVM/lib/Target/X86/X86TargetObjectFile.cpp \
	LLVM/lib/Target/X86/X86VZeroUpper.cpp \
	LLVM/lib/Target/Mangler.cpp \
	LLVM/lib/Target/Target.cpp \
	LLVM/lib/Target/TargetData.cpp \
	LLVM/lib/Target/TargetELFWriterInfo.cpp \
	LLVM/lib/Target/TargetFrameLowering.cpp \
	LLVM/lib/Target/TargetInstrInfo.cpp \
	LLVM/lib/Target/TargetLibraryInfo.cpp \
	LLVM/lib/Target/TargetLoweringObjectFile.cpp \
	LLVM/lib/Target/TargetMachine.cpp \
	LLVM/lib/Target/TargetRegisterInfo.cpp \
	LLVM/lib/Target/TargetSubtargetInfo.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/Transforms/InstCombine/InstCombineAddSub.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineAndOrXor.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineCalls.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineCasts.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineCompares.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineLoadStoreAlloca.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineMulDivRem.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombinePHI.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineSelect.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineShifts.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineSimplifyDemanded.cpp \
	LLVM/lib/Transforms/InstCombine/InstCombineVectorOps.cpp \
	LLVM/lib/Transforms/InstCombine/InstructionCombining.cpp \
	LLVM/lib/Transforms/Scalar/ADCE.cpp \
	LLVM/lib/Transforms/Scalar/CodeGenPrepare.cpp \
	LLVM/lib/Transforms/Scalar/DeadStoreElimination.cpp \
	LLVM/lib/Transforms/Scalar/GVN.cpp \
	LLVM/lib/Transforms/Scalar/LICM.cpp \
	LLVM/lib/Transforms/Scalar/LoopStrengthReduce.cpp \
	LLVM/lib/Transforms/Scalar/Reassociate.cpp \
	LLVM/lib/Transforms/Scalar/Reg2Mem.cpp \
	LLVM/lib/Transforms/Scalar/ScalarReplAggregates.cpp \
	LLVM/lib/Transforms/Scalar/SCCP.cpp \
	LLVM/lib/Transforms/Scalar/SimplifyCFGPass.cpp \
	LLVM/lib/Transforms/Utils/AddrModeMatcher.cpp \
	LLVM/lib/Transforms/Utils/BasicBlockUtils.cpp \
	LLVM/lib/Transforms/Utils/BreakCriticalEdges.cpp \
	LLVM/lib/Transforms/Utils/BuildLibCalls.cpp \
	LLVM/lib/Transforms/Utils/DemoteRegToStack.cpp \
	LLVM/lib/Transforms/Utils/InstructionNamer.cpp \
	LLVM/lib/Transforms/Utils/LCSSA.cpp \
	LLVM/lib/Transforms/Utils/Local.cpp \
	LLVM/lib/Transforms/Utils/LoopSimplify.cpp \
	LLVM/lib/Transforms/Utils/LowerInvoke.cpp \
	LLVM/lib/Transforms/Utils/LowerSwitch.cpp \
	LLVM/lib/Transforms/Utils/Mem2Reg.cpp \
	LLVM/lib/Transforms/Utils/PromoteMemoryToRegister.cpp \
	LLVM/lib/Transforms/Utils/SimplifyCFG.cpp \
	LLVM/lib/Transforms/Utils/SSAUpdater.cpp \
	LLVM/lib/Transforms/Utils/UnifyFunctionExitNodes.cpp \

LOCAL_SRC_FILES += \
	LLVM/lib/VMCore/AsmWriter.cpp \
	LLVM/lib/VMCore/Attributes.cpp \
	LLVM/lib/VMCore/AutoUpgrade.cpp \
	LLVM/lib/VMCore/BasicBlock.cpp \
	LLVM/lib/VMCore/ConstantFold.cpp \
	LLVM/lib/VMCore/Constants.cpp \
	LLVM/lib/VMCore/Core.cpp \
	LLVM/lib/VMCore/DebugInfoProbe.cpp \
	LLVM/lib/VMCore/DebugLoc.cpp \
	LLVM/lib/VMCore/Dominators.cpp \
	LLVM/lib/VMCore/Function.cpp \
	LLVM/lib/VMCore/GCOV.cpp \
	LLVM/lib/VMCore/Globals.cpp \
	LLVM/lib/VMCore/GVMaterializer.cpp \
	LLVM/lib/VMCore/InlineAsm.cpp \
	LLVM/lib/VMCore/Instruction.cpp \
	LLVM/lib/VMCore/Instructions.cpp \
	LLVM/lib/VMCore/IntrinsicInst.cpp \
	LLVM/lib/VMCore/IRBuilder.cpp \
	LLVM/lib/VMCore/LeakDetector.cpp \
	LLVM/lib/VMCore/LLVMContext.cpp \
	LLVM/lib/VMCore/LLVMContextImpl.cpp \
	LLVM/lib/VMCore/Metadata.cpp \
	LLVM/lib/VMCore/Module.cpp \
	LLVM/lib/VMCore/Pass.cpp \
	LLVM/lib/VMCore/PassManager.cpp \
	LLVM/lib/VMCore/PassRegistry.cpp \
	LLVM/lib/VMCore/PrintModulePass.cpp \
	LLVM/lib/VMCore/Type.cpp \
	LLVM/lib/VMCore/Use.cpp \
	LLVM/lib/VMCore/User.cpp \
	LLVM/lib/VMCore/Value.cpp \
	LLVM/lib/VMCore/ValueSymbolTable.cpp \
	LLVM/lib/VMCore/ValueTypes.cpp \
	LLVM/lib/VMCore/Verifier.cpp \

LOCAL_SRC_FILES += \
	OpenGL/compiler/preprocessor/Diagnostics.cpp \
	OpenGL/compiler/preprocessor/DirectiveHandler.cpp \
	OpenGL/compiler/preprocessor/DirectiveParser.cpp \
	OpenGL/compiler/preprocessor/ExpressionParser.cpp \
	OpenGL/compiler/preprocessor/Input.cpp \
	OpenGL/compiler/preprocessor/Lexer.cpp \
	OpenGL/compiler/preprocessor/Macro.cpp \
	OpenGL/compiler/preprocessor/MacroExpander.cpp \
	OpenGL/compiler/preprocessor/Preprocessor.cpp \
	OpenGL/compiler/preprocessor/Token.cpp \
	OpenGL/compiler/preprocessor/Tokenizer.cpp \
	OpenGL/compiler/AnalyzeCallDepth.cpp \
	OpenGL/compiler/Compiler.cpp \
	OpenGL/compiler/debug.cpp \
	OpenGL/compiler/Diagnostics.cpp \
	OpenGL/compiler/DirectiveHandler.cpp \
	OpenGL/compiler/glslang_lex.cpp \
	OpenGL/compiler/glslang_tab.cpp \
	OpenGL/compiler/InfoSink.cpp \
	OpenGL/compiler/Initialize.cpp \
	OpenGL/compiler/InitializeParseContext.cpp \
	OpenGL/compiler/IntermTraverse.cpp \
	OpenGL/compiler/Intermediate.cpp \
	OpenGL/compiler/intermOut.cpp \
	OpenGL/compiler/ossource_posix.cpp \
	OpenGL/compiler/OutputASM.cpp \
	OpenGL/compiler/parseConst.cpp \
	OpenGL/compiler/ParseHelper.cpp \
	OpenGL/compiler/PoolAlloc.cpp \
	OpenGL/compiler/SymbolTable.cpp \
	OpenGL/compiler/TranslatorASM.cpp \
	OpenGL/compiler/util.cpp \
	OpenGL/compiler/ValidateLimitations.cpp \


LOCAL_CFLAGS += -DLOG_TAG=\"swiftshader\" -Wno-unused-parameter -DDISPLAY_LOGO=0
LOCAL_CFLAGS += -fno-operator-names -msse2 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
LOCAL_CFLAGS += -std=c++11

LOCAL_SHARED_LIBRARIES := \
	libhardware \
	liblog \
	libcutils \
	libui \
	libdl

# Android's make system also uses NDEBUG, so we need to set/unset it forcefully
# Uncomment for ON:
# LOCAL_CFLAGS += -UNDEBUG -g -O0
# Uncomment for OFF:
LOCAL_CFLAGS += -fomit-frame-pointer -DANGLE_DISABLE_TRACE

LOCAL_C_INCLUDES += \
	bionic \
	$(LOCAL_PATH)/LLVM/include-android \
	$(LOCAL_PATH)/LLVM/include-linux \
	$(LOCAL_PATH)/LLVM/include \
	$(LOCAL_PATH)/LLVM/lib/Target/X86 \
        $(LOCAL_PATH)/OpenGL/include \
        $(LOCAL_PATH)/OpenGL/ \
        $(LOCAL_PATH) \
        $(LOCAL_PATH)/Renderer/ \
        $(LOCAL_PATH)/Common/ \
        $(LOCAL_PATH)/Shader/ \
        $(LOCAL_PATH)/Main/

LOCAL_LDFLAGS := -Wl,--no-warn-shared-textrel

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
