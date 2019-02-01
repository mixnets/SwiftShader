for dir in "Pipeline" "Reactor" "System" "Vulkan" "WSI"
do
    find ./src/$dir  -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs clang-format -i
done

