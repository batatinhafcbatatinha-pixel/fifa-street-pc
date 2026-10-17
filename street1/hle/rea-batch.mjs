// Minimal REA MCP client: opens a binary once and runs a batch of tool calls.
// usage: node rea-batch.mjs <binary> <calls.json> <out.json>
// calls.json: [{"name":"list_procedures","arguments":{...}}, ...]
import {spawn} from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const cli = path.resolve(here, '../../npm10-cache/_npx/af752b90495482d7/node_modules/rea-agents/scripts/rea.mjs');
const [binary, callsFile, outFile] = process.argv.slice(2);
const calls = JSON.parse(fs.readFileSync(callsFile, 'utf8'));
const child = spawn(process.execPath, [cli, 'mcp'], {windowsHide: true, env: {
  ...process.env,
  GHIDRA_INSTALL_DIR: 'C:/Users/dudua/Downloads/ghidra_12.1.4_PUBLIC_20260921/ghidra_12.1.4_PUBLIC',
  JAVA_HOME: 'C:/Program Files/Java/jdk-24',
  REA_GHIDRA_STARTUP_TIMEOUT_MS: '1800000',
}});
let seq = 0, buf = '', stderr = '';
const waiting = new Map();
child.stdout.on('data', d => {
  buf += d;
  let p;
  while ((p = buf.indexOf('\n')) >= 0) {
    const line = buf.slice(0, p); buf = buf.slice(p + 1);
    try { const m = JSON.parse(line); if (waiting.has(m.id)) { waiting.get(m.id)(m); waiting.delete(m.id); } } catch {}
  }
});
child.stderr.on('data', d => { stderr += d; if (stderr.length > 200000) stderr = stderr.slice(-100000); });
const request = (method, params, ms = 1800000) => new Promise((resolve, reject) => {
  const id = ++seq;
  const t = setTimeout(() => { waiting.delete(id); reject(Error('Timeout ' + method + '\n' + stderr.slice(-3000))); }, ms);
  waiting.set(id, m => { clearTimeout(t); resolve(m); });
  child.stdin.write(JSON.stringify({jsonrpc: '2.0', id, method, params}) + '\n');
});
const text = r => (r.result?.content || []).map(c => c.text).join('\n');
const results = [];
try {
  await request('initialize', {protocolVersion: '2024-11-05', capabilities: {}, clientInfo: {name: 'fifa-hle', version: '1'}});
  child.stdin.write(JSON.stringify({jsonrpc: '2.0', method: 'notifications/initialized'}) + '\n');
  const t0 = Date.now();
  const open = await request('tools/call', {name: 'open_binary', arguments: {path: path.resolve(binary), provider_id: 'ghidra'}});
  console.error(`open_binary ${((Date.now() - t0) / 1000).toFixed(1)}s`);
  results.push({name: 'open_binary', isError: !!open.result?.isError, text: text(open)});
  if (!open.result?.isError) {
    for (const c of calls) {
      const r = await request('tools/call', c);
      results.push({name: c.name, arguments: c.arguments, isError: !!r.result?.isError, text: text(r) || JSON.stringify(r.error)});
      console.error(`${c.name} done`);
    }
  }
} catch (e) {
  results.push({error: String(e)});
} finally {
  fs.writeFileSync(outFile, JSON.stringify(results, null, 1));
  child.stdin.end();
  setTimeout(() => child.kill(), 2000).unref();
}
