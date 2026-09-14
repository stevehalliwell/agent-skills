#!/usr/bin/env node
/**
 * Run a named glTF-Transform profile without overwriting the input.
 * Usage: node gltf-optimize.mjs <input.glb|input.gltf> <output.glb|output.gltf> --profile meshopt|web [--texture-size N]
 */
import { access } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';
import { spawn } from 'node:child_process';

function usage() {
  console.error('Usage: node gltf-optimize.mjs <input> <output> --profile meshopt|web [--texture-size N]');
}

const args = process.argv.slice(2);
const positionals = [];
let profile;
let textureSize;
for (let index = 0; index < args.length; index += 1) {
  if (args[index] === '--profile') profile = args[++index];
  else if (args[index] === '--texture-size') textureSize = Number(args[++index]);
  else positionals.push(args[index]);
}

if (positionals.length !== 2 || !['meshopt', 'web'].includes(profile) || (textureSize !== undefined && (!Number.isInteger(textureSize) || textureSize < 1))) {
  usage();
  process.exitCode = 1;
} else {
  const [input, output] = positionals.map((path) => resolve(path));
  try {
    await access(input);
    await access(dirname(output));
    try {
      await access(output);
      throw new Error(`Refusing to overwrite existing output: ${output}`);
    } catch (error) {
      if (!String(error.message).startsWith('Refusing')) {
        // Output does not exist, which is required for a safe run.
      } else throw error;
    }

    const commandArgs = ['--yes', '@gltf-transform/cli', profile === 'meshopt' ? 'meshopt' : 'optimize', input, output, '-v'];
    if (profile === 'meshopt') commandArgs.push('--level', 'medium');
    if (textureSize !== undefined) {
      if (profile !== 'web') throw new Error('--texture-size is only supported with the web profile.');
      commandArgs.push('--texture-size', String(textureSize));
    }

    console.error(`Running: npx ${commandArgs.join(' ')}`);
    const npxCli = join(dirname(process.execPath), 'node_modules', 'npm', 'bin', 'npx-cli.js');
    const child = process.platform === 'win32'
      ? spawn(process.execPath, [npxCli, ...commandArgs], { stdio: 'inherit' })
      : spawn('npx', commandArgs, { stdio: 'inherit' });
    child.on('error', (error) => {
      console.error(`gltf-optimize: ${error.message}`);
      process.exitCode = 1;
    });
    child.on('exit', (code) => { process.exitCode = code ?? 1; });
  } catch (error) {
    console.error(`gltf-optimize: ${error.message}`);
    process.exitCode = 1;
  }
}
