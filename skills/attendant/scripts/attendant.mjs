#!/usr/bin/env node
import * as import_yaml_external from "./yaml.mjs";
var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __require = /* @__PURE__ */ ((x) => typeof require !== "undefined" ? require : typeof Proxy !== "undefined" ? new Proxy(x, {
  get: (a, b) => (typeof require !== "undefined" ? require : a)[b]
}) : x)(function(x) {
  if (typeof require !== "undefined") return require.apply(this, arguments);
  throw Error('Dynamic require of "' + x + '" is not supported');
});
var __esm = (fn, res, err) => function __init() {
  if (err) throw err[0];
  try {
    return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
  } catch (e) {
    throw err = [e], e;
  }
};
var __commonJS = (cb, mod) => function __require2() {
  try {
    return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
  } catch (e) {
    throw mod = 0, e;
  }
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));

// src/contract.ts
import { access as access2, readFile as readFile2 } from "node:fs/promises";
import { basename as basename2, isAbsolute, join, resolve as resolve2 } from "node:path";
function add(diagnostics, code, path, message) {
  diagnostics.push({ code, path, message });
}
function isPrimitive(value2) {
  return value2 === null || typeof value2 === "string" || typeof value2 === "number" || typeof value2 === "boolean";
}
function configPath(projectRoot) {
  return join(projectRoot, ".pi", "attendant.tables");
}
function isRecordFileName(name) {
  return name.endsWith(".md") && !name.startsWith(".");
}
function parseTableLine(line) {
  const trimmed = line.trim();
  if (!trimmed || trimmed.startsWith("#")) return void 0;
  const alias = /^(.*?)\s+as\s+(.+)$/.exec(trimmed);
  if (!alias) return { directory: trimmed };
  return { directory: alias[1].trim(), name: alias[2].trim() };
}
function parseSchema(text, schemaPath, diagnostics) {
  const opening = /^---\r?\n/.exec(text);
  if (!opening) {
    add(diagnostics, "schema-frontmatter", schemaPath, "Schema must begin with YAML front matter.");
    return [];
  }
  const rest = text.slice(opening[0].length);
  const closingIndex = rest.search(/^---\r?$/m);
  if (closingIndex < 0) {
    add(diagnostics, "schema-frontmatter", schemaPath, "Schema front matter is missing closing delimiter.");
    return [];
  }
  const document = (0, import_yaml2.parseDocument)(rest.slice(0, closingIndex), { uniqueKeys: true });
  for (const error of document.errors) {
    add(diagnostics, "schema-yaml", schemaPath, error.message);
  }
  if (document.errors.length > 0 || !(0, import_yaml2.isMap)(document.contents)) return [];
  const fields = [];
  for (const pair of document.contents.items) {
    if (!(0, import_yaml2.isScalar)(pair.key) || typeof pair.key.value !== "string" || !pair.key.value) {
      add(diagnostics, "schema-key", schemaPath, "Schema field names must be non-empty strings.");
      continue;
    }
    const name = pair.key.value;
    if (reservedFields.has(name)) {
      add(diagnostics, "schema-field-reserved", schemaPath, `${name} is reserved for Attendant.`);
      continue;
    }
    const value2 = pair.value;
    if (value2 === null || (0, import_yaml2.isScalar)(value2) && value2.value === null) {
      fields.push({ name, kind: "untyped", optional: true });
      continue;
    }
    if ((0, import_yaml2.isSeq)(value2)) {
      if (value2.items.length === 0) {
        add(diagnostics, "schema-enum", schemaPath, `${name} must be a non-empty top-level scalar enum array.`);
        continue;
      }
      const enumValues = [];
      for (const item of value2.items) {
        if (!(0, import_yaml2.isScalar)(item) || !isPrimitive(item.value) || item.value === null) {
          add(diagnostics, "schema-enum", schemaPath, `${name} must be a non-empty top-level scalar enum array.`);
          break;
        }
        enumValues.push(item.value);
      }
      if (enumValues.length !== value2.items.length) continue;
      fields.push({ name, kind: "enum", optional: false, defaultValue: enumValues[0], enumValues });
      continue;
    }
    if (!(0, import_yaml2.isScalar)(value2) || !isPrimitive(value2.value)) {
      add(diagnostics, "schema-nested", schemaPath, `${name} must use a top-level scalar, enum array, or null.`);
      continue;
    }
    if (typeof value2.value !== "string") {
      fields.push({ name, kind: "default", optional: false, defaultValue: value2.value });
      continue;
    }
    const raw = value2.value;
    const optional = raw.startsWith("?");
    const marker = optional ? raw.slice(1) : raw;
    if (marker === "date" || marker === "datetime" || marker === "string[]") {
      fields.push({ name, kind: marker === "string[]" ? "string-array" : marker, optional });
      continue;
    }
    const reference = /^(ref|ref\[\]):(.+)$/.exec(marker);
    if (reference) {
      fields.push({ name, kind: reference[1] === "ref" ? "ref" : "ref-array", optional, referenceCollection: reference[2] });
      continue;
    }
    if (marker === "ref" || marker === "ref[]" || marker.startsWith("ref:") || marker.startsWith("ref[]:")) {
      add(diagnostics, "schema-ref", schemaPath, `${name} reference marker needs a collection name.`);
      continue;
    }
    if (optional) {
      add(diagnostics, "schema-marker", schemaPath, `${name} uses unsupported optional marker ${raw}.`);
      continue;
    }
    fields.push({ name, kind: "default", optional: false, defaultValue: raw });
  }
  return fields;
}
async function loadContract(projectRoot) {
  const diagnostics = [];
  const collections = [];
  const tablesPath = configPath(projectRoot);
  let config;
  try {
    config = await readFile2(tablesPath, "utf8");
  } catch {
    add(diagnostics, "tables-missing", tablesPath, "Missing .pi/attendant.tables.");
    return { collections, diagnostics };
  }
  const names = /* @__PURE__ */ new Set();
  for (const [index, line] of config.split(/\r?\n/).entries()) {
    const entry = parseTableLine(line);
    if (!entry) continue;
    if (!entry.directory) {
      add(diagnostics, "tables-line", tablesPath, `Line ${index + 1} has no directory.`);
      continue;
    }
    const directory = isAbsolute(entry.directory) ? entry.directory : resolve2(projectRoot, entry.directory);
    const name = entry.name ?? basename2(directory);
    if (!name || name.includes("/") || name.startsWith("__attendant_")) {
      add(diagnostics, "collection-name", tablesPath, `Line ${index + 1} has invalid or reserved collection name ${JSON.stringify(name)}.`);
      continue;
    }
    if (names.has(name)) {
      add(diagnostics, "collection-duplicate", tablesPath, `Collection name ${JSON.stringify(name)} is repeated.`);
      continue;
    }
    names.add(name);
    const schemaPath = join(directory, ".schema.md");
    try {
      await access2(schemaPath);
    } catch {
      add(diagnostics, "schema-missing", schemaPath, `Collection ${JSON.stringify(name)} has no .schema.md.`);
      continue;
    }
    let schema2;
    try {
      schema2 = await readFile2(schemaPath, "utf8");
    } catch {
      add(diagnostics, "schema-read", schemaPath, "Schema cannot be read.");
      continue;
    }
    collections.push({ name, directory, schemaPath, fields: parseSchema(schema2, schemaPath, diagnostics) });
  }
  const collectionNames = new Set(collections.map((collection) => collection.name));
  for (const collection of collections) {
    for (const field of collection.fields) {
      if ((field.kind === "ref" || field.kind === "ref-array") && !collectionNames.has(field.referenceCollection)) {
        add(diagnostics, "schema-ref", collection.schemaPath, `${field.name} references unknown collection ${JSON.stringify(field.referenceCollection)}.`);
      }
    }
  }
  return { collections, diagnostics };
}
var import_yaml2, reservedFields;
var init_contract = __esm({
  "src/contract.ts"() {
    "use strict";
    import_yaml2 = import_yaml_external;
    reservedFields = /* @__PURE__ */ new Set(["id", "name", "created_at", "updated_at", "desc", "tags", "hidden", "source_path", "source_hash"]);
  }
});

// src/records.ts
import { readdir, readFile as readFile3 } from "node:fs/promises";
import { basename as basename3, join as join2 } from "node:path";
function resolveReference(value2, sourceCollection, field) {
  const separator = value2.indexOf("/");
  if (separator < 0) return field === "depends_on" && value2.length > 0 ? { collection: sourceCollection, name: value2 } : void 0;
  if (separator < 1 || separator === value2.length - 1) return void 0;
  return { collection: value2.slice(0, separator), name: value2.slice(separator + 1) };
}
function add2(diagnostics, code, path, message) {
  diagnostics.push({ code, path, message });
}
function isScalarValue(value2) {
  return value2 === null || typeof value2 === "string" || typeof value2 === "number" || typeof value2 === "boolean";
}
function parseFrontMatter(text, path, diagnostics) {
  const opening = /^---\r?\n/.exec(text);
  if (!opening) {
    add2(diagnostics, "record-frontmatter", path, "Record must begin with YAML front matter.");
    return void 0;
  }
  const rest = text.slice(opening[0].length);
  const closingIndex = rest.search(/^---\r?$/m);
  if (closingIndex < 0) {
    add2(diagnostics, "record-frontmatter", path, "Record front matter is missing closing delimiter.");
    return void 0;
  }
  const document = (0, import_yaml3.parseDocument)(rest.slice(0, closingIndex), { uniqueKeys: true });
  for (const error of document.errors) add2(diagnostics, "record-yaml", path, error.message);
  if (document.errors.length > 0 || !(0, import_yaml3.isMap)(document.contents)) return void 0;
  const frontMatter = {};
  for (const pair of document.contents.items) {
    if (!(0, import_yaml3.isScalar)(pair.key) || typeof pair.key.value !== "string" || !pair.key.value) {
      add2(diagnostics, "record-key", path, "Front matter keys must be non-empty strings.");
      continue;
    }
    const value2 = pair.value;
    if (value2 === null || (0, import_yaml3.isScalar)(value2) && isScalarValue(value2.value)) {
      frontMatter[pair.key.value] = value2 === null ? null : value2.value;
      continue;
    }
    if ((0, import_yaml3.isSeq)(value2) && value2.items.every((item) => (0, import_yaml3.isScalar)(item) && isScalarValue(item.value))) {
      frontMatter[pair.key.value] = value2.items.map((item) => item.value);
      continue;
    }
    add2(diagnostics, "record-nested", path, `${pair.key.value} must be a top-level scalar, null, or scalar array.`);
  }
  return frontMatter;
}
function isUuidV7(value2) {
  return typeof value2 === "string" && /^[0-9a-f]{8}-[0-9a-f]{4}-7[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(value2);
}
function isDate(value2) {
  if (!/^\d{4}-\d{2}-\d{2}$/.test(value2)) return false;
  const date2 = /* @__PURE__ */ new Date(`${value2}T00:00:00.000Z`);
  return !Number.isNaN(date2.valueOf()) && date2.toISOString().slice(0, 10) === value2;
}
function isDateTime(value2) {
  return /^\d{4}-\d{2}-\d{2}T/.test(value2) && !Number.isNaN(Date.parse(value2));
}
function validateUniversal(frontMatter, path, diagnostics) {
  if ("id" in frontMatter && !isUuidV7(frontMatter.id)) add2(diagnostics, "record-id", path, "id must be UUIDv7.");
  if ("name" in frontMatter && typeof frontMatter.name !== "string") add2(diagnostics, "record-name", path, "name must be a string.");
  if ("created_at" in frontMatter && (typeof frontMatter.created_at !== "string" || !isDateTime(frontMatter.created_at))) {
    add2(diagnostics, "record-created-at", path, "created_at must be an ISO/RFC3339 date-time.");
  }
  if ("desc" in frontMatter && typeof frontMatter.desc !== "string") add2(diagnostics, "record-desc", path, "desc must be a string.");
  if ("tags" in frontMatter && (!Array.isArray(frontMatter.tags) || frontMatter.tags.some((tag) => typeof tag !== "string"))) {
    add2(diagnostics, "record-tags", path, "tags must be a string array.");
  }
  if ("hidden" in frontMatter && typeof frontMatter.hidden !== "boolean") add2(diagnostics, "record-hidden", path, "hidden must be boolean.");
}
function validateFields(collection, frontMatter, path, diagnostics) {
  for (const field of collection.fields) {
    const value2 = frontMatter[field.name];
    if (value2 === void 0 || value2 === null) {
      if (!field.optional && !["default", "enum", "string-array", "untyped"].includes(field.kind)) {
        add2(diagnostics, "record-required", path, `${field.name} is required.`);
      }
      continue;
    }
    if (field.kind === "default" && typeof value2 !== typeof field.defaultValue) {
      add2(diagnostics, "record-type", path, `${field.name} must match its scalar default type.`);
    }
    if (field.kind === "enum" && !field.enumValues?.some((allowed) => Object.is(allowed, value2))) {
      add2(diagnostics, "record-enum", path, `${field.name} must be one of its declared enum values.`);
    }
    if (field.kind === "date" && (typeof value2 !== "string" || !isDate(value2))) add2(diagnostics, "record-date", path, `${field.name} must be YYYY-MM-DD.`);
    if (field.kind === "datetime" && (typeof value2 !== "string" || !isDateTime(value2))) add2(diagnostics, "record-datetime", path, `${field.name} must be ISO/RFC3339 date-time.`);
    if (field.kind === "ref" && typeof value2 !== "string") add2(diagnostics, "record-ref", path, `${field.name} must be a collection/name string.`);
    if (field.kind === "ref-array" && (!Array.isArray(value2) || value2.some((item) => typeof item !== "string"))) {
      add2(diagnostics, "record-ref", path, `${field.name} must be a collection/name string array.`);
    }
    if (field.kind === "string-array" && (!Array.isArray(value2) || value2.some((item) => typeof item !== "string"))) {
      add2(diagnostics, "record-string-array", path, `${field.name} must be a string array.`);
    }
  }
}
function snapshotRecord(collection, path, text) {
  const diagnostics = [];
  const frontMatter = parseFrontMatter(text, path, diagnostics);
  if (frontMatter) {
    validateUniversal(frontMatter, path, diagnostics);
    validateFields(collection, frontMatter, path, diagnostics);
  }
  return { collection, path, text, frontMatter, diagnostics };
}
function validateSnapshots(contract, snapshots) {
  const diagnostics = [...contract.diagnostics, ...snapshots.flatMap((snapshot) => snapshot.diagnostics)];
  const names = /* @__PURE__ */ new Map();
  const references = [];
  for (const collection of contract.collections) names.set(collection.name, /* @__PURE__ */ new Set());
  const ids = /* @__PURE__ */ new Map();
  for (const collection of contract.collections) ids.set(collection.name, /* @__PURE__ */ new Set());
  for (const snapshot of snapshots) {
    const frontMatter = snapshot.frontMatter;
    if (!frontMatter) continue;
    const collectionNames = names.get(snapshot.collection.name);
    const collectionIds = ids.get(snapshot.collection.name);
    if (typeof frontMatter.id === "string") {
      if (collectionIds.has(frontMatter.id)) add2(diagnostics, "record-id-duplicate", snapshot.path, `Duplicate id ${JSON.stringify(frontMatter.id)}.`);
      collectionIds.add(frontMatter.id);
    }
    const name = typeof frontMatter.name === "string" ? frontMatter.name : basename3(snapshot.path, ".md");
    if (collectionNames.has(name)) add2(diagnostics, "record-name-duplicate", snapshot.path, `Duplicate name ${JSON.stringify(name)}.`);
    collectionNames.add(name);
    for (const field of snapshot.collection.fields) {
      const value2 = frontMatter[field.name];
      if (field.kind === "ref" && typeof value2 === "string") references.push({ collection: snapshot.collection, field: field.name, value: value2, path: snapshot.path });
      if (field.kind === "ref-array" && Array.isArray(value2)) {
        for (const target of value2) if (typeof target === "string") references.push({ collection: snapshot.collection, field: field.name, value: target, path: snapshot.path });
      }
    }
  }
  for (const reference of references) {
    const target = resolveReference(reference.value, reference.collection.name, reference.field);
    if (!target) {
      add2(diagnostics, "record-ref", reference.path, `${reference.field} must use collection/name.`);
      continue;
    }
    if (!names.get(target.collection)?.has(target.name)) add2(diagnostics, "record-ref-missing", reference.path, `${reference.field} target ${JSON.stringify(reference.value)} was not found.`);
  }
  return diagnostics;
}
async function validateRecords(projectRoot) {
  const contract = await loadContract(projectRoot);
  const diagnostics = [...contract.diagnostics];
  let records = 0;
  const names = /* @__PURE__ */ new Map();
  const references = [];
  for (const collection of contract.collections) {
    const collectionNames = /* @__PURE__ */ new Set();
    const collectionIds = /* @__PURE__ */ new Set();
    names.set(collection.name, collectionNames);
    let entries;
    try {
      entries = await readdir(collection.directory, { withFileTypes: true });
    } catch {
      add2(diagnostics, "collection-read", collection.directory, "Collection directory cannot be read.");
      continue;
    }
    for (const entry of entries) {
      if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
      const path = join2(collection.directory, entry.name);
      let text;
      try {
        text = await readFile3(path, "utf8");
      } catch {
        add2(diagnostics, "record-read", path, "Record cannot be read as UTF-8.");
        continue;
      }
      records += 1;
      const frontMatter = parseFrontMatter(text, path, diagnostics);
      if (!frontMatter) continue;
      validateUniversal(frontMatter, path, diagnostics);
      validateFields(collection, frontMatter, path, diagnostics);
      if (typeof frontMatter.id === "string") {
        if (collectionIds.has(frontMatter.id)) add2(diagnostics, "record-id-duplicate", path, `Duplicate id ${JSON.stringify(frontMatter.id)}.`);
        collectionIds.add(frontMatter.id);
      }
      const name = typeof frontMatter.name === "string" ? frontMatter.name : basename3(entry.name, ".md");
      if (collectionNames.has(name)) add2(diagnostics, "record-name-duplicate", path, `Duplicate name ${JSON.stringify(name)}.`);
      collectionNames.add(name);
      for (const field of collection.fields) {
        const value2 = frontMatter[field.name];
        if (field.kind === "ref" && typeof value2 === "string") references.push({ collection, field: field.name, value: value2, path });
        if (field.kind === "ref-array" && Array.isArray(value2)) {
          for (const target of value2) if (typeof target === "string") references.push({ collection, field: field.name, value: target, path });
        }
      }
    }
  }
  for (const reference of references) {
    const target = resolveReference(reference.value, reference.collection.name, reference.field);
    if (!target) {
      add2(diagnostics, "record-ref", reference.path, `${reference.field} must use collection/name.`);
      continue;
    }
    if (!names.get(target.collection)?.has(target.name)) add2(diagnostics, "record-ref-missing", reference.path, `${reference.field} target ${JSON.stringify(reference.value)} was not found.`);
  }
  return { collections: contract.collections, diagnostics, records };
}
var import_yaml3;
var init_records = __esm({
  "src/records.ts"() {
    "use strict";
    import_yaml3 = import_yaml_external;
    init_contract();
  }
});

// src/doctor.ts
import { createHash as createHash2 } from "node:crypto";
import { access as access3, readFile as readFile4, readdir as readdir2, stat } from "node:fs/promises";
import { constants as fsConstants } from "node:fs";
import { DatabaseSync } from "node:sqlite";
import { join as join3 } from "node:path";
function hash(text) {
  return createHash2("sha256").update(text).digest("hex");
}
function check(id, status, message, details) {
  return { id, status, message, details };
}
function nodeAtLeast(required2) {
  const actual = process.versions.node.split(".").map(Number);
  const expected = required2.split(".").map(Number);
  return actual[0] > expected[0] || actual[0] === expected[0] && (actual[1] > expected[1] || actual[1] === expected[1] && actual[2] >= expected[2]);
}
async function projectionCheck(root2, checks) {
  const path = join3(root2, ".attendant", "attendant.sqlite");
  try {
    await access3(path);
  } catch {
    checks.push(check("projection", "error", "Projection is missing.", { path }));
    return "missing";
  }
  let db;
  try {
    db = new DatabaseSync(path, { readOnly: true, allowExtension: false });
  } catch {
    checks.push(check("projection", "error", "Projection cannot be opened.", { path }));
    return "corrupt";
  }
  try {
    const quick = db.prepare("PRAGMA quick_check").get();
    if (Object.values(quick)[0] !== "ok") {
      checks.push(check("projection", "error", "Projection integrity check failed.", { path }));
      return "corrupt";
    }
  } catch {
    checks.push(check("projection", "error", "Projection integrity check failed.", { path }));
    return "corrupt";
  }
  try {
    const contract = await loadContract(root2);
    const metadata = new Map(db.prepare('SELECT key, value FROM "__attendant_metadata"').all().map((row) => [row.key, row.value]));
    if (metadata.get("config_hash") !== hash(await readFile4(join3(root2, ".pi", "attendant.tables"), "utf8"))) {
      checks.push(check("projection", "error", "Projection is stale: config changed."));
      return "stale";
    }
    const cached = new Map(db.prepare('SELECT source_path, mtime_ms, size FROM "__attendant_cache"').all().map((row) => [row.source_path, row]));
    for (const collection of contract.collections) {
      const schemaHash = hash(`${collection.name}\\0${collection.directory}\\0${await readFile4(collection.schemaPath, "utf8")}`);
      if (metadata.get(`schema_hash:${collection.name}`) !== schemaHash) {
        checks.push(check("projection", "error", "Projection is stale: schema changed.", { collection: collection.name }));
        return "stale";
      }
      for (const entry of await readdir2(collection.directory, { withFileTypes: true })) {
        if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
        const sourcePath = join3(collection.directory, entry.name);
        const modified = await stat(sourcePath);
        const prior = cached.get(sourcePath);
        if (!prior || prior.mtime_ms !== modified.mtimeMs || prior.size !== modified.size) {
          checks.push(check("projection", "error", "Projection is stale: source records changed.", { path: sourcePath }));
          return "stale";
        }
        cached.delete(sourcePath);
      }
    }
    if (cached.size) {
      checks.push(check("projection", "error", "Projection is stale: source records were removed."));
      return "stale";
    }
    checks.push(check("projection", "ok", "Projection is fresh.", { path }));
    return "fresh";
  } catch {
    checks.push(check("projection", "error", "Projection layout is stale; normal data actions will refresh it.", { path }));
    return "stale";
  } finally {
    db.close();
  }
}
async function doctor(root2) {
  const checks = [];
  if (nodeAtLeast("24.10.0")) checks.push(check("runtime-node", "ok", "Node runtime supported.", { version: process.versions.node }));
  else checks.push(check("runtime-node", "error", "Node runtime is below 24.10.0.", { version: process.versions.node }));
  try {
    const db = new DatabaseSync(":memory:");
    try {
      const version = db.prepare("SELECT sqlite_version() AS version").get().version;
      db.exec("CREATE VIRTUAL TABLE probe USING fts5(content, tokenize = 'trigram')");
      checks.push(check("runtime-sqlite", "ok", "SQLite FTS5 with trigram tokenizer available.", { version }));
    } finally {
      db.close();
    }
  } catch {
    checks.push(check("runtime-sqlite", "error", "SQLite FTS5 trigram tokenizer is unavailable."));
  }
  const contract = await loadContract(root2);
  const configPath3 = join3(root2, ".pi", "attendant.tables");
  try {
    await access3(configPath3);
    checks.push(check("config", "ok", "Attendant config found.", { collections: contract.collections.length }));
  } catch {
    checks.push(check("config", "error", "Missing .pi/attendant.tables.", { path: configPath3 }));
  }
  try {
    await access3(root2, fsConstants.W_OK);
    checks.push(check("state-write", "ok", "Project root is writable for generated state."));
  } catch {
    checks.push(check("state-write", "error", "Project root is not writable for generated state."));
  }
  try {
    const ignore = await readFile4(join3(root2, ".gitignore"), "utf8");
    checks.push(check("git-ignore", /(?:^|\n)\/?\.attendant\/?\s*(?:\n|$)/.test(ignore) ? "ok" : "warn", /(?:^|\n)\/?\.attendant\/?\s*(?:\n|$)/.test(ignore) ? "Generated state is ignored." : "Generated .attendant state is not ignored."));
  } catch {
    checks.push(check("git-ignore", "warn", "No .gitignore entry for generated .attendant state."));
  }
  const projection = await projectionCheck(root2, checks);
  const validation = await validateRecords(root2);
  checks.push(check("diagnostics", validation.diagnostics.length ? "warn" : "ok", `${validation.diagnostics.length} record/schema diagnostic(s).`, { count: validation.diagnostics.length }));
  return { ok: !checks.some((item) => item.status === "error"), checks, diagnostics: validation.diagnostics, projection };
}
var init_doctor = __esm({
  "src/doctor.ts"() {
    "use strict";
    init_contract();
    init_records();
  }
});

// src/add-table.ts
import { randomBytes as randomBytes2 } from "node:crypto";
import { mkdir as mkdir2, readFile as readFile5, readdir as readdir3, rename as rename2, rm as rm2, writeFile as writeFile2 } from "node:fs/promises";
import { basename as basename4, dirname as dirname2, isAbsolute as isAbsolute2, join as join4, relative as relative2, resolve as resolve3, sep as sep2 } from "node:path";
function fail2(message) {
  throw new Error(`add-table-${message}`);
}
function safeName(value2, label) {
  if (!/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(value2)) fail2(`${label}: ${JSON.stringify(value2)} is not a safe collection name.`);
}
function configPath2(root2) {
  return join4(root2, ".pi", "attendant.tables");
}
function configLine(root2, directory, alias) {
  const relativePath = relative2(root2, directory);
  const path = relativePath && relativePath !== ".." && !relativePath.startsWith(`..${sep2}`) ? relativePath.split(sep2).join("/") : directory;
  return `${path}${alias ? ` as ${alias}` : ""}`;
}
function configuredNames(root2, config) {
  const names = /* @__PURE__ */ new Set();
  for (const line of config.split(/\r?\n/)) {
    const entry = parseTableLine(line);
    if (!entry) continue;
    names.add(entry.name ?? basename4(resolve3(root2, entry.directory)));
  }
  return names;
}
async function addTable(root2, rawDirectory, rawAlias) {
  if (typeof rawDirectory !== "string" || !rawDirectory.trim() || rawDirectory.includes("\0")) fail2("directory: path must be non-empty.");
  const projectRoot = resolve3(root2);
  const directory = resolve3(projectRoot, rawDirectory);
  const relativeDirectory = relative2(projectRoot, directory);
  if (directory === projectRoot || !isAbsolute2(rawDirectory) && (relativeDirectory === ".." || relativeDirectory.startsWith(`..${sep2}`))) {
    fail2("directory: path must identify a collection directory.");
  }
  const alias = rawAlias?.trim() || void 0;
  const name = alias ?? basename4(directory);
  safeName(name, "name");
  if (alias) safeName(alias, "alias");
  const tablesPath = configPath2(projectRoot);
  let config;
  try {
    config = await readFile5(tablesPath, "utf8");
  } catch {
    fail2(`config: missing ${tablesPath}.`);
  }
  const contract = await loadContract(projectRoot);
  if (contract.diagnostics.length) fail2(`config: ${contract.diagnostics[0].message}`);
  const names = configuredNames(projectRoot, config);
  if (names.has(name)) fail2(`duplicate: collection ${JSON.stringify(name)} already exists.`);
  const duplicateDirectory = config.split(/\r?\n/).some((line2) => {
    const entry = parseTableLine(line2);
    return entry && resolve3(projectRoot, entry.directory) === directory;
  });
  if (duplicateDirectory) fail2(`duplicate: directory ${JSON.stringify(directory)} is already configured.`);
  if (name.startsWith("__attendant_")) fail2(`name: ${JSON.stringify(name)} is reserved.`);
  const schemaPath = join4(directory, ".schema.md");
  const templatePath = join4(directory, ".template.md");
  let existingDirectory = false;
  try {
    const entries = await readdir3(directory, { withFileTypes: true });
    existingDirectory = true;
    if (entries.length) fail2(`directory: target must not contain files.`);
  } catch (error) {
    if (error.code !== "ENOENT") throw error;
  }
  const line = configLine(projectRoot, directory, alias);
  if (config.split(/\r?\n/).some((item) => item.trim() === line)) fail2(`duplicate: config entry ${JSON.stringify(line)} already exists.`);
  const created = [schemaPath, templatePath];
  const temporaryConfig = join4(dirname2(tablesPath), `.${basename4(tablesPath)}.attendant-${process.pid}-${randomBytes2(4).toString("hex")}`);
  let targetCreated = false;
  try {
    if (!existingDirectory) {
      await mkdir2(directory, { recursive: true });
      targetCreated = true;
    }
    await writeFile2(schemaPath, "---\n---\n", { flag: "wx" });
    await writeFile2(templatePath, "", { flag: "wx" });
    const currentConfig = await readFile5(tablesPath, "utf8");
    if (currentConfig !== config) fail2("config: changed while collection was being created.");
    const nextConfig = `${config.replace(/\s*$/, "")}${config.trim() ? "\n" : ""}${line}
`;
    await writeFile2(temporaryConfig, nextConfig, { flag: "wx" });
    await rename2(temporaryConfig, tablesPath);
    return { name, directory, schemaPath, templatePath, configPath: tablesPath };
  } catch (error) {
    await rm2(temporaryConfig, { force: true });
    for (const path of created) await rm2(path, { force: true });
    if (targetCreated) {
      try {
        if (!(await readdir3(directory)).length) await rm2(directory, { force: true });
      } catch {
      }
    }
    if (error instanceof Error && error.message.startsWith("add-table-")) throw error;
    throw new Error(`add-table-write: ${error instanceof Error ? error.message : String(error)}`);
  }
}
var init_add_table = __esm({
  "src/add-table.ts"() {
    "use strict";
    init_contract();
  }
});

// src/corrections.ts
import { randomBytes as randomBytes3 } from "node:crypto";
import { readdir as readdir4, readFile as readFile6, rename as rename3, writeFile as writeFile3 } from "node:fs/promises";
import { basename as basename5, dirname as dirname3, join as join5 } from "node:path";
function add3(diagnostics, code, path, message) {
  diagnostics.push({ code, path, message });
}
function uuidv72() {
  const timestamp = Date.now().toString(16).padStart(12, "0");
  const random = randomBytes3(10).toString("hex");
  const variant = (Number.parseInt(random[3], 16) & 3 | 8).toString(16);
  return `${timestamp.slice(0, 8)}-${timestamp.slice(8)}-7${random.slice(0, 3)}-${variant}${random.slice(4, 7)}-${random.slice(7, 19)}`;
}
function parseRecord(text, path, diagnostics) {
  const opening = /^---(\r?\n)/.exec(text);
  if (!opening) {
    add3(diagnostics, "record-frontmatter", path, "Record must begin with YAML front matter.");
    return void 0;
  }
  const rest = text.slice(opening[0].length);
  const closing = /^---\r?$/m.exec(rest);
  if (!closing || closing.index === void 0) {
    add3(diagnostics, "record-frontmatter", path, "Record front matter is missing closing delimiter.");
    return void 0;
  }
  const document = (0, import_yaml4.parseDocument)(rest.slice(0, closing.index), { uniqueKeys: true });
  for (const error of document.errors) add3(diagnostics, "record-yaml", path, error.message);
  if (document.errors.length > 0 || !(0, import_yaml4.isMap)(document.contents)) return void 0;
  const afterClosing = rest.slice(closing.index + closing[0].length);
  return { document, rest: afterClosing, newline: opening[1] === "\r\n" ? "\r\n" : "\n" };
}
function has(document, field) {
  return document.get(field, true) !== void 0;
}
function addDefaults(collection, document, fields) {
  for (const field of collection.fields) {
    if (has(document, field.name)) continue;
    if (field.kind === "default" || field.kind === "enum") fields[field.name] = field.defaultValue;
    if (field.kind === "string-array") fields[field.name] = [];
  }
}
function buildText(parsed, fields) {
  for (const [field, value2] of Object.entries(fields)) parsed.document.set(field, value2);
  const frontMatter = parsed.document.toString({ lineWidth: 0 }).replace(/\n/g, parsed.newline);
  return `---${parsed.newline}${frontMatter}---${parsed.rest}`;
}
function oldestFirst(left, right) {
  return left.createdAt - right.createdAt || left.path.localeCompare(right.path);
}
function currentString(candidate, field) {
  const value2 = candidate.fields[field] ?? candidate.parsed.document.get(field);
  return typeof value2 === "string" ? value2 : "";
}
async function planCollection(collection, diagnostics) {
  let entries;
  try {
    entries = await readdir4(collection.directory, { withFileTypes: true, encoding: "utf8" });
  } catch {
    add3(diagnostics, "collection-read", collection.directory, "Collection directory cannot be read.");
    return [];
  }
  const materializedAt = (/* @__PURE__ */ new Date()).toISOString();
  const candidates = [];
  for (const entry of entries.sort((left, right) => left.name.localeCompare(right.name))) {
    if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
    const path = join5(collection.directory, entry.name);
    let text;
    try {
      text = await readFile6(path, "utf8");
    } catch {
      add3(diagnostics, "record-read", path, "Record cannot be read as UTF-8.");
      continue;
    }
    const parsed = parseRecord(text, path, diagnostics);
    if (!parsed) continue;
    const fields = {};
    if (!isUuidV7(parsed.document.get("id"))) fields.id = uuidv72();
    if (!has(parsed.document, "name")) fields.name = basename5(entry.name, ".md");
    if (!has(parsed.document, "created_at")) fields.created_at = materializedAt;
    if (!has(parsed.document, "desc")) fields.desc = "";
    if (!has(parsed.document, "tags")) fields.tags = [];
    addDefaults(collection, parsed.document, fields);
    const created = fields.created_at ?? parsed.document.get("created_at");
    candidates.push({
      path,
      parsed,
      fields,
      name: typeof (fields.name ?? parsed.document.get("name")) === "string" ? String(fields.name ?? parsed.document.get("name")) : basename5(entry.name, ".md"),
      id: typeof (fields.id ?? parsed.document.get("id")) === "string" ? String(fields.id ?? parsed.document.get("id")) : "",
      createdAt: typeof created === "string" && !Number.isNaN(Date.parse(created)) ? Date.parse(created) : Number.POSITIVE_INFINITY
    });
  }
  const byName = /* @__PURE__ */ new Map();
  const byId = /* @__PURE__ */ new Map();
  for (const candidate of candidates) {
    const name = currentString(candidate, "name");
    const id = currentString(candidate, "id");
    (byName.get(name) ?? byName.set(name, []).get(name)).push(candidate);
    if (id) (byId.get(id) ?? byId.set(id, []).get(id)).push(candidate);
  }
  const usedNames = new Set(candidates.map((candidate) => currentString(candidate, "name")));
  for (const [name, duplicates] of byName) {
    if (duplicates.length < 2) continue;
    for (const candidate of duplicates.sort(oldestFirst).slice(1)) {
      let count = 2;
      let next = `${name} (${count})`;
      while (usedNames.has(next)) next = `${name} (${++count})`;
      candidate.fields.name = next;
      usedNames.add(next);
    }
  }
  for (const duplicates of byId.values()) {
    if (duplicates.length < 2) continue;
    for (const candidate of duplicates.sort(oldestFirst).slice(1)) candidate.fields.id = uuidv72();
  }
  return candidates.filter((candidate) => Object.keys(candidate.fields).length > 0).map((candidate) => ({ path: candidate.path, fields: candidate.fields, nextText: buildText(candidate.parsed, candidate.fields) }));
}
async function planCorrections(projectRoot) {
  const contract = await loadContract(projectRoot);
  const diagnostics = [...contract.diagnostics];
  const corrections = (await Promise.all(contract.collections.map((collection) => planCollection(collection, diagnostics)))).flat();
  return { corrections, diagnostics };
}
async function applyCorrection(correction) {
  const temporaryPath = join5(dirname3(correction.path), `.${basename5(correction.path)}.attendant-${process.pid}-${randomBytes3(4).toString("hex")}`);
  await writeFile3(temporaryPath, correction.nextText, "utf8");
  await rename3(temporaryPath, correction.path);
}
var import_yaml4;
var init_corrections = __esm({
  "src/corrections.ts"() {
    "use strict";
    import_yaml4 = import_yaml_external;
    init_contract();
    init_records();
  }
});

// src/projection.ts
import { createHash as createHash3 } from "node:crypto";
import { mkdir as mkdir3, mkdtemp, readdir as readdir5, readFile as readFile7, rename as rename4, rm as rm3, stat as stat2 } from "node:fs/promises";
import { DatabaseSync as DatabaseSync2 } from "node:sqlite";
import { basename as basename6, join as join6 } from "node:path";
function quote(name) {
  return `"${name.replaceAll('"', '""')}"`;
}
function internal(name) {
  return `__attendant_${name}`;
}
function json(value2) {
  return JSON.stringify(value2);
}
function date(value2) {
  return typeof value2 === "string" ? value2 : null;
}
function datetime(value2) {
  if (typeof value2 !== "string") return null;
  const assumedUtc = /(?:Z|[+-]\d\d:\d\d)$/i.test(value2) ? value2 : `${value2}Z`;
  const parsed = new Date(assumedUtc);
  return Number.isNaN(parsed.valueOf()) ? null : parsed.toISOString();
}
function value(field, source) {
  const raw = source[field.name];
  if (raw === void 0) {
    if (field.kind === "default" || field.kind === "enum") return typeof field.defaultValue === "boolean" ? Number(field.defaultValue) : field.defaultValue ?? null;
    if (field.kind === "string-array") return "[]";
    return null;
  }
  if (raw === null) return null;
  if (field.kind === "date") return date(raw);
  if (field.kind === "datetime") return datetime(raw);
  if (field.kind === "string-array") return json(raw);
  if (field.kind === "default" || field.kind === "enum") return typeof raw === "boolean" ? Number(raw) : typeof raw === "string" || typeof raw === "number" ? raw : null;
  return Array.isArray(raw) ? json(raw) : typeof raw === "boolean" ? Number(raw) : typeof raw === "string" || typeof raw === "number" ? raw : null;
}
function fieldType(field) {
  if (field.kind === "date" || field.kind === "datetime" || field.kind === "ref" || field.kind === "ref-array" || field.kind === "string-array") return "TEXT";
  if (field.kind === "default" || field.kind === "enum") return typeof field.defaultValue === "number" ? "NUMERIC" : typeof field.defaultValue === "boolean" ? "INTEGER" : "TEXT";
  return "";
}
function body(text) {
  const match = /^---\r?\n[\s\S]*?^---\r?\n?/m.exec(text);
  return match ? text.slice(match[0].length) : text;
}
function tableSql(collection) {
  const mandatory = [
    '"id" TEXT PRIMARY KEY',
    '"name" TEXT NOT NULL',
    '"created_at" TEXT NOT NULL',
    '"updated_at" TEXT NOT NULL',
    '"desc" TEXT NOT NULL',
    '"tags" TEXT NOT NULL',
    '"hidden" INTEGER NOT NULL',
    '"source_path" TEXT NOT NULL UNIQUE',
    '"source_hash" TEXT NOT NULL'
  ];
  const declared = collection.fields.map((field) => `${quote(field.name)} ${fieldType(field)}`.trim());
  return `CREATE TABLE ${quote(collection.name)} (${[...mandatory, ...declared].join(", ")})`;
}
function hash2(text) {
  return createHash3("sha256").update(text).digest("hex");
}
function generatedPaths(target) {
  return [target, `${target}-wal`, `${target}-shm`];
}
async function removeGenerated(target) {
  for (; ; ) {
    try {
      await Promise.all(generatedPaths(target).map((path) => rm3(path, { force: true })));
      return;
    } catch (error) {
      if (!(error && typeof error === "object" && "code" in error && error.code === "EBUSY")) throw error;
      await new Promise((resolve5) => setTimeout(resolve5, 10));
    }
  }
}
async function clearToolResults(directory) {
  await rm3(join6(directory, "tool-results"), { recursive: true, force: true });
}
function writerPragmas(db) {
  db.exec("PRAGMA journal_mode = WAL; PRAGMA synchronous = NORMAL; PRAGMA foreign_keys = ON; PRAGMA busy_timeout = 0");
}
function isBusy(error) {
  return error instanceof Error && /SQLITE_BUSY|database is locked/i.test(error.message);
}
async function beginImmediate(db) {
  for (; ; ) {
    try {
      db.exec("BEGIN IMMEDIATE");
      return;
    } catch (error) {
      if (!isBusy(error)) throw error;
      await new Promise((resolve5) => setTimeout(resolve5, 10));
    }
  }
}
function rebuildEdges(db, contract) {
  db.exec(`DELETE FROM ${quote(internal("edges"))}`);
  const insert = db.prepare(`INSERT INTO ${quote(internal("edges"))} (source_collection, source_path, source_id, field, target_collection, target_name, target_path) VALUES (?, ?, ?, ?, ?, ?, ?)`);
  const byName = new Map(contract.collections.map((collection) => [collection.name, collection]));
  for (const collection of contract.collections) {
    for (const field of collection.fields.filter((item) => item.kind === "ref" || item.kind === "ref-array")) {
      const target = byName.get(field.referenceCollection);
      if (!target) continue;
      const rows = db.prepare(`SELECT id, source_path, ${quote(field.name)} AS target FROM ${quote(collection.name)} WHERE ${quote(field.name)} IS NOT NULL`).all();
      for (const row of rows) {
        const targets = field.kind === "ref" ? [row.target] : JSON.parse(row.target);
        for (const value2 of targets) {
          const resolved = resolveReference(value2, collection.name, field.name);
          if (!resolved) continue;
          const targetPath = resolved.collection === target.name ? db.prepare(`SELECT source_path FROM ${quote(target.name)} WHERE name = ?`).get(resolved.name)?.source_path ?? null : field.name === "depends_on" && resolved.collection === collection.name ? db.prepare(`SELECT source_path FROM ${quote(collection.name)} WHERE name = ?`).get(resolved.name)?.source_path ?? null : null;
          insert.run(collection.name, row.source_path, row.id, field.name, resolved.collection, resolved.name, targetPath);
        }
      }
    }
  }
}
async function build(projectRoot, path) {
  const validation = await validateRecords(projectRoot);
  const contract = await loadContract(projectRoot);
  const diagnostics = [...validation.diagnostics];
  const blocked = new Set(diagnostics.filter((item) => blockingCodes.has(item.code)).map((item) => item.path));
  const db = new DatabaseSync2(path);
  try {
    writerPragmas(db);
    await beginImmediate(db);
    db.exec(`CREATE TABLE ${quote(internal("diagnostics"))} (collection_name TEXT, source_path TEXT NOT NULL, code TEXT NOT NULL, message TEXT NOT NULL)`);
    db.exec(`CREATE TABLE ${quote(internal("metadata"))} (key TEXT PRIMARY KEY, value TEXT NOT NULL)`);
    db.exec(`CREATE TABLE ${quote(internal("cache"))} (source_path TEXT PRIMARY KEY, collection_name TEXT NOT NULL, mtime_ms REAL NOT NULL, size INTEGER NOT NULL, source_hash TEXT NOT NULL, front_matter TEXT NOT NULL, local_diagnostics TEXT NOT NULL, content TEXT NOT NULL)`);
    db.exec(`CREATE TABLE ${quote(internal("edges"))} (source_collection TEXT NOT NULL, source_path TEXT NOT NULL, source_id TEXT NOT NULL, field TEXT NOT NULL, target_collection TEXT NOT NULL, target_name TEXT NOT NULL, target_path TEXT)`);
    const insertDiagnostic = db.prepare(`INSERT INTO ${quote(internal("diagnostics"))} VALUES (?, ?, ?, ?)`);
    const insertMetadata = db.prepare(`INSERT INTO ${quote(internal("metadata"))} VALUES (?, ?)`);
    const insertCache = db.prepare(`INSERT INTO ${quote(internal("cache"))} VALUES (?, ?, ?, ?, ?, ?, ?, ?)`);
    insertMetadata.run("layout_version", layoutVersion);
    insertMetadata.run("config_hash", createHash3("sha256").update(await readFile7(join6(projectRoot, ".pi", "attendant.tables"), "utf8")).digest("hex"));
    for (const collection of contract.collections) {
      insertMetadata.run(`schema_hash:${collection.name}`, createHash3("sha256").update(collection.name).update("\\0").update(collection.directory).update("\\0").update(await readFile7(collection.schemaPath, "utf8")).digest("hex"));
    }
    for (const diagnostic of diagnostics) insertDiagnostic.run(null, diagnostic.path, diagnostic.code, diagnostic.message);
    let records = 0;
    for (const collection of contract.collections) {
      db.exec(tableSql(collection));
      const fts = internal(`fts_${collection.name}`);
      const trigram = internal(`trigram_${collection.name}`);
      db.exec(`CREATE VIRTUAL TABLE ${quote(fts)} USING fts5(record_id UNINDEXED, content, tokenize = 'unicode61 remove_diacritics 2')`);
      db.exec(`CREATE VIRTUAL TABLE ${quote(trigram)} USING fts5(record_id UNINDEXED, content, tokenize = 'trigram')`);
      const columns = ["id", "name", "created_at", "updated_at", "desc", "tags", "hidden", "source_path", "source_hash", ...collection.fields.map((field) => field.name)];
      const insert = db.prepare(`INSERT INTO ${quote(collection.name)} (${columns.map(quote).join(", ")}) VALUES (${columns.map(() => "?").join(", ")})`);
      const insertFts = db.prepare(`INSERT INTO ${quote(fts)} (record_id, content) VALUES (?, ?)`);
      const insertTrigram = db.prepare(`INSERT INTO ${quote(trigram)} (record_id, content) VALUES (?, ?)`);
      const entries = await readdir5(collection.directory, { withFileTypes: true });
      for (const entry of entries.sort((a, b) => a.name.localeCompare(b.name))) {
        if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
        const sourcePath = join6(collection.directory, entry.name);
        const modified = await stat2(sourcePath);
        const text = await readFile7(sourcePath, "utf8");
        const sourceHash = hash2(text);
        const snapshot = snapshotRecord(collection, sourcePath, text);
        insertCache.run(sourcePath, collection.name, modified.mtimeMs, modified.size, sourceHash, json(snapshot.frontMatter ?? null), json(snapshot.diagnostics), text);
        if (blocked.has(sourcePath)) continue;
        const fields = snapshot.frontMatter ?? {};
        if (fields.hidden === true) continue;
        const missing = ["id", "name", "created_at", "desc", "tags"].filter((field) => fields[field] === void 0);
        if (missing.length > 0) {
          const diagnostic = { code: "record-missing-mandatory", path: sourcePath, message: `Missing mandatory Attendant field(s): ${missing.join(", ")}.` };
          diagnostics.push(diagnostic);
          insertDiagnostic.run(collection.name, diagnostic.path, diagnostic.code, diagnostic.message);
          continue;
        }
        const id = typeof fields.id === "string" ? fields.id : null;
        const name = typeof fields.name === "string" ? fields.name : basename6(entry.name, ".md");
        const created = datetime(fields.created_at) ?? (/* @__PURE__ */ new Date(0)).toISOString();
        const tags = Array.isArray(fields.tags) ? json(fields.tags) : "[]";
        const row = [id, name, created, modified.mtime.toISOString(), typeof fields.desc === "string" ? fields.desc : "", tags, fields.hidden ? 1 : 0, sourcePath, sourceHash, ...collection.fields.map((field) => value(field, fields))];
        if (!id) continue;
        insert.run(...row);
        const content = `${json(fields)}
${body(text)}`;
        insertFts.run(id, content);
        insertTrigram.run(id, content);
        records += 1;
      }
    }
    rebuildEdges(db, contract);
    db.exec("COMMIT");
    db.exec("PRAGMA wal_checkpoint(TRUNCATE)");
    return { collections: contract.collections.length, records, diagnostics, state: "rebuilt" };
  } catch (error) {
    try {
      db.exec("ROLLBACK");
    } catch {
    }
    throw error;
  } finally {
    db.close();
  }
}
async function cachedProjection(projectRoot, target) {
  let db;
  try {
    db = new DatabaseSync2(target);
  } catch {
    return void 0;
  }
  try {
    const quickCheck = db.prepare("PRAGMA quick_check").get();
    if (Object.values(quickCheck)[0] !== "ok") return void 0;
    const contract = await loadContract(projectRoot);
    const metadata = new Map(db.prepare(`SELECT key, value FROM ${quote(internal("metadata"))}`).all().map(({ key, value: value2 }) => [key, value2]));
    const configHash = hash2(await readFile7(join6(projectRoot, ".pi", "attendant.tables"), "utf8"));
    if (metadata.get("layout_version") !== layoutVersion || metadata.get("config_hash") !== configHash) return void 0;
    for (const collection of contract.collections) {
      const schemaHash = hash2(`${collection.name}\\0${collection.directory}\\0${await readFile7(collection.schemaPath, "utf8")}`);
      if (metadata.get(`schema_hash:${collection.name}`) !== schemaHash) return void 0;
    }
    const cache = new Map(db.prepare(`SELECT source_path, mtime_ms, size, source_hash FROM ${quote(internal("cache"))}`).all().map((row) => [row.source_path, row]));
    const changed = [];
    for (const collection of contract.collections) {
      const entries = await readdir5(collection.directory, { withFileTypes: true });
      for (const entry of entries) {
        if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
        const path = join6(collection.directory, entry.name);
        const modified = await stat2(path);
        const prior = cache.get(path);
        if (!prior) return void 0;
        cache.delete(path);
        if (prior.mtime_ms !== modified.mtimeMs || prior.size !== modified.size) changed.push({ path, mtimeMs: modified.mtimeMs, size: modified.size });
      }
    }
    if (cache.size) return void 0;
    const updates = [];
    for (const item of changed) {
      const current = hash2(await readFile7(item.path, "utf8"));
      const prior = db.prepare(`SELECT source_hash FROM ${quote(internal("cache"))} WHERE source_path = ?`).get(item.path)?.source_hash;
      if (current !== prior) return void 0;
      updates.push(item);
    }
    if (updates.length) {
      writerPragmas(db);
      await beginImmediate(db);
      const update = db.prepare(`UPDATE ${quote(internal("cache"))} SET mtime_ms = ?, size = ? WHERE source_path = ?`);
      for (const item of updates) update.run(item.mtimeMs, item.size, item.path);
      db.exec("COMMIT");
    }
    const diagnostics = db.prepare(`SELECT collection_name, source_path, code, message FROM ${quote(internal("diagnostics"))}`).all();
    const records = contract.collections.reduce((count, collection) => count + Number(db.prepare(`SELECT count(*) AS count FROM ${quote(collection.name)}`).get().count), 0);
    return { collections: contract.collections.length, records, diagnostics, state: "reused" };
  } catch {
    try {
      db.exec("ROLLBACK");
    } catch {
    }
    return void 0;
  } finally {
    db.close();
  }
}
function cacheSnapshot(collection, row) {
  try {
    const frontMatter = JSON.parse(row.front_matter);
    const diagnostics = JSON.parse(row.local_diagnostics);
    if (frontMatter !== null && (typeof frontMatter !== "object" || Array.isArray(frontMatter)) || !Array.isArray(diagnostics)) return void 0;
    return { collection, path: row.source_path, text: row.content, frontMatter: frontMatter ?? void 0, diagnostics };
  } catch {
    return void 0;
  }
}
async function reconcileProjection(projectRoot, target) {
  let db;
  try {
    db = new DatabaseSync2(target);
  } catch {
    return void 0;
  }
  try {
    const contract = await loadContract(projectRoot);
    const metadata = new Map(db.prepare(`SELECT key, value FROM ${quote(internal("metadata"))}`).all().map((row) => [row.key, row.value]));
    if (metadata.get("layout_version") !== layoutVersion || metadata.get("config_hash") !== hash2(await readFile7(join6(projectRoot, ".pi", "attendant.tables"), "utf8"))) return void 0;
    for (const collection of contract.collections) {
      if (metadata.get(`schema_hash:${collection.name}`) !== hash2(`${collection.name}\\0${collection.directory}\\0${await readFile7(collection.schemaPath, "utf8")}`)) return void 0;
    }
    const cache = new Map(db.prepare(`SELECT source_path, collection_name, mtime_ms, size, source_hash, front_matter, local_diagnostics, content FROM ${quote(internal("cache"))}`).all().map((row) => [row.source_path, row]));
    const snapshots = [];
    const changed = /* @__PURE__ */ new Set();
    for (const collection of contract.collections) {
      const entries = await readdir5(collection.directory, { withFileTypes: true });
      for (const entry of entries) {
        if (!entry.isFile() || !isRecordFileName(entry.name)) continue;
        const path = join6(collection.directory, entry.name);
        const modified = await stat2(path);
        const prior = cache.get(path);
        cache.delete(path);
        if (prior && prior.collection_name === collection.name && prior.mtime_ms === modified.mtimeMs && prior.size === modified.size) {
          const snapshot = cacheSnapshot(collection, prior);
          if (!snapshot) return void 0;
          snapshots.push(snapshot);
          continue;
        }
        const text = await readFile7(path, "utf8");
        const sourceHash = hash2(text);
        if (prior && prior.collection_name === collection.name && prior.source_hash === sourceHash) {
          const snapshot = cacheSnapshot(collection, prior);
          if (!snapshot) return void 0;
          snapshots.push(snapshot);
          changed.add(path);
          prior.mtime_ms = modified.mtimeMs;
          prior.size = modified.size;
          continue;
        }
        snapshots.push(snapshotRecord(collection, path, text));
        changed.add(path);
      }
    }
    const deleted = [...cache.values()];
    if (!changed.size && !deleted.length) return void 0;
    const diagnostics = validateSnapshots(contract, snapshots);
    const blocked = new Set(diagnostics.filter((item) => blockingCodes.has(item.code)).map((item) => item.path));
    for (const snapshot of snapshots) {
      const fields = snapshot.frontMatter;
      if (!fields || blocked.has(snapshot.path) || fields.hidden === true) continue;
      const missing = ["id", "name", "created_at", "desc", "tags"].filter((field) => fields[field] === void 0);
      if (missing.length) {
        diagnostics.push({ code: "record-missing-mandatory", path: snapshot.path, message: `Missing mandatory Attendant field(s): ${missing.join(", ")}.` });
        blocked.add(snapshot.path);
      }
    }
    writerPragmas(db);
    await beginImmediate(db);
    const deleteCache = db.prepare(`DELETE FROM ${quote(internal("cache"))} WHERE source_path = ?`);
    const upsertCache = db.prepare(`INSERT OR REPLACE INTO ${quote(internal("cache"))} VALUES (?, ?, ?, ?, ?, ?, ?, ?)`);
    const deleteDiagnostics = db.prepare(`DELETE FROM ${quote(internal("diagnostics"))}`);
    deleteDiagnostics.run();
    const insertDiagnostic = db.prepare(`INSERT INTO ${quote(internal("diagnostics"))} VALUES (?, ?, ?, ?)`);
    for (const diagnostic of diagnostics) insertDiagnostic.run(null, diagnostic.path, diagnostic.code, diagnostic.message);
    for (const row of deleted) deleteCache.run(row.source_path);
    for (const snapshot of snapshots) {
      const modified = await stat2(snapshot.path);
      const sourceHash = hash2(snapshot.text);
      const prior = db.prepare(`SELECT source_hash FROM ${quote(internal("cache"))} WHERE source_path = ?`).get(snapshot.path)?.source_hash;
      if (changed.has(snapshot.path) || !prior) upsertCache.run(snapshot.path, snapshot.collection.name, modified.mtimeMs, modified.size, sourceHash, json(snapshot.frontMatter ?? null), json(snapshot.diagnostics), snapshot.text);
    }
    for (const collection of contract.collections) {
      const existing = new Set(db.prepare(`SELECT source_path FROM ${quote(collection.name)}`).all().map((row) => row.source_path));
      const fts = internal(`fts_${collection.name}`);
      const trigram = internal(`trigram_${collection.name}`);
      for (const snapshot of snapshots.filter((item) => item.collection.name === collection.name)) {
        const fields = snapshot.frontMatter;
        const project = !!fields && !blocked.has(snapshot.path) && fields.hidden !== true;
        if (!changed.has(snapshot.path) && existing.has(snapshot.path) === project) continue;
        const old = db.prepare(`SELECT id FROM ${quote(collection.name)} WHERE source_path = ?`).get(snapshot.path);
        if (old) {
          db.prepare(`DELETE FROM ${quote(fts)} WHERE record_id = ?`).run(old.id);
          db.prepare(`DELETE FROM ${quote(trigram)} WHERE record_id = ?`).run(old.id);
          db.prepare(`DELETE FROM ${quote(collection.name)} WHERE source_path = ?`).run(snapshot.path);
        }
        if (!project || !fields) continue;
        const columns = ["id", "name", "created_at", "updated_at", "desc", "tags", "hidden", "source_path", "source_hash", ...collection.fields.map((field) => field.name)];
        const row = [typeof fields.id === "string" ? fields.id : null, typeof fields.name === "string" ? fields.name : basename6(snapshot.path, ".md"), datetime(fields.created_at) ?? (/* @__PURE__ */ new Date(0)).toISOString(), (await stat2(snapshot.path)).mtime.toISOString(), typeof fields.desc === "string" ? fields.desc : "", Array.isArray(fields.tags) ? json(fields.tags) : "[]", fields.hidden ? 1 : 0, snapshot.path, hash2(snapshot.text), ...collection.fields.map((field) => value(field, fields))];
        if (!row[0]) continue;
        db.prepare(`INSERT INTO ${quote(collection.name)} (${columns.map(quote).join(", ")}) VALUES (${columns.map(() => "?").join(", ")})`).run(...row);
        const content = `${json(fields)}\\n${body(snapshot.text)}`;
        db.prepare(`INSERT INTO ${quote(fts)} (record_id, content) VALUES (?, ?)`).run(row[0], content);
        db.prepare(`INSERT INTO ${quote(trigram)} (record_id, content) VALUES (?, ?)`).run(row[0], content);
      }
      for (const row of deleted.filter((item) => item.collection_name === collection.name)) {
        const old = db.prepare(`SELECT id FROM ${quote(collection.name)} WHERE source_path = ?`).get(row.source_path);
        if (old) {
          db.prepare(`DELETE FROM ${quote(fts)} WHERE record_id = ?`).run(old.id);
          db.prepare(`DELETE FROM ${quote(trigram)} WHERE record_id = ?`).run(old.id);
          db.prepare(`DELETE FROM ${quote(collection.name)} WHERE source_path = ?`).run(row.source_path);
        }
      }
    }
    rebuildEdges(db, contract);
    db.exec("COMMIT");
    const records = contract.collections.reduce((count, collection) => count + Number(db.prepare(`SELECT count(*) AS count FROM ${quote(collection.name)}`).get().count), 0);
    return { collections: contract.collections.length, records, diagnostics, state: "updated" };
  } catch {
    try {
      db.exec("ROLLBACK");
    } catch {
    }
    return void 0;
  } finally {
    db.close();
  }
}
async function projectionIsFresh(projectRoot) {
  const target = join6(projectRoot, ".attendant", "attendant.sqlite");
  return (await cachedProjection(projectRoot, target))?.state === "reused";
}
async function syncProjection(projectRoot) {
  const directory = join6(projectRoot, ".attendant");
  const target = join6(directory, "attendant.sqlite");
  await mkdir3(directory, { recursive: true });
  const cached = await cachedProjection(projectRoot, target);
  if (cached) return cached;
  const reconciled = await reconcileProjection(projectRoot, target);
  if (reconciled) {
    await clearToolResults(directory);
    return reconciled;
  }
  const errors = [];
  for (let attempt = 0; attempt < 2; attempt++) {
    const temporaryDirectory = await mkdtemp(join6(directory, ".attendant-"));
    const temporary = join6(temporaryDirectory, "attendant.sqlite");
    try {
      const result = await build(projectRoot, temporary);
      await removeGenerated(target);
      await rename4(temporary, target);
      await rm3(temporaryDirectory, { recursive: true, force: true });
      await clearToolResults(directory);
      return result;
    } catch (error) {
      errors.push(error);
      await rm3(temporaryDirectory, { recursive: true, force: true });
      await removeGenerated(target);
    }
  }
  throw new AggregateError(errors, "Attendant projection rebuild failed twice.");
}
var layoutVersion, blockingCodes;
var init_projection = __esm({
  "src/projection.ts"() {
    "use strict";
    init_contract();
    init_records();
    layoutVersion = "2";
    blockingCodes = /* @__PURE__ */ new Set([
      "record-frontmatter",
      "record-yaml",
      "record-key",
      "record-nested",
      "record-id",
      "record-name",
      "record-created-at",
      "record-desc",
      "record-tags",
      "record-hidden",
      "record-required",
      "record-type",
      "record-enum",
      "record-date",
      "record-datetime",
      "record-ref",
      "record-string-array",
      "record-id-duplicate",
      "record-name-duplicate"
    ]);
  }
});

// src/lifecycle.ts
async function prepareProject(projectRoot, failOnDiagnostics = false) {
  const corrected = [];
  let validation;
  if (!await projectionIsFresh(projectRoot)) {
    const plan = await planCorrections(projectRoot);
    for (const correction of plan.corrections) {
      await applyCorrection(correction);
      corrected.push({ path: correction.path, fields: correction.fields });
    }
    validation = await validateRecords(projectRoot);
    if (failOnDiagnostics && validation.diagnostics.length) {
      throw new Error(`prepare-invalid: ${validation.diagnostics[0].message}`);
    }
  }
  const sync = await syncProjection(projectRoot);
  if (failOnDiagnostics && sync.diagnostics.length) {
    throw new Error(`prepare-invalid: ${sync.diagnostics[0].message}`);
  }
  return { validation, sync, corrected };
}
var init_lifecycle = __esm({
  "src/lifecycle.ts"() {
    "use strict";
    init_corrections();
    init_projection();
    init_records();
  }
});

// src/create.ts
import { randomBytes as randomBytes4 } from "node:crypto";
import { link, readFile as readFile8, readdir as readdir6, rm as rm4, writeFile as writeFile4 } from "node:fs/promises";
import { basename as basename7, join as join7 } from "node:path";
function uuidv73() {
  const timestamp = Date.now().toString(16).padStart(12, "0");
  const random = randomBytes4(10).toString("hex");
  const variant = (Number.parseInt(random[3], 16) & 3 | 8).toString(16);
  return `${timestamp.slice(0, 8)}-${timestamp.slice(8)}-7${random.slice(0, 3)}-${variant}${random.slice(4, 7)}-${random.slice(7, 19)}`;
}
async function templateBody(collection) {
  const path = join7(collection.directory, ".template.md");
  let text;
  try {
    text = await readFile8(path, "utf8");
  } catch (error) {
    if (error.code === "ENOENT") return "";
    throw error;
  }
  const opening = /^---\r?\n/.exec(text);
  if (!opening) return text;
  const rest = text.slice(opening[0].length);
  const closing = /^---\r?$/m.exec(rest);
  if (!closing || closing.index === void 0) throw new Error(`create-template: ${path} front matter is missing closing delimiter.`);
  const body2 = rest.slice(closing.index + closing[0].length);
  return body2.startsWith("\n") ? body2.slice(1) : body2;
}
function fieldsFor(collection, name, requested) {
  const declared = new Set(["desc", "tags", ...collection.fields.map((field) => field.name)]);
  for (const key of Object.keys(requested)) if (!declared.has(key)) throw new Error(`create-field: ${JSON.stringify(key)} is not declared by ${collection.name}.`);
  const fields = { id: uuidv73(), name, created_at: (/* @__PURE__ */ new Date()).toISOString(), desc: requested.desc ?? "", tags: requested.tags ?? [] };
  for (const field of collection.fields) {
    if (field.name in requested) fields[field.name] = requested[field.name];
    else if (field.kind === "default" || field.kind === "enum") fields[field.name] = field.defaultValue;
    else if (field.kind === "string-array") fields[field.name] = [];
    else if (!field.optional && field.kind !== "untyped") throw new Error(`create-required: ${field.name} is required for ${collection.name}.`);
  }
  return fields;
}
async function createRecord(root2, collectionName, name, requested = {}) {
  if (!/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(name)) throw new Error("create-name: name must be a safe filename stem.");
  const contract = await loadContract(root2);
  const collection = contract.collections.find((item) => item.name === collectionName);
  if (!collection) throw new Error(`create-collection: unknown collection ${JSON.stringify(collectionName)}.`);
  const path = join7(collection.directory, `${name}.md`);
  const fields = fieldsFor(collection, name, requested);
  await prepareProject(root2, true);
  const text = `---
${(0, import_yaml5.stringify)(fields).trimEnd()}
---
${await templateBody(collection)}`;
  const snapshot = snapshotRecord(collection, path, text);
  if (snapshot.diagnostics.length) throw new Error(`create-invalid: ${snapshot.diagnostics[0].message}`);
  const entries = await readdir6(collection.directory, { withFileTypes: true });
  if (entries.some((entry) => entry.isFile() && entry.name === basename7(path))) throw new Error(`create-collision: ${basename7(path)} already exists.`);
  const temporary = join7(collection.directory, `.${name}.attendant-${process.pid}-${randomBytes4(4).toString("hex")}`);
  try {
    await writeFile4(temporary, text, { flag: "wx" });
    await link(temporary, path);
  } catch (error) {
    if (error.code === "EEXIST") throw new Error(`create-collision: ${basename7(path)} already exists.`);
    throw error;
  } finally {
    await rm4(temporary, { force: true });
  }
  await syncProjection(root2);
  return { path, fields };
}
var import_yaml5;
var init_create = __esm({
  "src/create.ts"() {
    "use strict";
    import_yaml5 = import_yaml_external;
    init_contract();
    init_records();
    init_lifecycle();
    init_projection();
  }
});

// src/query.ts
import { DatabaseSync as DatabaseSync3, constants } from "node:sqlite";
import { join as join8 } from "node:path";
function databasePath(root2) {
  return join8(root2, ".attendant", "attendant.sqlite");
}
function quote2(name) {
  return `"${name.replaceAll('"', '""')}"`;
}
function errorSql(sql) {
  return sql.length > MAX_ERROR_SQL_LENGTH ? `${sql.slice(0, MAX_ERROR_SQL_LENGTH)}\u2026` : sql;
}
function parameterNames(params) {
  const names = Object.keys(params);
  return names.length ? names.join(", ") : "none";
}
function queryHint(sql, message) {
  if (/no such column:\s*collection\b/i.test(message) || /\bSELECT\s+collection\b/i.test(sql)) {
    return "Use a literal collection alias such as SELECT 'tasks' AS collection; collection is not a projected table column. Use attendant with operation search for text search.";
  }
  if (/no such table/i.test(message)) return "Use attendant with operation schema to inspect projected table names, or operation search for text search.";
  return void 0;
}
function queryError(kind, sql, params, message) {
  const hint = queryHint(sql, message);
  return new Error(`${kind}: query operation failed. SQL: ${errorSql(sql)}. Parameters: ${parameterNames(params)}. Cause: ${message}${hint ? ` Hint: ${hint}` : ""}`);
}
function oneStatement(sql) {
  if (!sql.trim() || sql.includes("\0")) throw new Error("sql-shape: SQL must contain one non-empty statement.");
  let quote3 = "";
  let semicolons = 0;
  for (let index = 0; index < sql.length; index++) {
    const char = sql[index];
    if (quote3) {
      if (char === quote3 && sql[index + 1] === quote3) {
        index++;
        continue;
      }
      if (char === quote3) quote3 = "";
      continue;
    }
    if (char === "'" || char === '"' || char === "`") {
      quote3 = char;
      continue;
    }
    if (char === ";") semicolons++;
    if (char === "-" && sql[index + 1] === "-" || char === "/" && sql[index + 1] === "*") throw new Error("sql-shape: SQL comments are not supported.");
  }
  if (quote3 || semicolons > 1 || semicolons === 1 && !/^\s*[^;]+;\s*$/.test(sql)) throw new Error("sql-shape: SQL must contain one statement.");
}
async function open(root2, allowFtsShadows = false) {
  await prepareProject(root2);
  const contract = await loadContract(root2);
  const allowedTables = /* @__PURE__ */ new Set(["sqlite_schema", "sqlite_master", "__attendant_diagnostics", "__attendant_edges", ...contract.collections.flatMap((collection) => [collection.name, `__attendant_fts_${collection.name}`, ...allowFtsShadows ? [`__attendant_trigram_${collection.name}`] : []])]);
  const db = new DatabaseSync3(databasePath(root2), { readOnly: true, allowExtension: false });
  db.exec("PRAGMA query_only = ON; PRAGMA foreign_keys = ON; PRAGMA busy_timeout = 0");
  const denied = /* @__PURE__ */ new Set(["load_extension", "readfile", "writefile", "eval"]);
  db.setAuthorizer((action, arg1, arg2) => {
    if (action === constants.SQLITE_READ) {
      const table = arg1 ?? "";
      return allowedTables.has(table) || allowFtsShadows && /^__attendant_(?:fts|trigram)_.+_(config|content|data|docsize|idx)$/.test(table) ? constants.SQLITE_OK : constants.SQLITE_DENY;
    }
    if (action === constants.SQLITE_FUNCTION) return denied.has((arg2 ?? arg1 ?? "").toLowerCase()) ? constants.SQLITE_DENY : constants.SQLITE_OK;
    if (action === constants.SQLITE_SELECT || action === constants.SQLITE_RECURSIVE || action === constants.SQLITE_PRAGMA) return constants.SQLITE_OK;
    return constants.SQLITE_DENY;
  });
  return { db, contract };
}
async function schema(root2) {
  const { db, contract } = await open(root2, true);
  const columns = (table) => db.prepare(`PRAGMA table_info(${quote2(table)})`).all();
  try {
    return {
      collections: contract.collections.map((collection) => {
        const ftsTable = `__attendant_fts_${collection.name}`;
        return {
          name: collection.name,
          table: collection.name,
          columns: columns(collection.name),
          fields: collection.fields.map((field) => ({
            name: field.name,
            kind: field.kind,
            optional: field.optional,
            ...field.enumValues ? { enumValues: field.enumValues } : {},
            ...field.defaultValue !== void 0 ? { defaultValue: field.defaultValue } : {},
            ...field.referenceCollection ? { referenceCollection: field.referenceCollection } : {}
          })),
          fts: { table: ftsTable, columns: columns(ftsTable) }
        };
      }),
      systemTables: [
        { name: "__attendant_diagnostics", columns: columns("__attendant_diagnostics") },
        { name: "__attendant_edges", columns: columns("__attendant_edges") }
      ],
      diagnosticsTable: "__attendant_diagnostics"
    };
  } finally {
    db.close();
  }
}
function shape(rows, limit2) {
  const columns = rows[0] ? Object.keys(rows[0]) : [];
  const result = rows.slice(0, Math.max(1, Math.min(limit2, 1e4)));
  while (result.length && Buffer.byteLength(JSON.stringify(result)) > 10 * 1024 * 1024) result.pop();
  return { columns, rows: result, returned: result.length, truncated: result.length < rows.length };
}
function supportsPartial(text) {
  const operators = /* @__PURE__ */ new Set(["AND", "OR", "NOT", "NEAR"]);
  const terms = [...text.matchAll(/[\p{L}\p{N}]+/gu)].map((match) => match[0]).filter((term) => !operators.has(term.toUpperCase()));
  return terms.length > 0 && terms.every((term) => [...term].length >= 3);
}
function compareSearchRows(left, right) {
  return Number(left.score) - Number(right.score) || String(left.collection).localeCompare(String(right.collection)) || String(left.id).localeCompare(String(right.id));
}
async function search(root2, text, collections, limit2 = 100) {
  if (!text.trim()) throw new Error("fts-query: search query must not be empty.");
  const { db, contract } = await open(root2, true);
  try {
    const selected = collections ? contract.collections.filter((collection) => collections.includes(collection.name)) : contract.collections;
    if (collections && selected.length !== collections.length) throw new Error("fts-query: unknown collection.");
    const complete = [];
    const partial = [];
    const seen = /* @__PURE__ */ new Set();
    for (const collection of selected) {
      const fts = `__attendant_fts_${collection.name}`;
      const select = (table) => db.prepare(`SELECT records.id, records.source_path, records.name, bm25(${quote2(table)}) AS score, snippet(${quote2(table)}, 1, '[', ']', '\u2026', 32) AS snippet FROM ${quote2(table)} JOIN ${quote2(collection.name)} AS records ON records.id = ${quote2(table)}.record_id WHERE ${quote2(table)} MATCH ?`);
      for (const row of select(fts).all(text)) {
        complete.push({ collection: collection.name, ...row });
        seen.add(`${collection.name}\0${row.id}`);
      }
      if (!supportsPartial(text)) continue;
      const trigram = `__attendant_trigram_${collection.name}`;
      for (const row of select(trigram).all(text)) {
        if (!seen.has(`${collection.name}\0${row.id}`)) partial.push({ collection: collection.name, ...row });
      }
    }
    complete.sort(compareSearchRows);
    partial.sort(compareSearchRows);
    return shape([...complete, ...partial], limit2);
  } catch (error) {
    throw new Error(`fts-query: ${error instanceof Error ? error.message.replace(/^[^:]+:\s*/, "") : "search failed."}`);
  } finally {
    db.close();
  }
}
async function query(root2, sql, params = {}, limit2 = 100) {
  try {
    oneStatement(sql);
  } catch (error) {
    throw queryError("sql-shape", sql, params, error instanceof Error ? error.message.replace(/^sql-shape:\s*/, "") : "SQL shape is invalid.");
  }
  const { db } = await open(root2);
  try {
    const bound = Object.fromEntries(Object.entries(params).map(([name, value2]) => [name, typeof value2 === "boolean" ? Number(value2) : value2]));
    const rows = db.prepare(sql).all(bound);
    return shape(rows, limit2);
  } catch (error) {
    const message = error instanceof Error ? error.message : "query failed.";
    throw queryError(/prohibited|not authorized/i.test(message) ? "sql-denied" : "sql-execution", sql, params, message);
  } finally {
    db.close();
  }
}
var MAX_ERROR_SQL_LENGTH;
var init_query = __esm({
  "src/query.ts"() {
    "use strict";
    init_contract();
    init_lifecycle();
    MAX_ERROR_SQL_LENGTH = 2e3;
  }
});

// src/cli-arguments.ts
import { readFile as readFile9 } from "node:fs/promises";
function parseCli(argv) {
  const [candidate, ...arguments_] = argv;
  if (!candidate || candidate === "--help" || candidate === "-h") return { help: true, options: {} };
  if (!(candidate in commands)) throw new Error(`cli-command: unknown command ${JSON.stringify(candidate)}.`);
  const command3 = candidate;
  const options = /* @__PURE__ */ new Map([["--project", { name: "project", alias: "p" }], ["-p", { name: "project", alias: "p" }], ["--help", { name: "help", alias: "h", value: true }], ["-h", { name: "help", alias: "h", value: true }]]);
  for (const option of commands[command3]) {
    options.set(`--${option.name}`, option);
    if (option.alias) options.set(`-${option.alias}`, option);
  }
  const parsed = { command: command3, help: false, options: {} };
  for (let index = 0; index < arguments_.length; index += 1) {
    const token = arguments_[index];
    const option = options.get(token);
    if (!option) throw new Error(token.startsWith("-") ? `cli-option: ${JSON.stringify(token)} is not valid for ${command3}.` : `cli-argument: unexpected positional argument ${JSON.stringify(token)}.`);
    if (option.name in parsed.options || option.name === "project" && parsed.project) throw new Error(`cli-option: ${JSON.stringify(token)} may be supplied only once.`);
    if (option.value) {
      if (option.name === "help") parsed.help = true;
      else parsed.options[option.name] = true;
      continue;
    }
    const value2 = arguments_[index += 1];
    if (!value2 || value2.startsWith("--")) throw new Error(`cli-option: ${JSON.stringify(token)} requires a value.`);
    if (option.name === "project") parsed.project = value2;
    else parsed.options[option.name] = value2;
  }
  return parsed;
}
function inputReader(stdin2 = async () => "") {
  let stdinUsed = false;
  return async (value2) => {
    if (value2 === "-") {
      if (stdinUsed) throw new Error("cli-input: stdin may be used for only one option.");
      stdinUsed = true;
      return stdin2();
    }
    if (!value2.startsWith("@")) return value2;
    if (value2.length === 1) throw new Error("cli-input: @file requires a path.");
    try {
      return await readFile9(value2.slice(1), "utf8");
    } catch (error) {
      throw new Error(`cli-input: cannot read ${JSON.stringify(value2.slice(1))}: ${error instanceof Error ? error.message : String(error)}`);
    }
  };
}
async function jsonInput(value2, read, fallback = {}) {
  if (value2 === void 0) return fallback;
  const text = await read(value2);
  try {
    return JSON.parse(text);
  } catch (error) {
    throw new Error(`cli-json: invalid JSON: ${error instanceof Error ? error.message : String(error)}`);
  }
}
var commands;
var init_cli_arguments = __esm({
  "src/cli-arguments.ts"() {
    "use strict";
    commands = {
      "add-table": [{ name: "directory", alias: "d" }, { name: "alias", alias: "a" }],
      create: [{ name: "collection", alias: "c" }, { name: "name", alias: "n" }, { name: "fields", alias: "f" }],
      doctor: [],
      query: [{ name: "sql", alias: "s" }, { name: "params", alias: "P" }, { name: "limit", alias: "l" }],
      schema: [],
      search: [{ name: "query", alias: "q" }, { name: "collections", alias: "c" }, { name: "limit", alias: "l" }],
      sync: [],
      validate: [{ name: "no-correct", value: true }, { name: "strict", value: true }]
    };
  }
});

// src/cli.ts
var cli_exports = {};
import { resolve as resolve4 } from "node:path";
function stdin() {
  return (async () => {
    let text = "";
    for await (const chunk of process.stdin) text += chunk;
    return text;
  })();
}
function required(value2, command3, option) {
  if (typeof value2 !== "string" || !value2) throw new Error(`cli-input: --${option} is required for ${command3}.`);
  return value2;
}
function limit(value2) {
  if (value2 === void 0) return void 0;
  if (typeof value2 !== "string" || !/^\d+$/.test(value2)) throw new Error("cli-input: --limit must be a non-negative integer.");
  return Number(value2);
}
async function run() {
  const parsed = parseCli(process.argv.slice(2));
  if (parsed.help) return parsed.command ? { ...help, command: parsed.command, detail: help.commands[parsed.command] } : help;
  if (!parsed.command) throw new Error("cli-command: a command is required.");
  const root2 = resolve4(parsed.project ?? process.cwd());
  const { command: command3, options } = parsed;
  const read = inputReader(stdin);
  if (command3 === "doctor") {
    const result2 = await doctor(root2);
    if (!result2.ok) process.exitCode = 1;
    return result2;
  }
  if (command3 === "schema") return schema(root2);
  if (command3 === "sync") return syncProjection(root2);
  if (command3 === "add-table") return addTable(root2, required(options.directory, command3, "directory"), typeof options.alias === "string" ? options.alias : void 0);
  if (command3 === "create") {
    const fields = await jsonInput(typeof options.fields === "string" ? options.fields : void 0, read);
    if (!fields || typeof fields !== "object" || Array.isArray(fields)) throw new Error("create-fields: --fields must be a JSON object.");
    return createRecord(root2, required(options.collection, command3, "collection"), required(options.name, command3, "name"), fields);
  }
  if (command3 === "query") {
    const params = await jsonInput(typeof options.params === "string" ? options.params : void 0, read);
    if (!params || typeof params !== "object" || Array.isArray(params)) throw new Error("query-params: --params must be a JSON object.");
    return query(root2, await read(required(options.sql, command3, "sql")), params, limit(options.limit));
  }
  if (command3 === "search") {
    const collections = typeof options.collections === "string" ? (await read(options.collections)).split(",").filter(Boolean) : void 0;
    return search(root2, await read(required(options.query, command3, "query")), collections, limit(options.limit));
  }
  const plan = await planCorrections(root2);
  const strict = options.strict === true;
  const noCorrect = strict || options["no-correct"] === true;
  if (!noCorrect) for (const correction of plan.corrections) await applyCorrection(correction);
  const result = await validateRecords(root2);
  const corrections = plan.corrections.map(({ path, fields }) => ({ path, fields }));
  if (strict && (result.diagnostics.length || corrections.length)) process.exitCode = 1;
  return { ...result, corrections, corrected: !noCorrect, strict };
}
var help;
var init_cli = __esm({
  async "src/cli.ts"() {
    "use strict";
    init_doctor();
    init_add_table();
    init_create();
    init_query();
    init_projection();
    init_corrections();
    init_records();
    init_cli_arguments();
    help = {
      usage: "node skills/attendant/scripts/attendant.mjs <command> [options]",
      global: { project: "--project, -p <path>; defaults to current directory", input: "@path reads UTF-8; - reads stdin once for SQL, JSON, search text, and collections", output: "one JSON value on stdout; diagnostics on stderr", exits: "0 on success; 1 on operation/input failure; doctor also exits 1 when unhealthy" },
      commands: {
        "add-table": { usage: "add-table --directory|-d <path> [--alias|-a <name>]", safety: "creates only an empty collection; rejects unsafe, duplicate, reserved, and non-empty targets" },
        create: { usage: "create --collection|-c <name> --name|-n <name> [--fields|-f <json|@file|->]", defaults: { fields: {} }, safety: "fields must be a JSON object matching the collection schema" },
        validate: { usage: "validate [--no-correct] [--strict]", defaults: { correction: "mechanical corrections applied" }, safety: "--no-correct preserves source; --strict preserves source and fails pending corrections or diagnostics" },
        sync: { usage: "sync", safety: "rebuilds disposable projection from Markdown source" },
        schema: { usage: "schema", output: "collections, fields, SQLite/FTS metadata, diagnostics, and links" },
        query: { usage: "query --sql|-s <sql|@file|-> [--params|-P <json|@file|->] [--limit|-l <integer>]", defaults: { params: {}, limit: 100 }, safety: "one read-only authorizer-contained SQL statement; use a literal collection alias" },
        search: { usage: "search --query|-q <text|@file|-> [--collections|-c <csv|@file|->] [--limit|-l <integer>]", defaults: { limit: 100 }, safety: "searches projected full text; limit is a non-negative integer" },
        doctor: { usage: "doctor", safety: "read-only health diagnosis; does not correct or refresh state" },
        migrate: { usage: "migrate <check|apply> --plan <path>", safety: "apply requires reviewed ready plan, matching source hashes, and a Git worktree with an existing commit" }
      }
    };
    try {
      console.log(JSON.stringify(await run()));
    } catch (error) {
      console.error(error instanceof Error ? error.message : String(error));
      process.exitCode = 1;
    }
  }
});

// src/migrate.ts
var import_yaml = import_yaml_external;
import { createHash, randomBytes } from "node:crypto";
import { access, mkdir, readFile, rename, rm, writeFile } from "node:fs/promises";
import { basename, dirname, relative, resolve, sep } from "node:path";
import { spawnSync } from "node:child_process";
var root = "";
var command = "";
var args = [];
function fail(message) {
  throw new Error(`migration: ${message}`);
}
function sha256(text) {
  return createHash("sha256").update(text).digest("hex");
}
function projectPath(path, label) {
  if (typeof path !== "string" || !path) fail(`${label} must be a non-empty project-relative path.`);
  const resolved = resolve(root, path);
  const rel = relative(root, resolved);
  if (!rel || rel === ".." || rel.startsWith(`..${sep}`)) fail(`${label} must stay within project root.`);
  return resolved;
}
function uuidv7() {
  const timestamp = Date.now().toString(16).padStart(12, "0");
  const random = randomBytes(10).toString("hex");
  const variant = (Number.parseInt(random[3], 16) & 3 | 8).toString(16);
  return `${timestamp.slice(0, 8)}-${timestamp.slice(8)}-7${random.slice(0, 3)}-${variant}${random.slice(4, 7)}-${random.slice(7, 19)}`;
}
function gitRepository() {
  const inside = spawnSync("git", ["rev-parse", "--is-inside-work-tree"], { cwd: root, encoding: "utf8" });
  if (inside.status !== 0 || inside.stdout.trim() !== "true") fail("apply requires a Git worktree.");
  const head = spawnSync("git", ["rev-parse", "--verify", "HEAD"], { cwd: root, encoding: "utf8" });
  if (head.status !== 0) fail("apply requires an existing Git commit for rollback context.");
}
function object(value2, label) {
  if (!value2 || typeof value2 !== "object" || Array.isArray(value2)) fail(`${label} must be a mapping.`);
  return value2;
}
function parsePlan(text, path) {
  const opening = /^---\r?\n/.exec(text);
  if (!opening) fail(`plan ${path} must begin with YAML front matter.`);
  const rest = text.slice(opening[0].length);
  const closing = /^---\r?$/m.exec(rest);
  if (!closing || closing.index === void 0) fail(`plan ${path} is missing closing front-matter delimiter.`);
  const document = (0, import_yaml.parseDocument)(rest.slice(0, closing.index), { uniqueKeys: true });
  if (document.errors.length || !(0, import_yaml.isMap)(document.contents)) fail(`plan ${path} has invalid YAML front matter.`);
  const plan = object(document.toJS(), "plan");
  if (plan.status !== "ready") fail("plan status must be ready after review before apply.");
  if (!Array.isArray(plan.collections) || !Array.isArray(plan.files) || !plan.files.length) fail("plan needs non-empty collections and files arrays.");
  return { document, plan, suffix: rest.slice(closing.index + closing[0].length) };
}
function removeSpans(source, removals, sourceLabel) {
  if (removals === void 0) return source;
  if (!Array.isArray(removals)) fail(`${sourceLabel}.remove must be an array.`);
  const spans = removals.map((item, index) => {
    const span = object(item, `${sourceLabel}.remove[${index}]`);
    if (!Number.isInteger(span.start) || !Number.isInteger(span.end) || span.start < 0 || span.end < span.start || span.end > source.length || typeof span.text !== "string") fail(`${sourceLabel}.remove[${index}] needs start, end, and text.`);
    if (source.slice(span.start, span.end) !== span.text) fail(`${sourceLabel}.remove[${index}] no longer matches source.`);
    return span;
  }).sort((left, right) => right.start - left.start);
  for (let index = 1; index < spans.length; index++) if (spans[index - 1].start < spans[index].end) fail(`${sourceLabel}.remove spans overlap.`);
  let body2 = source;
  for (const span of spans) body2 = `${body2.slice(0, span.start)}${body2.slice(span.end)}`;
  return body2;
}
async function planApply(planPath) {
  const planText = await readFile(planPath, "utf8");
  const parsed = parsePlan(planText, relative(root, planPath));
  const tablesPath = resolve(root, ".pi", "attendant.tables");
  try {
    await access(tablesPath);
  } catch {
    fail("missing .pi/attendant.tables; run attendant setup first.");
  }
  const tables = await readFile(tablesPath, "utf8");
  const collectionWrites = [];
  const collectionNames = new Set(tables.split(/\r?\n/).map((line) => line.trim()).filter((line) => line && !line.startsWith("#")).map((line) => {
    const alias = /^(.*?)\s+as\s+(.+)$/.exec(line);
    return alias ? alias[2].trim() : basename(line.replace(/\/$/, ""));
  }));
  for (const [index, raw] of parsed.plan.collections.entries()) {
    const collection = object(raw, `collections[${index}]`);
    const directory = projectPath(collection.directory, `collections[${index}].directory`);
    const alias = collection.alias;
    if (alias !== void 0 && (typeof alias !== "string" || !/^[A-Za-z0-9][A-Za-z0-9_-]*$/.test(alias))) fail(`collections[${index}].alias is invalid.`);
    const name = alias ?? basename(directory);
    if (collectionNames.has(name)) fail(`collection name ${JSON.stringify(name)} already exists.`);
    collectionNames.add(name);
    if (typeof collection.schema !== "string" || !collection.schema.startsWith("---\n")) fail(`collections[${index}].schema must be complete flat-schema Markdown.`);
    const line = `${relative(root, directory).split(sep).join("/")}${alias ? ` as ${alias}` : ""}`;
    const schemaPath = resolve(directory, ".schema.md");
    const templatePath = resolve(directory, ".template.md");
    let templateExists = false;
    try {
      await access(schemaPath);
      fail(`schema already exists: ${relative(root, schemaPath)}.`);
    } catch (error) {
      if (error instanceof Error && error.message.startsWith("migration:")) throw error;
    }
    try {
      await access(templatePath);
      templateExists = true;
    } catch (error) {
      if (error && typeof error === "object" && "code" in error && error.code !== "ENOENT") throw error;
    }
    if (tables.split(/\r?\n/).map((item) => item.trim()).includes(line)) fail(`config already contains ${JSON.stringify(line)}.`);
    collectionWrites.push({ directory, schemaPath, templatePath, templateExists, line, schema: collection.schema });
  }
  const fileWrites = [];
  const destinations = /* @__PURE__ */ new Set();
  for (const [index, raw] of parsed.plan.files.entries()) {
    const file = object(raw, `files[${index}]`);
    const source = projectPath(file.source, `files[${index}].source`);
    const destination = projectPath(file.destination, `files[${index}].destination`);
    if (typeof file.source_hash !== "string") fail(`files[${index}].source_hash must be SHA-256.`);
    const sourceText = await readFile(source, "utf8");
    if (sha256(sourceText) !== file.source_hash) fail(`source changed: ${relative(root, source)}.`);
    if (destinations.has(destination)) fail(`destination repeated: ${relative(root, destination)}.`);
    destinations.add(destination);
    if (source !== destination) {
      try {
        await access(destination);
        fail(`destination exists: ${relative(root, destination)}.`);
      } catch (error) {
        if (error instanceof Error && error.message.startsWith("migration:")) throw error;
      }
    }
    const fields = object(file.fields, `files[${index}].fields`);
    const name = typeof fields.name === "string" ? fields.name : basename(destination, ".md");
    const frontMatter = { id: uuidv7(), name, created_at: (/* @__PURE__ */ new Date()).toISOString(), desc: "", tags: [], ...fields };
    const body2 = removeSpans(sourceText, file.remove, `files[${index}]`);
    fileWrites.push({ source, destination, body: body2, text: `---
${(0, import_yaml.stringify)(frontMatter).trimEnd()}
---
${body2}` });
  }
  for (const collection of collectionWrites) {
    if (collection.templateExists) continue;
    const firstRecord = fileWrites.find((file) => dirname(file.destination) === collection.directory);
    collection.template = firstRecord?.body ?? "";
  }
  return { ...parsed, tablesPath, tables, collectionWrites, fileWrites };
}
async function runMigration(projectRoot, argv) {
  root = resolve(projectRoot);
  [command, ...args] = argv;
  const planArg = args[0] === "--plan" ? args[1] : void 0;
  if (!(["check", "apply"].includes(command) && planArg && args.length === 2)) throw new Error("usage: attendant migrate <check|apply> --plan migrations/<slug>.md");
  try {
    const planPath = projectPath(planArg, "plan path");
    if (command === "apply") gitRepository();
    const prepared = await planApply(planPath);
    if (command === "check") {
      return { ok: true, collections: prepared.collectionWrites.map((item) => item.line), templates: prepared.collectionWrites.filter((item) => !item.templateExists).map((item) => relative(root, item.templatePath)), files: prepared.fileWrites.map((item) => relative(root, item.destination)) };
    }
    for (const collection of prepared.collectionWrites) {
      await mkdir(collection.directory, { recursive: true });
      await writeFile(collection.schemaPath, collection.schema, { encoding: "utf8", flag: "wx" });
      if (!collection.templateExists) await writeFile(collection.templatePath, collection.template, { encoding: "utf8", flag: "wx" });
    }
    const config = `${prepared.tables.replace(/\s*$/, "")}${prepared.tables.trim() ? "\n" : ""}${prepared.collectionWrites.map((item) => item.line).join("\n")}
`;
    await writeFile(prepared.tablesPath, config, "utf8");
    for (const file of prepared.fileWrites) {
      await mkdir(dirname(file.destination), { recursive: true });
      const temporary = resolve(dirname(file.destination), `.${basename(file.destination)}.attendant-${process.pid}-${randomBytes(4).toString("hex")}`);
      await writeFile(temporary, file.text, { flag: "wx" });
      await rename(temporary, file.destination);
      if (file.source !== file.destination) await rm(file.source);
    }
    prepared.document.set("status", "applied");
    prepared.document.set("applied_at", (/* @__PURE__ */ new Date()).toISOString());
    await writeFile(planPath, `---
${prepared.document.toString({ lineWidth: 0 })}---${prepared.suffix}`, "utf8");
    return { applied: true, collections: prepared.collectionWrites.map((item) => item.line), templates: prepared.collectionWrites.filter((item) => !item.templateExists).map((item) => relative(root, item.templatePath)), files: prepared.fileWrites.map((item) => relative(root, item.destination)) };
  } catch (error) {
    throw new Error(error instanceof Error ? error.message : "migration: failed.");
  }
}

// src/entry.ts
var [command2, ...args2] = process.argv.slice(2);
if (command2 === "migrate" && (args2.includes("--help") || args2.includes("-h"))) {
  console.log(JSON.stringify({ usage: "migrate <check|apply> --plan <path>", input: "plan is a project-relative Markdown migration plan", output: "one JSON value on stdout; diagnostics on stderr", exits: "0 on success; 1 on validation or apply failure", safety: "apply requires a reviewed ready plan, matching hashes, and a Git worktree with an existing commit" }));
} else if (command2 === "migrate") {
  try {
    console.log(JSON.stringify(await runMigration(process.cwd(), args2)));
  } catch (error) {
    console.error(error instanceof Error ? error.message : String(error));
    process.exitCode = 1;
  }
} else {
  await init_cli().then(() => cli_exports);
}
