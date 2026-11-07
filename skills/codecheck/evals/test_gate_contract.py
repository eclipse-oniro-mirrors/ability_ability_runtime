"""Contract smoke tests (stdlib unittest; actual YAML validation uses PyYAML)."""
from __future__ import annotations

import importlib.util
import json
import sys
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "validate_report.py"
spec = importlib.util.spec_from_file_location("codecheck_validator", SCRIPT)
mod = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = mod
spec.loader.exec_module(mod)


def make_meta(counts=None, executed=None):
    counts = counts or {"P0": 0, "P1": 0, "P2": 0, "P3": 0}
    required = ["behavior", "security"]
    executed = required if executed is None else executed
    score = max(0, 100 - 30 * counts["P0"] - 12 * counts["P1"] - 5 * counts["P2"] - 2 * counts["P3"])
    gate, risk = mod.expected_decision(counts, set(required), set(executed), score)
    ids = {p: [f"CC-{i:03d}" for i in range(1 + sum(counts[q] for q in ["P0", "P1", "P2", "P3"][:k]),
                                                      1 + sum(counts[q] for q in ["P0", "P1", "P2", "P3"][:k+1]))]
           for k, p in enumerate(["P0", "P1", "P2", "P3"])}
    return {
        "schema_version": "2.0", "scope": "example", "round": 1,
        "commit_id": "a" * 40, "change_id": "N/A", "report_id": "aaaaaaaa-R1", "date": "2026-11-06",
        "gate_decision": gate, "risk_level": risk, "score": score,
        "dimensions_required": required, "dimensions_executed": executed,
        "findings_total": sum(counts.values()), "findings_by_severity": counts,
        "gate_blockers": [*sorted(set(required) - set(executed))] if gate == "insufficient" else ids["P0"] if gate == "block" else [],
        "must_fix": ids["P1"] if gate == "conditional" else [],
        "followups": ids["P2"] + ids["P3"],
    }


def make_report(meta):
    text = ("# 代码检视报告\n\n"
            + mod.START + "\n```yaml\n"
            + mod.yaml.safe_dump({"codecheck_report": meta}, sort_keys=False, allow_unicode=True)
            + "```\n" + mod.END + "\n\n")
    text += ("## 1. 门禁结论\n\n| 项目 | 结论 |\n|---|---|\n"
             + f"| 决策 | **{meta['gate_decision']}** |\n"
             + f"| 风险等级 | {meta['risk_level']} |\n"
             + f"| 评分 | **{meta['score']}/100** |\n")
    if meta["gate_decision"] == "block":
        text += "\n## 2. 扣分原因\n\n...\n"
    text += ("\n## 3. 必须立即处理（P0/P1）\n\n"
             "| ID | 优先级 | 问题 | file:line | 触发路径 | 影响 |\n"
             "|---|---|---|---|---|---|\n"
             "\n## 4. 建议本轮或下一轮处理（P2/P3）\n\n"
             "\n## 5. 分维度速览\n\n"
             "\n## 6. 关键发现详情\n\n")
    if meta["findings_total"]:
        text += "### [CC-001] 示例 (P0)\n- **触发路径**：source→sink\n"
    return text


def make_trace(meta):
    verdicts = []
    ids = []
    for i in range(meta["findings_total"]):
        fid = f"CC-{i+1:03d}"
        ids.append(fid)
        verdicts.append({"candidate_id": f"K-{i+1:03d}", "verdict": "confirmed", "concern_ids": ["C01"],
                         "evidence": ["example.cpp:10"], "reason": "source-verified", "finding_id": fid})
    return {
        "schema_version": "2.0", "review_mode": "diff", "scope": "example",
        "base_sha": "b" * 40, "head_sha": "a" * 40,
        "knowledge": {"status": "not_needed", "revision": "N/A", "references": []},
        "concerns": [{"id": "C01", "summary": "example", "changed_locations": ["example.cpp:10"],
                      "discover": "done", "challenge": "done", "coverage_critical": False,
                      "limitations": [], "related_findings": ids}],
        "hypotheses": [{"id": "H-C01-01", "concern_ids": ["C01"], "invariant": "example invariant",
                        "outcome": "refuted", "feasibility": "refuted", "test_mode": "source_reasoning",
                        "test_result": "not_run"}], "candidate_verdicts": verdicts,
        "capability_coverage": {"required": meta["dimensions_required"], "executed": meta["dimensions_executed"],
                                "evidence": {k: "C01 example.cpp:10" for k in meta["dimensions_executed"]}},
        "independent_verifier": False, "limitations": [], "report_findings": ids,
    }


@unittest.skipIf(mod.yaml is None, "PyYAML not installed")
class TestReport(unittest.TestCase):
    def test_approve(self):
        m = make_meta()
        self.assertEqual([], mod.check_metadata(m, make_report(m)))
        self.assertEqual([], mod.check_trace(m, make_trace(m)))

    def test_block(self):
        m = make_meta({"P0": 1, "P1": 0, "P2": 0, "P3": 0})
        self.assertEqual("block", m["gate_decision"])
        self.assertEqual([], mod.check_metadata(m, make_report(m)))
        self.assertEqual([], mod.check_trace(m, make_trace(m)))

    def test_score_only_block_has_no_p0_blockers(self):
        # Multiple confirmed low-severity issues can exhaust the score.
        m = make_meta({"P0": 0, "P1": 0, "P2": 7, "P3": 0})
        self.assertEqual("block", m["gate_decision"])
        self.assertEqual([], m["gate_blockers"])
        self.assertEqual([], mod.check_metadata(m, make_report(m)))
        self.assertEqual([], mod.check_trace(m, make_trace(m)))

    def test_insufficient(self):
        m = make_meta(executed=["behavior"])
        self.assertEqual("insufficient", m["gate_decision"])
        self.assertEqual([], mod.check_metadata(m, make_report(m)))

    def test_wrong_score(self):
        m = make_meta()
        m["score"] = 97
        self.assertTrue(any("score must" in x for x in mod.check_metadata(m, make_report(m))))

    def test_critical_limited(self):
        m = make_meta()
        trace = make_trace(m)
        trace["concerns"][0].update(discover="limited", coverage_critical=True)
        self.assertTrue(any("critical limited" in x for x in mod.check_trace(m, trace)))

    def test_low_risk_concern_may_skip_challenge_with_reason(self):
        """Adaptive Challenge has no per-Concern counterexample quota."""
        m = make_meta()
        t = make_trace(m)
        t["concerns"].append({
            "id": "C02", "summary": "format-only change",
            "changed_locations": ["example.cpp:21"],
            "discover": "done", "challenge": "not_applicable",
            "challenge_justification": "Verified that only whitespace changed",
            "coverage_critical": False, "limitations": [], "related_findings": [],
        })
        self.assertEqual([], mod.check_trace(m, t))

    def test_done_challenge_requires_actual_hypothesis(self):
        m = make_meta()
        t = make_trace(m)
        t["hypotheses"] = []
        self.assertTrue(any("challenge done needs" in x for x in mod.check_trace(m, t)))

    def test_skipped_challenge_requires_reason(self):
        m = make_meta()
        t = make_trace(m)
        t["concerns"][0]["challenge"] = "not_applicable"
        t["hypotheses"] = []
        self.assertTrue(any("needs reason" in x for x in mod.check_trace(m, t)))

    def test_bad_verdict(self):
        m = make_meta({"P0": 0, "P1": 1, "P2": 0, "P3": 0})
        trace = make_trace(m)
        trace["candidate_verdicts"][0]["verdict"] = "refuted"
        self.assertTrue(any("non-confirmed" in x for x in mod.check_trace(m, trace)))


if __name__ == "__main__":
    unittest.main()
