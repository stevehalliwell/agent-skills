/* Actual CLI/API/install subprocess contracts, separate from timezone suite. */
#define TEST_GENERATOR
#define TEST_GENERATED_PREAMBLE "#include \"tests/core/integration/support.h\"\n"
#include "test.h"
TEST_GENERATE_DIR("tests/core/integration")
