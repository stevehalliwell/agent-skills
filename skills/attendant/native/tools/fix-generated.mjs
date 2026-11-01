#!/usr/bin/env node
// cimgui emits identical typedefs twice under CIMGUI_DEFINE_ENUMS_AND_STRUCTS.
// Remove only known duplicates; preserve both C and C++ views and fail on drift.
import { readFileSync, writeFileSync } from 'node:fs';
const path = new URL('../build/deps/cimgui/cimgui.h', import.meta.url);
let header = readFileSync(path, 'utf8').replaceAll('\r\n', '\n');
for (const declaration of [
  'typedef int ImDrawTextFlags;',
  ...['ImTextureRef', 'ImVec2', 'ImVec2i', 'ImVec4', 'ImColor', 'ImRect'].map(name => `typedef struct ${name}_c ${name};`),
]) {
  const first = header.indexOf(declaration);
  const second = header.indexOf(declaration, first + declaration.length);
  if (first < 0 || second < 0 || header.indexOf(declaration, second + declaration.length) >= 0)
    throw new Error(`Unexpected generated typedef count: ${declaration}`);
  header = header.slice(0, second) + header.slice(second + declaration.length);
}
// C99 has no anonymous unions. Name only these C-view union members.
// C++ uses the original ImGui definitions; runtime probe verifies size/offset ABI.
for (const [before, after] of [
  ['union { int val_i; float val_f; void* val_p; };', 'union { int val_i; float val_f; void* val_p; } value;'],
  ['union { int BackupInt[2]; float BackupFloat[2]; };', 'union { int BackupInt[2]; float BackupFloat[2]; } backup;'],
  ['ImGuiInputEventAppFocused AppFocused;\n    };', 'ImGuiInputEventAppFocused AppFocused;\n    } event;'],
]) {
  if (header.split(before).length !== 2) throw new Error(`Anonymous union patch anchor changed: ${before}`);
  header = header.replace(before, after);
}
writeFileSync(path, header);
