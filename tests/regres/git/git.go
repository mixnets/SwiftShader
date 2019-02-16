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

// Package git provides functions for interacting with Git.
package git

import (
	"encoding/hex"
	"os"
	"os/exec"
	"time"

	"../cause"
	"../shell"
)

const (
	gitTimeout = time.Minute * 5 // timeout for a git operation
)

var exe string

func init() {
	path, err := exec.LookPath("git")
	if err != nil {
		panic(cause.Wrap(err, "Couldn't find path to git executable"))
	}
	exe = path
}

// Hash is a 20 byte, git object hash.
type Hash [20]byte

func (h Hash) String() string { return hex.EncodeToString(h[:]) }

// ParseHash returns a Hash from a hexadecimal string.
func ParseHash(s string) Hash {
	b, _ := hex.DecodeString(s)
	h := Hash{}
	copy(h[:], b)
	return h
}

// Add calls 'git add <file>'.
func Add(project, file string) error {
	if err := shell.Shell(gitTimeout, exe, project, "add", file); err != nil {
		return err
	}
	return nil
}

// CommitFlags advanced flags for Commit
type CommitFlags struct {
	Name  string // Used for author and committer
	Email string // Used for author and committer
}

// Commit calls 'git commit -m <msg> --author <author>'.
func Commit(project, msg string, flags CommitFlags) error {
	args := []string{}
	if flags.Name != "" {
		args = append(args, "-c", "user.name="+flags.Name)
	}
	if flags.Email != "" {
		args = append(args, "-c", "user.email="+flags.Email)
	}
	args = append(args, "commit", "-m", msg)
	return shell.Shell(gitTimeout, exe, project, args...)
}

// Push pushes the local branch to remote.
func Push(project, remote, localBranch, remoteBranch string) error {
	return shell.Shell(gitTimeout, exe, project, "push",
		remote, localBranch+":"+remoteBranch)
}

// Checkout performs a git checkout of the given commit into path.
func Checkout(path, url string, commit Hash) error {
	if err := os.MkdirAll(path, 0777); err != nil {
		return cause.Wrap(err, "mkdir '"+path+"' failed")
	}

	for _, cmds := range [][]string{
		{"init"},
		{"remote", "add", "origin", url},
		{"fetch", "origin", commit.String()},
		{"checkout", commit.String()},
	} {
		if err := shell.Shell(gitTimeout, exe, path, cmds...); err != nil {
			os.RemoveAll(path)
			return err
		}
	}

	return nil
}

// FetchRefHash returns the git hash of the given ref.
func FetchRefHash(ref, url string) (Hash, error) {
	out, err := shell.Exec(gitTimeout, exe, "", nil, "ls-remote", url, ref)
	if err != nil {
		return Hash{}, err
	}
	return ParseHash(string(out)), nil
}
