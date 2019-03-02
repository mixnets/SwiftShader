GENERATE_GO="build/Visual Studio 15 2017 Win64/generate.go"
go run "${GENERATE_GO}"
if ! git diff HEAD; then
    echo "Visual Studio project files not updated."
    echo "Please run: "
    echo "  go run \"${GENERATE_GO}\""
    echo "to update the Visual Studio project files"
fi