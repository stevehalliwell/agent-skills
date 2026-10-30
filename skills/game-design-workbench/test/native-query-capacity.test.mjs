import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { resolve, join } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
const executable=process.env.GD_NATIVE_EXE && resolve(process.env.GD_NATIVE_EXE);
test('effect adapter retains bounded Resource evidence instead of inventing allocation failure',{skip:!executable},async()=>{
  const root=await mkdtemp(join(tmpdir(),'gd-query-capacity-'));
  try {
    for (const role of ['nouns','axes','mechanics']) await mkdir(join(root,role));
    await writeFile(join(root,'axes','Life.md'),'---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n');
    await writeFile(join(root,'nouns','unit.md'),'---\naxes: [axes/Life]\n---\n');
    const cases=Array.from({length:32},(_,index)=>`case c${index}:\n    target: Victim\n    context: ANY\n    Victim:\n        noun: nouns/unit\n        requires: ANY\n        becomes: ${Array(256).fill('Life.dead').join(', ')}\n`).join('');
    await writeFile(join(root,'mechanics','template.md'),'---\nstatus: accepted\n---\n```gd-rule\n'+cases+'```\n');
    await writeFile(join(root,'mechanics','caller.md'),'---\nstatus: accepted\n---\n```gd-rule\ncase root:\n    target: A\n    context: ANY\n    A:\n        noun: nouns/unit\n        requires: ANY\n        causes: '+Array(3).fill('mechanics/template with Victim=A').join('; ')+'\n```\n');
    const inputs=['nouns','axes','mechanics'].flatMap(role=>['--'+role,`${role}=${join(root,role)}`]);
    const result=spawnSync(executable,['query','--question','effect-contributors','--projection','effects',...inputs,'--axis','axes/Life','--value','dead','--fields','concepts'],{encoding:'utf8',timeout:30000,maxBuffer:4*1024*1024,windowsHide:true,env:{...process.env,GD_AGENT:'1'}});
    assert.equal(result.status,0,result.stderr+'\n'+result.stdout); const value=JSON.parse(result.stdout);
    assert.equal(value.ok,true); assert.deepEqual(value.errors,[]); assert.deepEqual(Object.keys(value.data),['concepts']);
    assert.equal(value.coverage.complete,false); assert.equal(value.coverage.limited,true); assert.equal(value.coverage.resource_limited,true);
    assert.equal(value.coverage.resource_relations,32768); assert.equal(value.coverage.source_incomplete,true);
    assert.equal(value.data.concepts.length,1); assert.ok(value.data.concepts[0].incoming>8192);
    assert.equal(value.data.concepts[0].incoming,value.coverage.retained_contributors);
  } finally { await rm(root,{recursive:true,force:true}); }
});
