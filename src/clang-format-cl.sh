CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
${CLANG_FORMAT} --version

BASEDIR=$(git rev-parse --show-toplevel)

# Get all added, copied, or modified files (except for .sh files)
FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -v '\.sh')

for FILE in $FILES
do
    ${CLANG_FORMAT} -i -style=file "$BASEDIR/$FILE"
done

