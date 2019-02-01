SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
pushd ${SRC_DIR}
for dir in "Pipeline" "Reactor" "System" "Vulkan" "WSI"
do
    find ${SRC_DIR}/$dir  -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs clang-format -i
done
popd

