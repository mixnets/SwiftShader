//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef _PARSER_HELPER_INCLUDED_
#define _PARSER_HELPER_INCLUDED_

#include "Diagnostics.h"
#include "DirectiveHandler.h"
#include "localintermediate.h"
#include "preprocessor/Preprocessor.h"
#include "Compiler.h"
#include "SymbolTable.h"

struct TMatrixFields {
    bool wholeRow;
    bool wholeCol;
    int row;
    int col;
};

//
// The following are extra variables needed during parsing, grouped together so
// they can be passed to the parser without needing a global.
//
struct TParseContext {
    TParseContext(TSymbolTable& symt, TExtensionBehavior& ext, TIntermediate& interm, GLenum type, int options, bool checksPrecErrors, const char* sourcePath, TInfoSink& is) :
            intermediate(interm),
            symbolTable(symt),
            shaderType(type),
            compileOptions(options),
            sourcePath(sourcePath),
            treeRoot(0),
            lexAfterType(false),
            loopNestingLevel(0),
            structNestingLevel(0),
            inTypeParen(false),
            currentFunctionType(NULL),
            functionReturnsValue(false),
            checksPrecisionErrors(checksPrecErrors),
            diagnostics(is),
            shaderVersion(100),
            directiveHandler(ext, diagnostics, shaderVersion),
            preprocessor(&diagnostics, &directiveHandler),
            scanner(NULL) {  }
    TIntermediate& intermediate; // to hold and build a parse tree
    TSymbolTable& symbolTable;   // symbol table that goes with the language currently being parsed
    GLenum shaderType;              // vertex or fragment language (future: pack or unpack)
    int shaderVersion;
    int compileOptions;
    const char* sourcePath;      // Path of source file or NULL.
    TIntermNode* treeRoot;       // root of parse tree being created
    bool lexAfterType;           // true if we've recognized a type, so can only be looking for an identifier
    int loopNestingLevel;        // 0 if outside all loops
    int structNestingLevel;      // incremented while parsing a struct declaration
    bool inTypeParen;            // true if in parentheses, looking only for an identifier
    const TType* currentFunctionType;  // the return type of the function that's currently being parsed
    bool functionReturnsValue;   // true if a non-void function has a return
    bool checksPrecisionErrors;  // true if an error will be generated when a variable is declared without precision, explicit or implicit.
    TString HashErrMsg;
    bool AfterEOF;
    TDiagnostics diagnostics;
    TDirectiveHandler directiveHandler;
    pp::Preprocessor preprocessor;
    void* scanner;

    int getShaderVersion() const { return shaderVersion; }
    int numErrors() const { return diagnostics.numErrors(); }
    TInfoSink& infoSink() { return diagnostics.infoSink(); }
    void error(const TSourceLoc &loc, const char *reason, const char* token,
               const char* extraInfo="");
    void warning(const TSourceLoc &loc, const char* reason, const char* token,
                 const char* extraInfo="");
    void trace(const char* str);
    void recover();

    bool parseVectorFields(const TString&, int vecSize, TVectorFields&, const TSourceLoc &line);
    bool parseMatrixFields(const TString&, int matSize, TMatrixFields&, const TSourceLoc &line);

    bool reservedErrorCheck(const TSourceLoc &line, const TString& identifier);
    void assignError(const TSourceLoc &line, const char* op, TString left, TString right);
    void unaryOpError(const TSourceLoc &line, const char* op, TString operand);
    void binaryOpError(const TSourceLoc &line, const char* op, TString left, TString right);
    bool precisionErrorCheck(const TSourceLoc &line, TPrecision precision, TBasicType type);
    bool lValueErrorCheck(const TSourceLoc &line, const char* op, TIntermTyped*);
    bool constErrorCheck(TIntermTyped* node);
    bool integerErrorCheck(TIntermTyped* node, const char* token);
    bool globalErrorCheck(const TSourceLoc &line, bool global, const char* token);
    bool constructorErrorCheck(const TSourceLoc &line, TIntermNode*, TFunction&, TOperator, TType*);
    bool arraySizeErrorCheck(const TSourceLoc &line, TIntermTyped* expr, int& size);
    bool arrayQualifierErrorCheck(const TSourceLoc &line, TPublicType type);
    bool arrayTypeErrorCheck(const TSourceLoc &line, TPublicType type);
    bool arrayErrorCheck(const TSourceLoc &line, TString& identifier, TPublicType type, TVariable*& variable);
    bool voidErrorCheck(const TSourceLoc &line, const TString&, const TPublicType&);
    bool boolErrorCheck(const TSourceLoc &line, const TIntermTyped*);
    bool boolErrorCheck(const TSourceLoc &line, const TPublicType&);
    bool samplerErrorCheck(const TSourceLoc &line, const TPublicType& pType, const char* reason);
    bool structQualifierErrorCheck(const TSourceLoc &line, const TPublicType& pType);
    bool parameterSamplerErrorCheck(const TSourceLoc &line, TQualifier qualifier, const TType& type);
    bool nonInitConstErrorCheck(const TSourceLoc &line, TString& identifier, TPublicType& type, bool array);
    bool nonInitErrorCheck(const TSourceLoc &line, TString& identifier, TPublicType& type, TVariable*& variable);
    bool paramErrorCheck(const TSourceLoc &line, TQualifier qualifier, TQualifier paramQualifier, TType* type);
    bool extensionErrorCheck(const TSourceLoc &line, const TString&);

    const TExtensionBehavior& extensionBehavior() const { return directiveHandler.extensionBehavior(); }
    bool supportsExtension(const char* extension);
    void handleExtensionDirective(const TSourceLoc &line, const char* extName, const char* behavior);

    const TPragma& pragma() const { return directiveHandler.pragma(); }
    void handlePragmaDirective(const TSourceLoc &line, const char* name, const char* value);

    bool containsSampler(TType& type);
    bool areAllChildConst(TIntermAggregate* aggrNode);
    const TFunction* findFunction(const TSourceLoc &line, TFunction* pfnCall, bool *builtIn = 0);
    bool executeInitializer(const TSourceLoc &line, TString& identifier, TPublicType& pType,
                            TIntermTyped* initializer, TIntermNode*& intermNode, TVariable* variable = 0);
    bool arraySetMaxSize(TIntermSymbol*, TType*, int, bool, const TSourceLoc &line);

    TIntermTyped* addConstructor(TIntermNode*, const TType*, TOperator, TFunction*, const TSourceLoc &line);
    TIntermTyped* foldConstConstructor(TIntermAggregate* aggrNode, const TType& type);
    TIntermTyped* addConstVectorNode(TVectorFields&, TIntermTyped*, const TSourceLoc &line);
    TIntermTyped* addConstMatrixNode(int , TIntermTyped*, const TSourceLoc &line);
    TIntermTyped* addConstArrayNode(int index, TIntermTyped* node, const TSourceLoc &line);
    TIntermTyped* addConstStruct(TString& , TIntermTyped*, const TSourceLoc &line);
	TIntermTyped* addIndexExpression(TIntermTyped *baseExpression, const TSourceLoc& location, TIntermTyped *indexExpression);

    TLayoutQualifier parseLayoutQualifier(const TString &qualifierType, const TSourceLoc& qualifierTypeLine);
    TLayoutQualifier parseLayoutQualifier(const TString &qualifierType, const TSourceLoc& qualifierTypeLine, const TString &intValueString, int intValue, const TSourceLoc& intValueLine);
    TLayoutQualifier joinLayoutQualifiers(TLayoutQualifier leftQualifier, TLayoutQualifier rightQualifier);

    // Performs an error check for embedded struct declarations.
    // Returns true if an error was raised due to the declaration of
    // this struct.
    bool enterStructDeclaration(const TSourceLoc &line, const TString& identifier);
    void exitStructDeclaration();

    bool structNestingErrorCheck(const TSourceLoc &line, const TType& fieldType);
};

int PaParseStrings(int count, const char* const string[], const int length[],
                   TParseContext* context);

#endif // _PARSER_HELPER_INCLUDED_
