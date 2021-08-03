// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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
package parse

import (
	"errors"
	"flag"

	"../shell"
)

var (
	CacheDir      = flag.String("cache", "cache", "path to the output cache directory")
	GerritEmail   = flag.String("email", "$SS_REGRES_EMAIL", "gerrit email address for posting regres results")
	GerritUser    = flag.String("user", "$SS_REGRES_USER", "gerrit username for posting regres results")
	GerritPass    = flag.String("pass", "$SS_REGRES_PASS", "gerrit password for posting regres results")
	GithubUser    = flag.String("gh-user", "$SS_GITHUB_USER", "github user for posting coverage results")
	GithubPass    = flag.String("gh-pass", "$SS_GITHUB_PASS", "github password for posting coverage results")
	KeepCheckouts = flag.Bool("keep", false, "don't delete checkout directories after use")
	DryRun        = flag.Bool("dry", false, "don't post regres reports to gerrit")
	MaxProcMemory = flag.Uint64("max-proc-mem", shell.MaxProcMemory, "maximum virtual memory per child process")
	DailyNow      = flag.Bool("dailynow", false, "Start by running the daily pass")
	DailyOnly     = flag.Bool("dailyonly", false, "Run only the daily pass")
	DailyChange   = flag.String("dailychange", "", "Change hash to use for daily pass, HEAD if not provided")
	Priority      = flag.Int("priority", 0, "Prioritize a single change with the given number")
	Limit         = flag.Int("limit", 0, "only run a maximum of this number of tests")
)

func Parse() {
	flag.ErrHelp = errors.New("regres is a tool to detect regressions between versions of SwiftShader")

	flag.Parse()
}
