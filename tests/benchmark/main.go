// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"os/exec"
	"regexp"
	"runtime"
	"strings"
	"syscall"
	"time"
)

func main() {
	flag.Usage = func() {
		fmt.Fprintf(flag.CommandLine.Output(), "benchmark <script.json> [script parameters...]\n")
		flag.PrintDefaults()
	}

	if len(os.Args) < 2 {
		flag.Usage()
		os.Exit(1)
	}

	if err := run(os.Args[1], os.Args[2:]); err != nil {
		fmt.Fprint(os.Stderr, err, "\n")
		os.Exit(1)
	}
}

var paramRE = regexp.MustCompile(`\${([^}]+)}`)

func run(path string, args []string) error {
	fileBytes, err := ioutil.ReadFile(path)
	if err != nil {
		return err
	}

	file := string(fileBytes)

	flags := flag.FlagSet{}

	params := map[string]*string{}
	paramMatches := paramRE.FindAllStringSubmatch(string(file), -1)
	for _, match := range paramMatches {
		if len(match) < 2 {
			continue
		}
		name := match[1]
		_, found := params[name]
		if !found {
			params[name] = flags.String(name, "", "")
		}
	}

	if err := flags.Parse(args); err != nil {
		return err
	}

	for name, flag := range params {
		file = strings.Replace(file, "${"+name+"}", *flag, -1)
	}

	decoder := json.NewDecoder(strings.NewReader(file))

	var apps []app
	if err := decoder.Decode(&apps); err != nil {
		return err
	}

	numCPUThreads := runtime.NumCPU()

	type Result struct {
		appName                    string
		numThreads                 int
		averageFPS, minFPS, maxFPS float32
	}

	out, err := os.Create("results.txt")
	if err != nil {
		return err
	}
	defer out.Close()

	var errs []error
	for i, app := range apps {
		results := []Result{}

		for numThreads := 1; numThreads <= numCPUThreads; numThreads++ {
			fps, err := app.run(numThreads)
			if err != nil {
				errs = append(errs, err)
				continue
			}
			r := Result{
				appName:    app.Name,
				numThreads: numThreads,
				minFPS:     math.MaxFloat32,
			}
			for _, n := range fps {
				r.averageFPS += n
				if r.maxFPS < n {
					r.maxFPS = n
				}
				if r.minFPS > n {
					r.minFPS = n
				}
			}
			r.averageFPS /= float32(len(fps))
			results = append(results, r)
		}

		if i > 0 {
			fmt.Fprintf(out, "\n")
		}
		fmt.Fprintf(out, "App\tThreads\tAvr FPS\tMin FPS\tMax FPS\n")
		for _, results := range results {
			fmt.Fprintf(out, "%v\t%d\t%.2f\t%.2f\t%.2f\n", results.appName, results.numThreads, results.averageFPS, results.minFPS, results.maxFPS)
		}
	}

	if len(errs) > 0 {
		msg := fmt.Sprintf("%d errors:\n", len(errs))
		for i, err := range errs {
			msg += fmt.Sprintf("[%v]: %v\n", i, err)
		}
		return fmt.Errorf("%v", msg)
	}

	return nil
}

type app struct {
	Name     string   `json:"name"`
	Path     string   `json:"app"`
	Args     []string `json:"args"`
	Duration string   `json:"duration"`
	Parse    string   `json:"parse"`
}

func (a *app) run(numThreads int) ([]float32, error) {
	re, err := regexp.Compile(a.Parse)
	if err != nil {
		return nil, err
	}

	duration, err := time.ParseDuration(a.Duration)
	if err != nil {
		return nil, err
	}

	var out bytes.Buffer
	cmd := exec.Command(a.Path, a.Args...)
	cmd.Stdout = &out
	cmd.Env = append(os.Environ(), fmt.Sprintf("SWIFTSHADER_MAX_THREADS=%d", numThreads))
	if err := cmd.Start(); err != nil {
		return nil, err
	}

	time.Sleep(duration)

	cmd.Process.Signal(syscall.SIGINT)
	time.Sleep(time.Second)
	if cmd.ProcessState != nil && !cmd.ProcessState.Exited() {
		cmd.Process.Kill()
	}

	matches := re.FindAllSubmatch(out.Bytes(), -1)
	res := []float32{}
	for _, match := range matches {
		var fps float32
		if _, err := fmt.Sscanf(string(match[1]), "%f", &fps); err != nil {
			return nil, err
		}
		res = append(res, fps)
	}

	return res, nil
}
