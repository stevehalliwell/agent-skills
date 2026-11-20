#!/usr/bin/env node
// Repeatable offline-report browser audit. Uses existing visible Chromium via CDP;
// no browser/package install, OS input, source writes, or remote resource access.
// Usage: node scripts/check-offline-report.mjs --cdp-url ws://127.0.0.1:PORT/devtools/browser/ID --report report.html [--output directory]
// Exit 0 checks pass; 1 audit/runtime failure; 2 invalid arguments.
import { resolve, join } from 'node:path';
import { pathToFileURL } from 'node:url';
import { mkdir, mkdtemp, writeFile, stat } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import assert from 'node:assert/strict';
const options = {};
try {
  const args = process.argv.slice(2);
  if (args.length === 1 && args[0] === '--help') {
    console.log('Audit existing headed Chromium: --cdp-url <local browser websocket> --report <html> [--output <artifact-parent>]. Blocks HTTP(S); verifies anchors/history, progress, focus/theme, mobile overflow, reduced motion, no-JS content, and print. Leaves preview open; artifacts use a new directory. Node.js >=22.');
    process.exit(0);
  }
  for (let index = 0; index < args.length; index += 2) {
    const key = args[index]; const value = args[index + 1];
    if (!['--cdp-url', '--report', '--output'].includes(key) || !value || value.startsWith('--') || key in options) throw Error('Use --help for required options.');
    options[key] = value;
  }
  const endpoint = new URL(options['--cdp-url']);
  if (!['ws:', 'wss:'].includes(endpoint.protocol) || !['127.0.0.1', 'localhost', '[::1]'].includes(endpoint.hostname) || endpoint.username || endpoint.password || !options['--report']) throw Error('Require local browser WebSocket and explicit report path.');
  if (!(await stat(resolve(options['--report']))).isFile()) throw Error('Report must be a regular local file.');
} catch (error) { console.error(error.message); process.exit(2); }
let socket;
let directory;
const pending = new Map();
const requests = [];
const errors = [];
let sequence = 0;
let session;
async function send(method, params = {}, target = session) {
  const id = ++sequence;
  return new Promise((resolveResult, reject) => {
    const timeout = setTimeout(() => { pending.delete(id); reject(Error(`CDP timeout: ${method}`)); }, 12000);
    pending.set(id, { resolve: result => { clearTimeout(timeout); resolveResult(result); }, reject: error => { clearTimeout(timeout); reject(error); } });
    socket.send(JSON.stringify({ id, method, params, ...(target ? { sessionId: target } : {}) }));
  });
}
async function evaluate(expression) {
  const result = await send('Runtime.evaluate', { expression, returnByValue: true, awaitPromise: true });
  if (result.exceptionDetails) throw Error(result.exceptionDetails.text);
  return result.result.value;
}
// Wait in Node: page callbacks do not run while script execution is disabled.
async function settled() { await new Promise(resolveWait => setTimeout(resolveWait, 100)); }
async function loaded(url) {
  for (let attempt = 0; attempt < 100; attempt++) {
    if (await evaluate(`document.readyState === 'complete' && location.href === ${JSON.stringify(url)}`)) { await settled(); return; }
    await new Promise(resolveWait => setTimeout(resolveWait, 50));
  }
  throw Error('Report navigation did not complete.');
}
async function viewport(width, mobile = false) { await send('Emulation.setDeviceMetricsOverride', { width, height: 900, deviceScaleFactor: 1, mobile }); await settled(); }
async function screenshot(name) { const image = await send('Page.captureScreenshot', { format: 'png', captureBeyondViewport: true }); await writeFile(join(directory, name), Buffer.from(image.data, 'base64'), { flag: 'wx' }); }
try {
  const parent = resolve(options['--output'] ?? tmpdir());
  await mkdir(parent, { recursive: true });
  directory = await mkdtemp(join(parent, 'offline-report-check-'));
  socket = new WebSocket(options['--cdp-url']);
  await new Promise((resolveOpen, reject) => { const timeout = setTimeout(() => reject(Error('CDP connect timeout')), 8000); socket.addEventListener('open', () => { clearTimeout(timeout); resolveOpen(); }, { once: true }); socket.addEventListener('error', () => { clearTimeout(timeout); reject(Error('CDP connection failed')); }, { once: true }); });
  socket.addEventListener('message', event => {
    const message = JSON.parse(event.data);
    if (message.id) { const item = pending.get(message.id); pending.delete(message.id); if (item) message.error ? item.reject(Error(message.error.message)) : item.resolve(message.result); }
    if (session && message.sessionId === session) {
      if (message.method === 'Network.requestWillBeSent' && /^https?:/.test(message.params.request.url)) requests.push(message.params.request.url);
      if (message.method === 'Runtime.exceptionThrown') errors.push(message.params.exceptionDetails.text);
      if (message.method === 'Runtime.consoleAPICalled' && message.params.type === 'error') errors.push(message.params.args.map(item => item.value ?? item.description).join(' '));
    }
  });
  const fileUrl = pathToFileURL(resolve(options['--report'])).href;
  const url = `${fileUrl}?audit=${Date.now()}`;
  const targets = (await send('Target.getTargets', {}, null)).targetInfos;
  let target = targets.find(item => item.type === 'page' && item.url.split(/[?#]/)[0] === fileUrl);
  if (!target) target = { targetId: (await send('Target.createTarget', { url: 'about:blank' }, null)).targetId };
  session = (await send('Target.attachToTarget', { targetId: target.targetId, flatten: true }, null)).sessionId;
  await send('Page.enable'); await send('Runtime.enable'); await send('Network.enable');
  await send('Emulation.setScriptExecutionDisabled', { value: false });
  await send('Network.setBlockedURLs', { urls: ['http://*', 'https://*'] });
  const screen = { media: 'screen', features: [{ name: 'prefers-reduced-motion', value: 'reduce' }, { name: 'prefers-color-scheme', value: 'light' }] };
  await send('Emulation.setEmulatedMedia', screen);
  await send('Page.navigate', { url }); await loaded(url); await viewport(1440);
  const links = await evaluate('[...document.querySelectorAll("#report-toc a")].map(a => a.hash)');
  assert.ok(links.length > 0); assert.equal(await evaluate('document.querySelectorAll("section[data-report-section]").length'), links.length);
  await screenshot('desktop.png');
  for (const hash of links) {
    await evaluate(`document.querySelector('#report-toc a[href="${hash}"]').click()`); await settled();
    assert.equal(await evaluate('location.hash'), hash);
    assert.equal(await evaluate('document.querySelector("#report-toc a[aria-current=location]").hash'), hash);
  }
  const direct = url + links[1];
  await send('Page.navigate', { url: direct }); await loaded(direct);
  const history = await send('Page.getNavigationHistory');
  assert.ok(history.currentIndex > 0);
  const previous = history.entries[history.currentIndex - 1];
  await send('Page.navigateToHistoryEntry', { entryId: previous.id }); await loaded(previous.url);
  await send('Page.navigateToHistoryEntry', { entryId: history.entries[history.currentIndex].id }); await loaded(direct);
  const length = await evaluate('history.length');
  await evaluate('scrollTo(0,0)'); await settled(); assert.equal(await evaluate('document.querySelector("#reading-progress").value'), 0);
  await evaluate('scrollTo(0,document.documentElement.scrollHeight)'); await settled(); assert.equal(await evaluate('document.querySelector("#reading-progress").value'), 100);
  assert.equal(await evaluate('history.length'), length);
  const theme = await evaluate('document.querySelector("#theme-toggle").getAttribute("aria-pressed")');
  await evaluate('document.querySelector("#theme-toggle").click()');
  assert.notEqual(await evaluate('document.querySelector("#theme-toggle").getAttribute("aria-pressed")'), theme);
  assert.equal(await evaluate('getComputedStyle(document.documentElement).scrollBehavior'), 'auto');
  for (const width of [320, 390]) {
    await viewport(width, true); await evaluate('scrollTo(0,0)'); await settled();
    assert.ok(await evaluate('document.documentElement.scrollWidth <= innerWidth + 1'), `Overflow at ${width}`);
    assert.equal(await evaluate('getComputedStyle(document.querySelector(".sidebar")).position'), 'static');
    await screenshot(`mobile-${width}.png`);
  }
  await viewport(1440);
  const keyboardUrl = `${url}&phase=keyboard`;
  await send('Page.navigate', { url: keyboardUrl }); await loaded(keyboardUrl);
  await send('Input.dispatchKeyEvent', { type: 'keyDown', key: 'Tab', code: 'Tab', windowsVirtualKeyCode: 9 });
  await send('Input.dispatchKeyEvent', { type: 'keyUp', key: 'Tab', code: 'Tab', windowsVirtualKeyCode: 9 });
  assert.equal(await evaluate('document.activeElement.className'), 'skip-link');
  assert.notEqual(await evaluate('getComputedStyle(document.activeElement).outlineStyle'), 'none');
  await send('Emulation.setScriptExecutionDisabled', { value: true });
  const noScriptUrl = `${url}&phase=no-script`;
  await send('Page.navigate', { url: noScriptUrl }); await loaded(noScriptUrl);
  assert.equal(await evaluate('document.documentElement.classList.contains("js")'), false);
  assert.ok(await evaluate('document.querySelector("main").innerText.length > 500 && document.querySelectorAll("table tr").length > 1'));
  assert.equal(await evaluate('getComputedStyle(document.querySelector(".toolbar")).display'), 'none');
  await evaluate(`document.querySelector('#report-toc a[href="${links[1]}"]').click()`); await settled(); assert.equal(await evaluate('location.hash'), links[1]);
  await send('Emulation.setScriptExecutionDisabled', { value: false });
  await send('Page.navigate', { url }); await loaded(url);
  await send('Emulation.setEmulatedMedia', { media: 'print' });
  assert.equal(await evaluate('getComputedStyle(document.querySelector(".sidebar")).display'), 'none');
  const pdf = await send('Page.printToPDF', { printBackground: true, preferCSSPageSize: true });
  await writeFile(join(directory, 'print.pdf'), Buffer.from(pdf.data, 'base64'), { flag: 'wx' });
  await send('Emulation.setEmulatedMedia', screen); await settled();
  assert.deepEqual(requests, []); assert.deepEqual(errors, []);
  const result = { ok: true, report: resolve(options['--report']), sections: links.length, http_requests: requests.length, browser_errors: errors.length, checks: ['anchors', 'direct hash', 'back/forward', 'scroll progress/history', 'theme', 'keyboard focus', 'mobile 320/390', 'reduced motion', 'JavaScript disabled', 'print PDF'], artifacts: directory };
  await writeFile(join(directory, 'verification.json'), JSON.stringify(result, null, 2) + '\n', { flag: 'wx' });
  console.log(JSON.stringify(result));
} catch (error) { console.error(error.message); console.log(JSON.stringify({ ok: false, error: error.message, artifacts: directory, requests, browser_errors: errors })); process.exitCode = 1; }
finally {
  if (session && socket?.readyState === WebSocket.OPEN) await Promise.allSettled([send('Emulation.setScriptExecutionDisabled', { value: false }), send('Emulation.setEmulatedMedia', { media: 'screen' })]);
  socket?.close();
}
