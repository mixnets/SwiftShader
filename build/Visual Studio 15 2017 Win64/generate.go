package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"
)

const (
	vsbuildDir    = "build/Visual Studio 15 2017 Win64/"
	fileListBegin = "<!-- FILE LIST BEGIN -->"
	fileListEnd   = "<!-- FILE LIST END -->"
)

func main() {
	if err := run(); err != nil {
		log.Fatal(err)
	}
}

// GenerateRule is the root level JSON structure for the generate.json file.
type GenerateRule struct {
	Output   string
	Template string
	Source   GlobRules
}

// GlobRules is a list of GlobRule.
type GlobRules []GlobRule

// GlobRule is a file list glob inclusion or exclusion.
type GlobRule struct {
	Include string
	Exclude string
}

func run() error {
	genRules := []GenerateRule{}
	if err := decodeJSONFile(filepath.Join(vsbuildDir, "generate.json"), &genRules); err != nil {
		return err
	}
	for _, genRule := range genRules {
		sources := files{}
		for _, sourceRule := range genRule.Source {
			if sourceRule.Include != "" {
				sources.add(glob(sourceRule.Include))
			}
			if sourceRule.Exclude != "" {
				sources.remove(glob(sourceRule.Exclude))
			}
			// sort.Sort(sources)

			path := filepath.Join(vsbuildDir, genRule.Output)

			contentBytes, err := ioutil.ReadFile(path)
			if err != nil {
				return err
			}

			content := string(contentBytes)

			replaceStart := strings.Index(content, fileListBegin)
			if replaceStart < 0 {
				return fmt.Errorf("File list start not found in file '%v'", path)
			}
			replaceStart += len(fileListBegin)

			replaceEnd := strings.Index(content, fileListEnd)
			if replaceEnd < 0 {
				return fmt.Errorf("File list end not found in file '%v'", path)
			}

			replacement, err := generate(filepath.Join(vsbuildDir, genRule.Template), sources)
			if err != nil {
				return err
			}

			out := content[:replaceStart] + replacement + "\n" + content[replaceEnd:]

			if err := ioutil.WriteFile(path, []byte(out), 0666); err != nil {
				return err
			}
		}
	}
	return nil
}

func decodeJSONFile(path string, obj interface{}) error {
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()
	return json.NewDecoder(f).Decode(obj)
}

type file struct {
	dir, name, ext string
}

func (f file) HasExt(exts ...string) bool {
	for _, ext := range exts {
		if f.ext == ext {
			return true
		}
	}
	return false
}

func (f file) String() string { return f.dir + f.name + f.ext }

type files []file

func (l *files) add(f files) {
	*l = append(*l, f...)
}

func (l *files) remove(f files) {
	s := f.set()
	*l = (*l).filter(func(f file) bool { return !s[f] })
}

func (l files) set() map[file]bool {
	out := map[file]bool{}
	for _, f := range l {
		out[f] = true
	}
	return out
}

func (l files) Len() int { return len(l) }
func (l files) Less(i, j int) bool {
	a, b := l[i], l[j]
	if a.dir < b.dir {
		return true
	}
	if a.dir > b.dir {
		return false
	}
	if a.ext < b.ext {
		return true
	}
	if a.ext > b.ext {
		return false
	}
	if a.name < b.name {
		return true
	}
	if a.name > b.name {
		return false
	}
	return false
}
func (l files) Swap(i, j int) { l[i], l[j] = l[j], l[i] }

func (l files) filter(pred func(file) bool) files {
	out := make(files, 0, len(l))
	for _, f := range l {
		if pred(f) {
			out = append(out, f)
		}
	}
	return out
}

func glob(path string) files {
	paths, err := filepath.Glob(path)
	if err != nil {
		log.Fatal(err)
	}
	out := files{}
	for _, path := range paths {
		dir, name := filepath.Split(path)
		dir = strings.ReplaceAll(dir, "/", `\`)
		ext := filepath.Ext(name)
		name = name[:len(name)-len(ext)]
		out = append(out, file{dir, name, ext})
	}
	return out
}

func generate(templatePath string, sources files) (string, error) {
	t := template.New("VS template")
	t.Funcs(template.FuncMap{
		"HasExt": file.HasExt,
	})

	templateBody, err := ioutil.ReadFile(templatePath)
	if err != nil {
		return "", err
	}

	if _, err := t.Parse(string(templateBody)); err != nil {
		return "", err
	}

	buf := bytes.Buffer{}

	data := struct{ SourceFiles files }{sources}
	if err := t.Execute(&buf, data); err != nil {
		return "", err
	}

	out := strings.NewReplacer(
		"\n", "",
		"\r", "",
		" ", "",
		"¶", "\r\n",
		"•", " ",
	).Replace(buf.String())

	return out, nil
}
