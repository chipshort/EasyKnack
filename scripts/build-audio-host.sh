#!/usr/bin/env bash
# Portable Linux build for the EasyKnack audio host.
#
# Developer flow:
#   mvn process-classes
#   ./scripts/build-audio-host.sh --cxx g++ --test
#
# The Windows build (scripts/build-audio-host.ps1) and the Windows Maven profile
# remain unchanged.
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: build-audio-host.sh [--cxx CXX] [--output DIR] [--test]

  --cxx CXX      C++17 compiler (default: $CXX or c++)
  --output DIR   output directory (default: target/native)
  --test         also build and run AudioGraphTest and LinuxAudioBackendTest
  -h, --help     show this help

Requires target/native/generated/ParameterContracts.hpp; run
"mvn process-classes" first.
EOF
}

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_root="$project_root/native/audio-host"
generated="$project_root/target/native/generated"
cxx="${CXX:-c++}"
output_directory="$project_root/target/native"
run_tests=false

while (($#)); do
    case "$1" in
        --cxx)
            [[ $# -ge 2 ]] || { echo "build-audio-host.sh: --cxx needs a value" >&2; exit 2; }
            cxx="$2"; shift 2 ;;
        --cxx=*) cxx="${1#*=}"; shift ;;
        --output)
            [[ $# -ge 2 ]] || { echo "build-audio-host.sh: --output needs a value" >&2; exit 2; }
            output_directory="$2"; shift 2 ;;
        --output=*) output_directory="${1#*=}"; shift ;;
        --test) run_tests=true; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "build-audio-host.sh: unknown argument: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ ! -f "$generated/ParameterContracts.hpp" ]]; then
    echo 'Run "mvn process-classes" first' >&2
    exit 1
fi

if ! command -v "$cxx" >/dev/null 2>&1; then
    echo "build-audio-host.sh: compiler not found: $cxx" >&2
    exit 1
fi

mkdir -p "$output_directory"
python3 "$project_root/scripts/generate-native-nodes.py" --output-directory "$generated"

mapfile -t node_sources < <(find "$source_root/src/nodes" -name '*.cpp' -type f -print | LC_ALL=C sort)

common_flags=(-std=c++17 -O2 -pthread
    -I "$source_root/third_party" -I "$source_root/src" -I "$generated")
common_libraries=(-ldl -lm)

# Compile one translation unit set into an executable.
# Usage: compile OUTPUT MAIN_SOURCE...
compile() {
    local output="$1"; shift
    "$cxx" "${common_flags[@]}" "$@" "${node_sources[@]}" -o "$output" "${common_libraries[@]}"
}

host="$output_directory/easyknack-audio-host"
echo "Building $host with $cxx"
compile "$host" "$source_root/src/main.cpp"

if [[ "$run_tests" != true ]]; then
    exit 0
fi

run_test() {
    local name="$1"
    local executable="$output_directory/$name"
    echo "Building native test: $name"
    compile "$executable" "$source_root/tests/$name.cpp"
    "$executable"
}

run_test AudioGraphTest
run_test LinuxAudioBackendTest
