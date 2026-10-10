#!/usr/bin/env node
/** Node.js 22+, no packages. Test dedicated visible tab through existing Chromium CDP.
 * Usage: node browser-check.mjs --cdp-url ws://127.0.0.1:PORT/devtools/browser/ID
 *   [--report path.html] [--output-dir path]
 * Open headed browser first, e.g. agent-browser --headed open about:blank;
 * obtain endpoint using agent-browser get cdp-url. Leaves tested tab open.
 * Writes JSON on stdout, errors on stderr. Exits 0 pass, 1 check/runtime failure.
 */
import assert from 'node:assert/strict';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { resolve, dirname } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { parseArgs } from 'node:util';

let socket;
try {
  const { values } = parseArgs({ options: {
    'cdp-url': { type: 'string' }, report: { type: 'string' },
    'output-dir': { type: 'string' }, help: { type: 'boolean' },
  } });
  if (values.help) {
    console.log('Usage: node browser-check.mjs --cdp-url <browser-websocket> [--report <file.html>] [--output-dir <path>]');
    process.exit(0);
  }
  const endpoint = new URL(values['cdp-url'] || '');
  assert(['ws:', 'wss:'].includes(endpoint.protocol), 'Provide Chromium browser websocket URL with --cdp-url.');
  const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
  const path = resolve(values.report || resolve(root, 'assets/report-template.html'));
  await readFile(path); // Refuse missing report before creating browser tab.
  const output = resolve(values['output-dir'] || resolve(tmpdir(), `offline-report-browser-${Date.now()}`));
  await mkdir(output, { recursive: true });
  const url = pathToFileURL(path).href;
  socket = new WebSocket(endpoint);
  await new Promise((accept, reject) => {
    socket.addEventListener('open', accept, { once: true });
    socket.addEventListener('error', reject, { once: true });
  });
  let sequence = 0;
  let session;
  const pending = new Map();
  const requests = [];
  const errors = [];
  socket.addEventListener('message', event => {
    const message = JSON.parse(event.data);
    if (message.id) {
      const operation = pending.get(message.id);
      if (!operation) return;
      pending.delete(message.id);
      clearTimeout(operation.timer);
      if (message.error) operation.reject(new Error(message.error.message));
      else operation.accept(message.result);
    } else if (message.sessionId === session) {
      if (message.method === 'Network.requestWillBeSent') requests.push(message.params.request.url);
      if (message.method === 'Runtime.exceptionThrown') errors.push(message.params.exceptionDetails.text);
      if (message.method === 'Runtime.consoleAPICalled' && message.params.type === 'error') errors.push(JSON.stringify(message.params.args));
    }
  });
  function send(method, params = {}, scoped = true) {
    const id = ++sequence;
    return new Promise((accept, reject) => {
      const timer = setTimeout(() => { pending.delete(id); reject(new Error(`CDP timeout: ${method}`)); }, 15000);
      pending.set(id, { accept, reject, timer });
      socket.send(JSON.stringify({ id, method, params, ...(scoped && session ? { sessionId: session } : {}) }));
    });
  }
  const target = await send('Target.createTarget', { url: 'about:blank' }, false);
  session = (await send('Target.attachToTarget', { targetId: target.targetId, flatten: true }, false)).sessionId;
  await send('Page.enable');
  await send('Runtime.enable');
  await send('Network.enable');
  await send('Network.emulateNetworkConditions', { offline: true, latency: 0, downloadThroughput: 0, uploadThroughput: 0 });
  await send('Emulation.setDeviceMetricsOverride', { width: 1440, height: 1000, deviceScaleFactor: 1, mobile: false });
  await send('Emulation.setEmulatedMedia', { features: [{ name: 'prefers-color-scheme', value: 'light' }, { name: 'prefers-reduced-motion', value: 'reduce' }] });
  async function evaluate(expression) {
    const response = await send('Runtime.evaluate', { expression, awaitPromise: true, returnByValue: true });
    assert(!response.exceptionDetails, JSON.stringify(response.exceptionDetails));
    return response.result.value;
  }
  async function waitFor(expression) {
    const deadline = Date.now() + 8000;
    do {
      if (await evaluate(expression)) return;
      await new Promise(accept => setTimeout(accept, 50));
    } while (Date.now() < deadline);
    throw new Error(`Browser condition not met: ${expression}`);
  }
  async function screenshot(name) {
    const result = await send('Page.captureScreenshot', { format: 'png' });
    await writeFile(resolve(output, name), Buffer.from(result.data, 'base64'));
  }
  await send('Page.navigate', { url });
  await waitFor("document.readyState === 'complete' && document.documentElement.classList.contains('js')");
  assert.equal(await evaluate('Chart.version'), '4.5.1');
  assert(await evaluate('Object.keys(Chart.instances).length > 0'), 'Template chart must render offline.');
  assert.equal(await evaluate("getComputedStyle(document.documentElement).scrollBehavior"), 'auto');
  assert.equal(await evaluate("document.querySelector('progress').value"), 0);
  assert.equal(await evaluate('document.documentElement.scrollWidth > innerWidth'), false);
  await screenshot('desktop.png');

  await evaluate("document.querySelector('#report-toc a[href=\"#evidence\"]').focus()");
  await send('Input.dispatchKeyEvent', { type: 'keyDown', key: 'Enter', code: 'Enter', windowsVirtualKeyCode: 13 });
  await send('Input.dispatchKeyEvent', { type: 'keyUp', key: 'Enter', code: 'Enter', windowsVirtualKeyCode: 13 });
  await waitFor("location.hash === '#evidence' && document.querySelector('#report-toc [aria-current]').hash === '#evidence'");
  await evaluate("document.querySelector('#report-toc a[href=\"#diagram\"]').click()");
  await waitFor("location.hash === '#diagram' && document.querySelector('#report-toc [aria-current]').hash === '#diagram'");
  await evaluate('history.back()');
  await waitFor("location.hash === '#evidence' && document.querySelector('#report-toc [aria-current]').hash === '#evidence'");
  await evaluate('history.forward()');
  await waitFor("location.hash === '#diagram' && document.querySelector('#report-toc [aria-current]').hash === '#diagram'");
  const historyLength = await evaluate('history.length');
  await evaluate("window.scrollTo({top: document.documentElement.scrollHeight, behavior:'instant'})");
  await waitFor("document.querySelector('progress').value === 100 && document.querySelector('#report-toc [aria-current]').hash === '#sources'");
  assert.equal(await evaluate('history.length'), historyLength, 'Scrolling must not add history.');
  await send('Page.navigate', { url: url + '#diagram' });
  await waitFor("document.readyState === 'complete' && document.querySelector('#report-toc [aria-current]')?.hash === '#diagram'");

  await evaluate("document.getElementById('theme-toggle').click()");
  assert.equal(await evaluate('document.documentElement.dataset.theme'), 'dark');
  await screenshot('dark.png');
  await send('Emulation.setEmulatedMedia', { media: 'print' });
  assert.equal(await evaluate("getComputedStyle(document.body).backgroundColor"), 'rgb(255, 255, 255)');
  assert.equal(await evaluate("getComputedStyle(document.querySelector('.sidebar')).display"), 'none');
  const pdf = await send('Page.printToPDF', { printBackground: true });
  await writeFile(resolve(output, 'report.pdf'), Buffer.from(pdf.data, 'base64'));
  assert.equal(await evaluate("[...document.querySelectorAll('details')].every(detail => !detail.open)"), true, 'Print must restore disclosure state.');
  await send('Emulation.setEmulatedMedia', { media: 'screen' });
  await send('Emulation.setDeviceMetricsOverride', { width: 390, height: 844, deviceScaleFactor: 1, mobile: false });
  await evaluate("window.scrollTo({top:0,behavior:'instant'})");
  await waitFor("getComputedStyle(document.querySelector('.sidebar')).position === 'static' && document.documentElement.scrollWidth <= innerWidth");
  await screenshot('mobile.png');
  await send('Emulation.setDeviceMetricsOverride', { width: 320, height: 700, deviceScaleFactor: 1, mobile: false });
  await waitFor('document.documentElement.scrollWidth <= innerWidth');

  await send('Emulation.setScriptExecutionDisabled', { value: true });
  await send('Page.navigate', { url });
  await waitFor("document.readyState === 'complete' && !document.documentElement.classList.contains('js')");
  assert.equal(await evaluate("getComputedStyle(document.getElementById('theme-toggle').parentElement).display"), 'none');
  assert(await evaluate("document.querySelector('table').getBoundingClientRect().height > 0"));
  await screenshot('no-javascript.png');
  await send('Emulation.setScriptExecutionDisabled', { value: false });
  await send('Emulation.setDeviceMetricsOverride', { width: 1440, height: 1000, deviceScaleFactor: 1, mobile: false });
  await send('Page.navigate', { url });
  await waitFor("document.readyState === 'complete' && document.documentElement.classList.contains('js')");
  assert.deepEqual(errors, []);
  assert.deepEqual(requests.filter(item => /^https?:/i.test(item)), []);
  console.log(JSON.stringify({ ok: true, report: path, output, targetId: target.targetId, checks: ['offline Chart.js', 'keyboard anchors', 'scrollspy/progress', 'Back/Forward', 'direct hash', 'no scroll-history spam', 'theme', 'reduced motion', 'print PDF', 'mobile 390/320', 'JavaScript disabled', 'no HTTP(S) requests', 'no runtime errors'] }));
} catch (error) {
  console.error(error.message);
  console.log(JSON.stringify({ ok: false, error: error.message }));
  process.exitCode = 1;
} finally {
  socket?.close();
}
