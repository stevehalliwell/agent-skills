#!/usr/bin/env node
/**
 * Summarize a GLB or JSON glTF without dependencies.
 * Usage: node gltf-report.mjs <model.glb|model.gltf> [--json]
 */
import { readFile, stat } from 'node:fs/promises';
import { extname, resolve } from 'node:path';

function usage() {
  console.error('Usage: node gltf-report.mjs <model.glb|model.gltf> [--json]');
}

function bytes(value) {
  if (value < 1024) return `${value} B`;
  if (value < 1024 ** 2) return `${(value / 1024).toFixed(1)} KiB`;
  return `${(value / 1024 ** 2).toFixed(2)} MiB`;
}

function triangleCount(primitive, accessors) {
  const count = accessors[primitive.indices]?.count ?? accessors[primitive.attributes?.POSITION]?.count ?? 0;
  switch (primitive.mode ?? 4) {
    case 4: return Math.floor(count / 3); // TRIANGLES
    case 5: case 6: return Math.max(0, count - 2); // STRIP, FAN
    default: return 0;
  }
}

function accessorByteLength(accessor) {
  const componentBytes = { 5120: 1, 5121: 1, 5122: 2, 5123: 2, 5125: 4, 5126: 4 }[accessor.componentType] ?? 0;
  const componentCount = { SCALAR: 1, VEC2: 2, VEC3: 3, VEC4: 4, MAT2: 4, MAT3: 9, MAT4: 16 }[accessor.type] ?? 0;
  return accessor.count * componentBytes * componentCount;
}

function parseGlb(buffer) {
  if (buffer.toString('utf8', 0, 4) !== 'glTF') throw new Error('Not a GLB file.');
  if (buffer.readUInt32LE(4) !== 2) throw new Error(`Unsupported GLB version ${buffer.readUInt32LE(4)}.`);
  const jsonLength = buffer.readUInt32LE(12);
  if (buffer.readUInt32LE(16) !== 0x4e4f534a) throw new Error('GLB has no JSON chunk.');
  return JSON.parse(buffer.toString('utf8', 20, 20 + jsonLength));
}

async function loadModel(path) {
  const extension = extname(path).toLowerCase();
  const buffer = await readFile(path);
  if (extension === '.glb') return parseGlb(buffer);
  if (extension === '.gltf') return JSON.parse(buffer.toString('utf8'));
  throw new Error('Model must end in .glb or .gltf.');
}

function report(path, size, gltf) {
  const accessors = gltf.accessors ?? [];
  const bufferViews = gltf.bufferViews ?? [];
  const primitives = (gltf.meshes ?? []).flatMap((mesh) => mesh.primitives ?? []);
  const textureSourceIndices = new Set((gltf.textures ?? []).map((texture) => texture.source).filter(Number.isInteger));
  const imageViews = new Set((gltf.images ?? []).map((image) => image.bufferView).filter(Number.isInteger));
  const accessorViews = new Set(accessors.map((accessor) => accessor.bufferView).filter(Number.isInteger));
  const viewBytes = (viewIndex) => bufferViews[viewIndex]?.byteLength ?? 0;
  const sumViewBytes = (viewIndices) => [...viewIndices].reduce((total, viewIndex) => total + viewBytes(viewIndex), 0);
  const imageBytes = sumViewBytes(imageViews);
  const namedNodes = (gltf.nodes ?? []).filter((node) => node.name).map((node) => node.name);
  const extensions = [...new Set([...(gltf.extensionsUsed ?? []), ...(gltf.extensionsRequired ?? [])])].sort();
  const semanticBytes = {};

  for (const primitive of primitives) {
    for (const [semantic, accessorIndex] of Object.entries(primitive.attributes ?? {})) {
      const accessor = accessors[accessorIndex];
      if (accessor) semanticBytes[semantic] = (semanticBytes[semantic] ?? 0) + accessorByteLength(accessor);
    }
    const indexAccessor = accessors[primitive.indices];
    if (indexAccessor) semanticBytes.INDICES = (semanticBytes.INDICES ?? 0) + accessorByteLength(indexAccessor);
  }

  const embeddedImages = (gltf.images ?? [])
    .map((image, index) => ({
      index,
      name: image.name ?? `image_${index}`,
      mimeType: image.mimeType ?? 'external or unknown',
      bytes: viewBytes(image.bufferView),
    }))
    .sort((a, b) => b.bytes - a.bytes);

  return {
    file: path,
    fileSizeBytes: size,
    asset: gltf.asset ?? {},
    counts: {
      scenes: (gltf.scenes ?? []).length,
      nodes: (gltf.nodes ?? []).length,
      meshes: (gltf.meshes ?? []).length,
      primitives: primitives.length,
      materials: (gltf.materials ?? []).length,
      textures: (gltf.textures ?? []).length,
      images: (gltf.images ?? []).length,
      animations: (gltf.animations ?? []).length,
      skins: (gltf.skins ?? []).length,
      cameras: (gltf.cameras ?? []).length,
      lights: gltf.extensions?.KHR_lights_punctual?.lights?.length ?? 0,
    },
    geometry: {
      vertices: primitives.reduce((total, primitive) => total + (accessors[primitive.attributes?.POSITION]?.count ?? 0), 0),
      triangles: primitives.reduce((total, primitive) => total + triangleCount(primitive, accessors), 0),
      drawCalls: primitives.length,
    },
    textures: {
      referencedImages: textureSourceIndices.size,
      embeddedImageBytes: imageBytes,
      embeddedImages,
    },
    sizeBreakdown: {
      embeddedImagesBytes: imageBytes,
      accessorBytes: sumViewBytes(accessorViews),
      geometryByAttribute: semanticBytes,
    },
    extensions: {
      used: gltf.extensionsUsed ?? [],
      required: gltf.extensionsRequired ?? [],
      all: extensions,
    },
    namedNodes,
  };
}

const args = process.argv.slice(2);
const json = args.includes('--json');
const files = args.filter((arg) => arg !== '--json');
if (files.length !== 1) {
  usage();
  process.exitCode = 1;
} else {
  try {
    const path = resolve(files[0]);
    const [model, file] = await Promise.all([loadModel(path), stat(path)]);
    const result = report(path, file.size, model);
    if (json) {
      console.log(JSON.stringify(result, null, 2));
    } else {
      console.log(`${result.file} (${bytes(result.fileSizeBytes)})`);
      console.log(`asset: ${result.asset.generator ?? 'unknown generator'}; glTF ${result.asset.version ?? 'unknown'}`);
      console.log(`scene: ${result.counts.nodes} nodes, ${result.counts.meshes} meshes, ${result.counts.primitives} primitives/draw calls`);
      console.log(`geometry: ${result.geometry.vertices.toLocaleString()} vertices, ${result.geometry.triangles.toLocaleString()} triangles`);
      console.log(`resources: ${result.counts.materials} materials, ${result.counts.textures} textures, ${result.counts.images} images (${bytes(result.textures.embeddedImageBytes)} embedded)`);
      console.log(`size: embedded images ${bytes(result.sizeBreakdown.embeddedImagesBytes)}; accessor data ${bytes(result.sizeBreakdown.accessorBytes)}`);
      console.log(`geometry data: ${Object.entries(result.sizeBreakdown.geometryByAttribute).map(([semantic, value]) => `${semantic} ${bytes(value)}`).join(', ') || 'none'}`);
      console.log('largest embedded images:');
      for (const image of result.textures.embeddedImages.slice(0, 10)) console.log(`  ${bytes(image.bytes).padStart(10)}  ${image.mimeType}  ${image.name}`);
      console.log(`features: ${result.counts.animations} animations, ${result.counts.skins} skins, ${result.counts.cameras} cameras, ${result.counts.lights} lights`);
      console.log(`extensions: ${result.extensions.all.join(', ') || 'none'}`);
      console.log(`named nodes: ${result.namedNodes.length}`);
    }
  } catch (error) {
    console.error(`gltf-report: ${error.message}`);
    process.exitCode = 1;
  }
}
