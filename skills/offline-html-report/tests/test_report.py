#!/usr/bin/env python3
"""Run with python tests/test_report.py. No packages/network/browser required."""
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'scripts/report.py'
spec = importlib.util.spec_from_file_location('offline_report', SCRIPT)
report = importlib.util.module_from_spec(spec)
spec.loader.exec_module(report)


class ReportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.template = (ROOT / 'assets/report-template.html').read_text(encoding='utf-8')

    def check_content(self, content, allow_examples=True):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'report.html'
            path.write_text(content, encoding='utf-8')
            return report.check_report(path, allow_examples)

    def test_bundled_template(self):
        result = self.check_content(self.template)
        self.assertTrue(result['ok'], result['errors'])
        self.assertEqual(result['sections'], 5)
        self.assertEqual(result['charts'], 1)

    def test_delivery_rejects_examples(self):
        result = self.check_content(self.template, allow_examples=False)
        self.assertFalse(result['ok'])
        self.assertIn('Template examples remain', '\n'.join(result['errors']))

    def test_remote_resource_rejected(self):
        for element in ['<script src="https://example.test/a.js"></script>', '<script src="data:text/javascript,void(0)"></script>', '<img src="sibling.png">', '<link rel="stylesheet" href="https://example.test/a.css">']:
            with self.subTest(element=element):
                self.assertFalse(self.check_content(self.template.replace('</head>', element + '</head>'))['ok'])

    def test_css_and_dynamic_network_rejected(self):
        for element in ['<style>body{background:url(https://example.test/a.png)}</style>', '<style>@import "x.css";</style>', '<script>fetch("https://example.test")</script>', '<script>new WebSocket("wss://example.test")</script>']:
            with self.subTest(element=element):
                self.assertFalse(self.check_content(self.template.replace('</head>', element + '</head>'))['ok'])

    def test_csp_and_vendor_guard(self):
        self.assertFalse(self.check_content(self.template.replace("connect-src 'none'", "connect-src *"))['ok'])
        self.assertFalse(self.check_content(self.template.replace('<script id="vendor-chartjs">', '<script id="vendor-chartjs">/* modified */'))['ok'])

    def test_vendor_license_preserved(self):
        changed = self.template.replace('THE SOFTWARE IS PROVIDED', 'LICENSE OMITTED', 1)
        self.assertFalse(self.check_content(changed)['ok'])

    def test_anchor_and_toc_guard(self):
        changes = [
            ('href="#summary"', 'href="#missing"'),
            ('id="summary"', 'id="sources"'),
            ('id="summary"', ''),
            ('aria-labelledby="summary-heading"', 'aria-labelledby="missing-heading"'),
        ]
        for old, new in changes:
            with self.subTest(old=old):
                self.assertFalse(self.check_content(self.template.replace(old, new, 1))['ok'])

    def test_invalid_json(self):
        changed = self.template.replace('{"type":"bar"', '{invalid:"bar"', 1)
        self.assertFalse(self.check_content(changed)['ok'])

    def test_template_hook_missing(self):
        self.assertFalse(self.check_content(self.template.replace('id="reading-progress"', 'id="other-progress"'))['ok'])

    def test_safe_embedded_image(self):
        changed = self.template.replace('</main>', '<img alt="Small example" src="data:image/png;base64,AAAA"></main>')
        self.assertTrue(self.check_content(changed)['ok'])

    def test_cli_copy_overwrite_and_json(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'nested' / 'résultats.html'
            command = [sys.executable, str(SCRIPT), 'create', str(path), '--json']
            created = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(created.returncode, 0, created.stderr)
            self.assertTrue(json.loads(created.stdout)['ok'])
            self.assertEqual(path.read_text(encoding='utf-8'), self.template)
            refused = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(refused.returncode, 1)
            self.assertFalse(json.loads(refused.stdout)['ok'])
            overwritten = subprocess.run(command + ['--overwrite'], capture_output=True, text=True)
            self.assertEqual(overwritten.returncode, 0, overwritten.stderr)
            checked = subprocess.run([sys.executable, str(SCRIPT), 'check', str(path), '--allow-examples'], capture_output=True, text=True)
            self.assertEqual(checked.returncode, 0, checked.stderr)
            self.assertTrue(json.loads(checked.stdout)['ok'])

    def test_cli_missing_file_and_wrong_extension(self):
        for args in [['check', 'missing-report.html'], ['create', 'report.txt']]:
            with self.subTest(args=args):
                result = subprocess.run([sys.executable, str(SCRIPT)] + args + ['--json'], capture_output=True, text=True)
                self.assertEqual(result.returncode, 1)
                self.assertFalse(json.loads(result.stdout)['ok'])
                self.assertTrue(result.stderr)

    def test_cli_help_missing_args_and_agent_mode(self):
        help_result = subprocess.run([sys.executable, str(SCRIPT), '--help'], capture_output=True, text=True)
        self.assertEqual(help_result.returncode, 0)
        missing = subprocess.run([sys.executable, str(SCRIPT)], capture_output=True, text=True)
        self.assertEqual(missing.returncode, 2)
        environment = dict(os.environ, OFFLINE_HTML_REPORT_AGENT='1')
        result = subprocess.run([sys.executable, str(SCRIPT), 'check', str(ROOT / 'assets/report-template.html'), '--allow-examples'], env=environment, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue(json.loads(result.stdout)['ok'])


if __name__ == '__main__':
    unittest.main()
