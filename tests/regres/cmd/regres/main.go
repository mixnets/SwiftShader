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

// Launch regres in a firejail sandbox. This limits the scope of what resources
// regres and its child processes can access. For more information, see run.go.
package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
    "path"
	"runtime"

    "../../cause"
    "../../parse"
	"../../util"
)

func main() {
    parse.Parse()
    cacheDir := *parse.CacheDir

    if err := setup(cacheDir); err != nil {
        fmt.Fprintln(os.Stderr, err)
        os.Exit(-1)
    }
}

func setup(cacheDir string) error {
    goExe, err := exec.LookPath("go")
    if err != nil {
        return cause.Wrap(err, "Failed to locate go.")
    }

    firejailExe, err := exec.LookPath("firejail")
    if err != nil {
        return cause.Wrap(err, "Failed to locate firejail, please install it with `sudo apt install firejail`")
    }

    firejailProfile, err := ioutil.TempFile("", "firejail_prof")
    if err != nil {
        return cause.Wrap(err, "Failed to create firejail profile")
    }
    defer os.Remove(firejailProfile.Name())

    // Create cache dir if it doesn't already exist
    if !util.IsDir(cacheDir) {
        if err := os.MkdirAll(cacheDir, 0777); err != nil {
            return cause.Wrap(err, "Failed to create cache directory: %v", cacheDir)
        }
    }

    // Create firejail profile
    content := "include PROFILE.local\ninclude globals.local\nread-write "
    buf := bytes.NewBufferString(content)
    fmt.Fprint(buf, cacheDir)
    if _, err := firejailProfile.Write([]byte(buf.String())); err != nil {
        return cause.Wrap(err, "Failed to write to firejail profile.")
    }

    _, fName, _, ok := runtime.Caller(0)
    if !ok {
        return fmt.Errorf("Failed to retrieve runtime Caller information.\n")
    }
    runGoPath := path.Join(path.Dir(fName), "run.go")

    args := append([]string{"--profile="+firejailProfile.Name(), "--"}, goExe, "run", runGoPath)
    args = append(args, os.Args[1:]...)
    cmd := exec.Command(firejailExe, args...)
    cmd.Stdout = os.Stdout
    cmd.Stderr = os.Stderr
    if err := cmd.Run(); err != nil {
        return cause.Wrap(err, "Failed to launch firejail command: %v %v", firejailExe, args)
    }

    return nil
}
