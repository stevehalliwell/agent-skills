/* Finite GUI subprocess contracts. Live viewer orchestration stays external. */
#define TEST_GENERATOR
#define TEST_GENERATED_PREAMBLE "#include \"tests/gui/synchronous/support.h\"\n"
#include "test.h"
TEST_GENERATE_DIR("tests/gui/synchronous")
