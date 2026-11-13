#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p tests/build
gcc -std=c11 -Wall -Wextra -Werror -I main main/ai_engine.c tests/test_ai.c -lm -o tests/build/test_ai
./tests/build/test_ai
gcc -std=c11 -Wall -Wextra -Werror -I main main/sht31_codec.c tests/test_sht31_codec.c -lm -o tests/build/test_sht31_codec
./tests/build/test_sht31_codec
