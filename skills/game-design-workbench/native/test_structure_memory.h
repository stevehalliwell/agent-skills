#ifndef TEST_STRUCTURE_MEMORY_H
#define TEST_STRUCTURE_MEMORY_H
/* Combined borrowed source/native queries can exceed the core harness's 64 MiB
   arena. Exercise explicit result ownership/failures on a separate caller heap. */
typedef struct { int outstanding,remaining; } StructureMemory;
static void *structure_allocate(void *context, size_t size) {
    StructureMemory *memory=context; void *result;
    if (!memory->remaining) return NULL;
    if (memory->remaining>0) --memory->remaining;
    result=malloc(size); if (result) ++memory->outstanding; return result;
}
static void structure_release(void *context, void *pointer) { StructureMemory *memory=context; if (pointer) { --memory->outstanding; free(pointer); } }
#endif
