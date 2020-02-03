package main

import (
	"fmt"
	"log"
	"os"
	"sort"

	"../../third_party/SPIRV-Tools/utils/vscode/src/schema"
)

var classIsStatement = map[string]bool{
	"@exclude":               true,
	"Miscellaneous":          false,
	"Debug":                  false,
	"Annotation":             false,
	"Extension":              false,
	"Mode-Setting":           false,
	"Type-Declaration":       false,
	"Constant-Creation":      false,
	"Memory":                 true,
	"Function":               false,
	"Image":                  true,
	"Conversion":             true,
	"Composite":              true,
	"Arithmetic":             true,
	"Bit":                    true,
	"Relational_and_Logical": true,
	"Derivative":             true,
	"Control-Flow":           true,
	"Atomic":                 true,
	"Primitive":              true,
	"Barrier":                true,
	"Group":                  true,
	"Device-Side_Enqueue":    true,
	"Pipe":                   true,
	"Non-Uniform":            true,
	"Reserved":               true,
}

func main() {
	if err := run(); err != nil {
		log.Fatalf("%v", err)
	}
}

func run() error {
	type Opcode struct {
		name        string
		id          int
		isStatement bool
	}

	ops := []Opcode{}

	for name, opcode := range schema.Opcodes {
		isStatement, ok := classIsStatement[opcode.Class]
		if !ok {
			return fmt.Errorf("Unhandled opcode class '%s'", opcode.Class)
		}

		ops = append(ops, Opcode{name: name, id: opcode.Opcode, isStatement: isStatement})
	}

	sort.Slice(ops, func(i, j int) bool { return ops[i].id < ops[j].id })

	f, err := os.Create("./src/Pipeline/SpirvShaderInstructions.inc")
	if err != nil {
		return err
	}
	defer f.Close()

	f.WriteString(`// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

// SpirvShaderInstructions.inc holds additional metadata for SPIR-V opcodes.
// SpirvShaderInstructions.inc should only be included by
// SpirvShaderInstructions.cpp.

#ifndef DECORATE_OP
#	error "SpirvShaderInstructions.inc must be included after defining DECORATE_OP()"
#endif

//   is-statement
//          |  op-name
//          |    |
//          v    V
`)
	for _, op := range ops {
		f.WriteString("DECORATE_OP(")
		if op.isStatement {
			f.WriteString("T")
		} else {
			f.WriteString("F")
		}
		f.WriteString(", ")
		f.WriteString(op.name)
		f.WriteString(")\n")
	}

	return nil
}
