"""Small lexical KB retrieval tests; NOT review correctness tests."""
from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

PATH = Path(__file__).resolve().parents[1] / "scripts" / "kb_lookup.py"
spec = importlib.util.spec_from_file_location("kb_lookup", PATH)
mod = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = mod
spec.loader.exec_module(mod)


class TestKBLookup(unittest.TestCase):
    def test_chinese_and_english_lookup(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "standards").mkdir()
            (root / "specifications").mkdir()
            (root / "standards" / "style.md").write_text("# Style\nUse explicit initialization.\n", encoding="utf-8")
            (root / "specifications" / "ability.md").write_text(
                "# Ability 生命周期\n终止回调应遵守生命周期状态转换要求。\n", encoding="utf-8")
            result = mod.search(root, "终止回调 生命周期", "both", 5)
            self.assertTrue(result["matches"])
            self.assertEqual("specifications/ability.md", result["matches"][0]["path"])
            self.assertEqual("specification", result["matches"][0]["source_kind"])
            self.assertEqual(2, result["matches"][0]["line"])
            result2 = mod.search(root, "initialization", "standard", 5)
            self.assertEqual("standards/style.md", result2["matches"][0]["path"])

    def test_missing_root(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(ValueError, "not found"):
                mod.search(Path(d), "ability")

    def test_token_extraction(self):
        t = mod.tokens("Ability 终止回调")
        self.assertIn("ability", t)
        self.assertIn("终止", t)


if __name__ == "__main__":
    unittest.main()
