"""Run: python <skill-dir>/scripts/test-pass-auth.py. Uses fake credentials only."""
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import time
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("pass_auth", Path(__file__).with_name("pass-auth.py"))
helper = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helper)


class AuthHandoffTests(unittest.TestCase):
    def test_only_selected_provider_travels_on_stdin(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "auth.json"
            credential = {"type": "oauth", "access": "fake-access", "refresh": "fake-refresh",
                          "expires": time.time() * 1000 + 600000}
            original = json.dumps({"openai": credential, "other": {"key": "fake-other"}})
            source.write_text(original)
            info = {"State": {"Running": True}, "Config": {"User": "node", "Labels": {"purpose": "herdr-pi-proof"}},
                    "HostConfig": {"Tmpfs": {helper.AGENT_DIR: "rw"}}}
            with patch.object(helper.subprocess, "run") as run:
                run.side_effect = [subprocess.CompletedProcess([], 0, json.dumps([info]).encode()),
                                   subprocess.CompletedProcess([], 0, b"")]
                result = helper.transfer(["child"], source, "openai")
                call = run.call_args_list[1]
                self.assertEqual(json.loads(call.kwargs["input"]), {"openai": credential})
                self.assertNotIn("fake-access", str(call.args))
                self.assertNotIn("fake-refresh", str(call.args))
                self.assertNotIn("fake-access", json.dumps(result))
                self.assertEqual(source.read_text(), original)

    def test_unowned_target_rejected_before_auth_read(self):
        info = {"Config": {"Labels": {}}}
        with patch.object(helper.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, json.dumps([info]).encode())), \
                patch.object(Path, "read_text") as read:
            with self.assertRaises(ValueError):
                helper.transfer(["unowned"], Path("auth.json"), "openai")
            read.assert_not_called()

    def test_expired_oauth_not_sent(self):
        info = {"State": {"Running": True}, "Config": {"User": "node", "Labels": {"purpose": "herdr-pi-proof"}},
                "HostConfig": {"Tmpfs": {helper.AGENT_DIR: "rw"}}}
        credential = {"type": "oauth", "access": "fake-access", "refresh": "fake-refresh", "expires": 0}
        with patch.object(helper.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, json.dumps([info]).encode())) as run, \
                patch.object(Path, "read_text", return_value=json.dumps({"openai": credential})):
            with self.assertRaises(ValueError):
                helper.transfer(["child"], Path("auth.json"), "openai")
            self.assertEqual(run.call_count, 1)

    def test_root_target_requires_explicit_permission(self):
        info = {"State": {"Running": True}, "Config": {"User": "0:0", "Labels": {"purpose": "herdr-pi-proof"}},
                "HostConfig": {"Tmpfs": {helper.AGENT_DIR: "rw"}}}
        credential = {"type": "api_key", "key": "fake-key"}
        with patch.object(helper.subprocess, "run") as run, \
                patch.object(Path, "read_text", return_value=json.dumps({"openai": credential})) as read:
            run.return_value = subprocess.CompletedProcess([], 0, json.dumps([info]).encode())
            with self.assertRaises(ValueError):
                helper.transfer(["child"], Path("auth.json"), "openai")
            read.assert_not_called()
            run.side_effect = [subprocess.CompletedProcess([], 0, json.dumps([info]).encode()),
                               subprocess.CompletedProcess([], 0, b"")]
            result = helper.transfer(["child"], Path("auth.json"), "openai", allow_root=True)
            self.assertEqual(result["status"], "provisioned")


if __name__ == "__main__":
    unittest.main()
