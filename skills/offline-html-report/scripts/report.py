#!/usr/bin/env python3
"""Create/check offline HTML reports. Python 3.10+, standard library only.

create copies bundled, already-inline template; never downloads dependencies.
check is a static guardrail, not a JavaScript sandbox or browser/a11y audit.
Piped/agent stdout is JSON; diagnostics use stderr. No interactive prompts.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from collections import Counter
from html.parser import HTMLParser
from pathlib import Path

SKILL_ROOT = Path(__file__).resolve().parent.parent
POLICY = (
    "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; "
    "img-src data:; font-src data:; media-src data:; connect-src 'none'; "
    "base-uri 'none'; form-action 'none'"
)
NETWORK_API = re.compile(r"\b(?:fetch\s*\(|XMLHttpRequest\b|WebSocket\b|EventSource\b|sendBeacon\s*\(|import\s*\(|importScripts\s*\(|serviceWorker\b)")
EXAMPLE_TEXT = "Template examples — not report findings"


class ReportParser(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.ids = []
        self.sections = []
        self.toc_links = []
        self.anchors = []
        self.chart_refs = []
        self.heading_refs = []
        self.policies = []
        self.problems = []
        self.scripts = {}
        self.styles = []
        self.text = []
        self.stack = []
        self.script = None
        self.style = None

    def handle_starttag(self, tag, attrs):
        attributes = dict(attrs)
        element_id = attributes.get('id')
        if element_id:
            self.ids.append(element_id)
        if tag == 'section' and 'data-report-section' in attributes:
            self.sections.append(element_id)
            self.heading_refs.append(attributes.get('aria-labelledby', ''))
        if tag == 'a':
            href = attributes.get('href', '')
            if href.startswith('#'):
                self.anchors.append(href[1:])
            if any(item[1] == 'report-toc' for item in self.stack):
                self.toc_links.append(href)
            if href.lower().startswith(('javascript:', 'file:')):
                self.problems.append('Links must use local anchors or ordinary source URLs, not JavaScript/file links.')
        if tag == 'meta' and attributes.get('http-equiv', '').lower() == 'content-security-policy':
            self.policies.append(attributes.get('content'))
        if tag in ('iframe', 'object', 'embed', 'base'):
            self.problems.append(f'<{tag}> is not allowed in offline report.')
        if tag == 'script' and attributes.get('src'):
            self.problems.append('Use inline script contents, not script src (including data URIs).')
        if tag == 'link' and attributes.get('href'):
            self.problems.append('Use inline styles/SVG favicon, not linked resources.')
        for name, value in attrs:
            if name.startswith('on') and value and NETWORK_API.search(value):
                self.problems.append(f'Network API found in {name}.')
            if name in ('src', 'poster', 'data') or (name in ('href', 'xlink:href') and tag in ('image', 'use', 'link')):
                if value and not value.startswith(('#', 'data:')):
                    self.problems.append(f'<{tag}> {name} must be inline/data URI; found {value[:100]}.')
            if name == 'srcset':
                self.problems.append('srcset not supported by static checker; use one embedded data-URI image.')
            if name == 'style' and value:
                self.styles.append(value)
        if tag == 'canvas' and 'data-chart' in attributes:
            self.chart_refs.append(attributes['data-chart'])
        if tag == 'script':
            key = element_id or f'unnamed-script-{len(self.scripts)}'
            self.script = key
            self.scripts[key] = {'type': attributes.get('type', ''), 'text': ''}
        if tag == 'style':
            self.style = len(self.styles)
            self.styles.append('')
        if tag not in {'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'param', 'source', 'track', 'wbr'}:
            self.stack.append((tag, element_id))

    def handle_startendtag(self, tag, attrs):
        self.handle_starttag(tag, attrs)
        self.handle_endtag(tag)

    def handle_endtag(self, tag):
        if tag == 'script':
            self.script = None
        if tag == 'style':
            self.style = None
        for index in range(len(self.stack) - 1, -1, -1):
            if self.stack[index][0] == tag:
                self.stack = self.stack[:index]
                break

    def handle_data(self, data):
        if self.script is not None:
            self.scripts[self.script]['text'] += data
        elif self.style is not None:
            self.styles[self.style] += data
        else:
            self.text.append(data)


def check_report(path: Path, allow_examples: bool = False) -> dict:
    source = path.read_text(encoding='utf-8')
    parser = ReportParser()
    parser.feed(source)
    parser.close()
    problems = parser.problems
    ids = set(parser.ids)
    for element_id, count in Counter(parser.ids).items():
        if count > 1:
            problems.append(f'Duplicate id: {element_id}.')
    if parser.stack:
        problems.append('Unclosed HTML elements: ' + ', '.join(tag for tag, _ in parser.stack))
    if not parser.sections or any(not item for item in parser.sections):
        problems.append('Every report section needs a unique id; at least one section required.')
    expected = ['#' + item for item in parser.sections if item]
    if parser.toc_links != expected:
        problems.append('Sidebar links must match report sections exactly, in reading order.')
    for anchor in parser.anchors:
        if anchor and anchor not in ids:
            problems.append(f'Missing anchor target: #{anchor}.')
    for heading in parser.heading_refs:
        if not heading or any(item not in ids for item in heading.split()):
            problems.append('Section aria-labelledby must name an existing heading.')
    if parser.policies != [POLICY]:
        problems.append('Keep exactly one bundled offline Content-Security-Policy unchanged.')
    required = {'report', 'report-toc', 'reading-progress', 'reading-status', 'theme-toggle', 'print-report', 'report-runtime', 'vendor-chartjs', 'chartjs-license'}
    missing = sorted(required - ids)
    if missing:
        problems.append('Missing template hooks: ' + ', '.join(missing))
    manifest = json.loads((SKILL_ROOT / 'assets/vendor.json').read_text(encoding='utf-8'))
    vendor = parser.scripts.get('vendor-chartjs', {}).get('text', '')
    if hashlib.sha256(vendor.encode('utf-8')).hexdigest() != manifest['embedded_js_sha256']:
        problems.append('Bundled Chart.js changed or missing; restore exact template vendor block.')
    for key, script in parser.scripts.items():
        if key == 'vendor-chartjs':
            continue
        if script['type'] == 'application/json':
            try:
                json.loads(script['text'])
            except json.JSONDecodeError:
                problems.append(f'Invalid embedded JSON: {key}.')
        elif NETWORK_API.search(script['text']):
            problems.append(f'Network API found in script {key}; remove runtime fetching.')
    for chart_ref in parser.chart_refs:
        script = parser.scripts.get(chart_ref)
        if not script or script['type'] != 'application/json':
            problems.append(f'Chart data must reference embedded application/json script: {chart_ref}.')
    for style in parser.styles:
        if re.search(r'@import\b', style, re.IGNORECASE):
            problems.append('CSS imports not allowed; inline stylesheet contents.')
        for value in re.findall(r'url\(\s*([^)]+)\s*\)', style, re.IGNORECASE):
            if not value.strip(' \t\r\n\"\'').startswith(('#', 'data:')):
                problems.append('CSS URL must be local fragment or data URI.')
    visible = '\n'.join(parser.text)
    license_text = (SKILL_ROOT / 'assets/chartjs-LICENSE.txt').read_text(encoding='utf-8')
    if ' '.join(license_text.split()) not in ' '.join(visible.split()):
        problems.append('Keep full bundled Chart.js license in delivered file.')
    if not allow_examples and EXAMPLE_TEXT in visible:
        problems.append('Template examples remain; replace with report evidence or remove unused components.')
    if re.search(r'\{\{[^{}]+\}\}|\b[A-Z_]+_PLACEHOLDER\b', visible):
        problems.append('Unfilled content placeholders remain.')
    return {
        'ok': not problems, 'path': str(path.resolve()), 'sections': len(parser.sections),
        'charts': len(parser.chart_refs), 'bytes': len(source.encode('utf-8')),
        'errors': list(dict.fromkeys(problems)),
        'unverified': ['browser rendering', 'network-blocked interactions', 'accessibility', 'print', 'content accuracy'],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    create = commands.add_parser('create', help='Copy self-contained template; replace sample content afterward.')
    create.add_argument('output', type=Path)
    create.add_argument('--overwrite', action='store_true', help='Explicitly allow replacing output.')
    create.add_argument('--json', action='store_true', help='Emit JSON, including errors.')
    check = commands.add_parser('check', help='Check static offline/template contract; browser tests still required.')
    check.add_argument('input', type=Path)
    check.add_argument('--allow-examples', action='store_true', help='Maintainer check of template; permit labelled sample content.')
    check.add_argument('--json', action='store_true', help='Emit JSON, including errors.')
    args = parser.parse_args()
    structured = args.json or not sys.stdout.isatty() or os.environ.get('OFFLINE_HTML_REPORT_AGENT') == '1' or bool(os.environ.get('CI'))
    try:
        if args.command == 'create':
            if args.output.suffix.lower() != '.html':
                raise ValueError('Output must end in .html.')
            text = (SKILL_ROOT / 'assets/report-template.html').read_text(encoding='utf-8')
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open('w' if args.overwrite else 'x', encoding='utf-8', newline='\n') as destination:
                destination.write(text)
            result = {'ok': True, 'path': str(args.output.resolve()), 'bytes': len(text.encode('utf-8')), 'next': 'Replace template examples, then run check on this file.'}
        else:
            result = check_report(args.input, args.allow_examples)
    except (OSError, ValueError) as error:
        result = {'ok': False, 'code': 'operation_failed', 'message': str(error)}
    if not result['ok']:
        result.setdefault('code', 'validation_failed')
        print(result.get('message', '\n'.join(result.get('errors', []))), file=sys.stderr)
    if structured:
        print(json.dumps(result, ensure_ascii=True))
    else:
        print(result.get('path', 'Operation failed') + (' — passed' if result['ok'] else ' — failed'))
    return 0 if result['ok'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
