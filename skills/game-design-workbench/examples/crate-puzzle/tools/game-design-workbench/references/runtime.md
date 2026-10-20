# Runtime

Before helper commands, load separately installed `attendant` skill and follow its operation instructions. Requires Node.js `>=24.10.0`. Set `GD_ATTENDANT_SKILL` to that skill's absolute directory:

```sh
export GD_ATTENDANT_SKILL="/absolute/path/to/attendant"
```

PowerShell:

```powershell
$env:GD_ATTENDANT_SKILL = 'C:/absolute/path/to/attendant'
```

Workbench calls Attendant's public runner. Never copy/vendor its code or guidance, import private modules, or install it automatically. No npm dependency, network service, compiler, or display needed for current helpers. Missing/invalid dependency fails `ATTENDANT_REQUIRED` before source writes.

Concept history requires local Git supporting `--no-lazy-fetch`. Current concept text/mode remains readable without Git.

Command placeholders:
- `<workbench>`: supplied skill directory or project's `tools/game-design-workbench` snapshot.
- `<attendant-skill>`: loaded Attendant skill directory.
- `<project>`: target project root; not shell working directory unless identical.

Windows / Node.js 24.16.0 verified. Other platforms are not claimed tested.
