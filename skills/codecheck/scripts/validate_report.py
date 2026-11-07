#!/usr/bin/env python3
"""Validate Codecheck V2 output structure and gate contract.

Usage:
  python3 skills/codecheck/scripts/validate_report.py path/to/report.md \
      --trace path/to/review_trace.json

Needs PyYAML (python3 -m pip install pyyaml). This tool checks machine/schema
consistency, not the truth of the code findings.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import re
import sys
from pathlib import Path

try:
    import yaml
except ImportError:
    yaml = None

START = "<!-- codecheck-report-metadata:start -->"
END = "<!-- codecheck-report-metadata:end -->"
KEYS = (
    "schema_version", "scope", "round", "commit_id", "change_id",
    "report_id", "date", "gate_decision", "risk_level", "score",
    "dimensions_required", "dimensions_executed", "findings_total",
    "findings_by_severity", "gate_blockers", "must_fix", "followups",
)
CAPABILITIES = {"behavior", "security", "input", "api"}
LEVELS = {"P0", "P1", "P2", "P3"}
STAGES = {"done", "limited", "not_applicable"}
DECISIONS = {"approve", "conditional", "block", "insufficient"}
RISKS = {"low", "medium", "high", "unknown"}


def issue(errors: list[str], message: str) -> None:
    errors.append(message)


def unique_strings(value: object, label: str, errors: list[str]) -> list[str]:
    if not isinstance(value, list) or any(not isinstance(x, str) for x in value):
        issue(errors, f"{label} must be a list of strings")
        return []
    if len(value) != len(set(value)):
        issue(errors, f"{label} has duplicated values")
    return value


def metadata_from_report(content: str) -> dict:
    if START not in content or END not in content:
        raise ValueError("missing codecheck-report-metadata markers")
    inside = content.split(START, 1)[1].split(END, 1)[0].strip()
    match = re.fullmatch(r"```yaml\s*\n(.*?)\n```", inside, flags=re.S)
    if not match:
        raise ValueError("expected one fenced YAML block between metadata markers")
    if yaml is None:
        raise ValueError("PyYAML is required to validate YAML (python3 -m pip install pyyaml)")
    parsed = yaml.safe_load(match.group(1))
    if not isinstance(parsed, dict) or list(parsed) != ["codecheck_report"]:
        raise ValueError("YAML root must be exactly codecheck_report")
    value = parsed["codecheck_report"]
    if not isinstance(value, dict):
        raise ValueError("codecheck_report must be a mapping")
    return value


def expected_decision(counts: dict[str, int], required: set[str], executed: set[str], score: int) -> tuple[str, str]:
    if required - executed:
        return "insufficient", "unknown"
    if counts["P0"]:
        return "block", "high"
    if counts["P1"]:
        return "conditional", "medium"
    if score >= 90:
        return "approve", "low"
    if score >= 70:
        return "conditional", "medium"
    return "block", "high"


def check_metadata(meta: dict, content: str) -> list[str]:
    errors: list[str] = []
    actual = set(meta)
    if actual != set(KEYS):
        issue(errors, f"YAML keys mismatch: missing={sorted(set(KEYS)-actual)}, extra={sorted(actual-set(KEYS))}")

    if meta.get("schema_version") != "2.0":
        issue(errors, "schema_version must be the string '2.0'")
    if not isinstance(meta.get("scope"), str) or not meta.get("scope"):
        issue(errors, "scope must be a nonempty string")
    rd = meta.get("round")
    if type(rd) is not int or rd <= 0:
        issue(errors, "round must be a positive integer")
    commit = meta.get("commit_id")
    if not isinstance(commit, str) or not re.fullmatch(r"(?:[0-9a-fA-F]{40}|N/A)", commit):
        issue(errors, "commit_id must be a full 40-char SHA or N/A")
    change = meta.get("change_id")
    if not isinstance(change, str) or not re.fullmatch(r"(?:I[0-9a-fA-F]{40}|N/A)", change):
        issue(errors, "change_id must be Change-Id I<40 hex> or N/A")
    expected_id_prefix = (change if change != "N/A" else (commit[:8] if isinstance(commit, str) and commit != "N/A" else "N/A"))
    if isinstance(rd, int) and meta.get("report_id") != f"{expected_id_prefix}-R{rd}":
        issue(errors, f"report_id does not match {expected_id_prefix}-R{rd}")
    date = meta.get("date")
    try:
        if not isinstance(date, str):
            raise ValueError("not a quoted date string")
        if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", date):
            raise ValueError("wrong format")
        dt.datetime.strptime(date, "%Y-%m-%d")
    except (ValueError, TypeError):
        issue(errors, "date must be a quoted YYYY-MM-DD string")

    required = unique_strings(meta.get("dimensions_required"), "dimensions_required", errors)
    executed = unique_strings(meta.get("dimensions_executed"), "dimensions_executed", errors)
    if not set(required).issubset(CAPABILITIES) or not set(executed).issubset(CAPABILITIES):
        issue(errors, "dimensions may only use known capability names (behavior/security/input/api)")
    if not set(executed).issubset(set(required)):
        issue(errors, "dimensions_executed must be a subset of dimensions_required")
    if not {"behavior", "security"}.issubset(set(required)):
        issue(errors, "dimensions_required must include baseline capabilities behavior and security")

    counts = meta.get("findings_by_severity")
    good_counts = isinstance(counts, dict) and set(counts) == LEVELS and all(type(v) is int and v >= 0 for v in counts.values())
    if not good_counts:
        issue(errors, "findings_by_severity must include exactly nonnegative integer P0/P1/P2/P3")
        counts = dict.fromkeys(LEVELS, 0)
    count_total = sum(counts.values())
    if type(meta.get("findings_total")) is not int or meta["findings_total"] != count_total:
        issue(errors, f"findings_total must equal {count_total}")
    score = max(0, 100 - 30 * counts["P0"] - 12 * counts["P1"] - 5 * counts["P2"] - 2 * counts["P3"])
    if type(meta.get("score")) is not int or meta["score"] != score:
        issue(errors, f"score must equal {score}")
    gate, risk = expected_decision(counts, set(required), set(executed), score)
    if meta.get("gate_decision") not in DECISIONS or meta.get("gate_decision") != gate:
        issue(errors, f"gate_decision must equal {gate}")
    if meta.get("risk_level") not in RISKS or meta.get("risk_level") != risk:
        issue(errors, f"risk_level must equal {risk}")

    for name in ("gate_blockers", "must_fix", "followups"):
        unique_strings(meta.get(name), name, errors)
    blockers = meta.get("gate_blockers") if isinstance(meta.get("gate_blockers"), list) else []
    must_fix = meta.get("must_fix") if isinstance(meta.get("must_fix"), list) else []
    followups = meta.get("followups") if isinstance(meta.get("followups"), list) else []
    if gate == "insufficient" and set(blockers) != set(required) - set(executed):
        issue(errors, "gate_blockers for insufficient must be exactly missing required capabilities")
    if gate not in {"insufficient", "block"} and blockers:
        issue(errors, "gate_blockers must be empty for approve/conditional")
    if gate == "block":
        if counts["P0"] > 0 and len(blockers) != counts["P0"]:
            issue(errors, "gate_blockers for block must contain all confirmed P0 IDs")
        if counts["P0"] == 0 and blockers:
            issue(errors, "gate_blockers for score-only block must be []")
    if gate == "conditional" and len(must_fix) != counts["P1"]:
        issue(errors, "must_fix for conditional must contain P1 IDs")
    if gate != "conditional" and must_fix:
        issue(errors, "must_fix must be [] unless gate is conditional")
    if len(followups) != counts["P2"] + counts["P3"]:
        issue(errors, "followups must contain all confirmed P2/P3 IDs")
    finding_ids = must_fix + followups + (blockers if gate == "block" else [])
    if any(not isinstance(x, str) or not re.fullmatch(r"CC-\d{3,}", x) for x in finding_ids):
        issue(errors, "finding IDs should use CC-001 format")

    headings = [(m.group(1), m.start()) for m in re.finditer(r"^## (\d+)\. ", content, flags=re.M)]
    actual_headings = [int(n) for n, _ in headings]
    expected_headings = [1, 2, 3, 4, 5, 6] if gate == "block" else [1, 3, 4, 5, 6]
    if actual_headings != expected_headings:
        issue(errors, f"expected section numbering/order {expected_headings}, got {actual_headings}")
    first_sec = re.search(r"^## 1\. .*?(?=^## \d+\. |\Z)", content, flags=re.S | re.M)
    if first_sec:
        sec = first_sec.group(0)
        if f"**{gate}**" not in sec:
            issue(errors, "section 1 missing computed gate_decision in **bold**")
        if f"**{score}/100**" not in sec:
            issue(errors, "section 1 missing computed score")
        if risk not in sec:
            issue(errors, "section 1 missing risk_level")
    if "| ID | 优先级 | 问题 | file:line | 触发路径 | 影响 |" not in content:
        issue(errors, "section 3 missing required header columns")
    if count_total > 0 and "- **触发路径**：" not in content:
        issue(errors, "confirmed findings require at least one detailed evidence card")
    return errors


def check_trace(meta: dict, trace: object) -> list[str]:
    errors: list[str] = []
    if not isinstance(trace, dict):
        return ["trace must be a JSON object"]
    if trace.get("schema_version") != "2.0":
        issue(errors, "trace.schema_version must be '2.0'")
    if trace.get("scope") != meta.get("scope"):
        issue(errors, "trace.scope differs from machine report scope")
    if trace.get("review_mode") not in {"diff", "snapshot", "path-audit"}:
        issue(errors, "trace.review_mode must be diff, snapshot or path-audit")
    if not isinstance(trace.get("independent_verifier"), bool):
        issue(errors, "trace.independent_verifier must be true or false")
    knowledge = trace.get("knowledge")
    if not isinstance(knowledge, dict) or knowledge.get("status") not in {"available", "partial", "unavailable", "not_needed"} or not isinstance(knowledge.get("references"), list):
        issue(errors, "trace.knowledge requires status and references list")
    for key in ("base_sha", "head_sha"):
        if not isinstance(trace.get(key), str) or not trace.get(key):
            issue(errors, f"trace.{key} required")
    coverage = trace.get("capability_coverage", {})
    if not isinstance(coverage, dict):
        coverage = {}
    if coverage.get("required") != meta.get("dimensions_required") or coverage.get("executed") != meta.get("dimensions_executed"):
        issue(errors, "trace capability projection differs from YAML dimensions")
    evidence = coverage.get("evidence")
    if not isinstance(evidence, dict):
        evidence = {}
    for alias in meta.get("dimensions_executed", []):
        if not evidence.get(alias):
            issue(errors, f"executed capability {alias} needs actual evidence or explicit N/A rationale")

    concerns = trace.get("concerns")
    if not isinstance(concerns, list) or not concerns:
        issue(errors, "trace.concerns must contain at least one Concern")
        concerns = []
    ids: list[str] = []
    for c in concerns:
        if not isinstance(c, dict):
            issue(errors, "each Concern must be an object")
            continue
        cid = c.get("id")
        if not isinstance(cid, str) or not re.fullmatch(r"C\d{2,}", cid):
            issue(errors, f"bad concern id: {cid!r}")
        else:
            ids.append(cid)
        if c.get("discover") not in STAGES or c.get("challenge") not in STAGES:
            issue(errors, f"{cid}: discover/challenge must be done, limited or not_applicable")
        if c.get("challenge") == "not_applicable" and not c.get("challenge_justification"):
            issue(errors, f"{cid}: challenge not_applicable needs reason")
        if not isinstance(c.get("coverage_critical"), bool):
            issue(errors, f"{cid}: coverage_critical must be boolean")
        if not isinstance(c.get("changed_locations"), list) or not c.get("changed_locations"):
            issue(errors, f"{cid}: changed_locations required")
        if c.get("coverage_critical") is True and (c.get("discover") == "limited" or c.get("challenge") == "limited") and meta.get("gate_decision") != "insufficient":
            issue(errors, f"{cid}: critical limited coverage requires insufficient gate")
    if len(ids) != len(set(ids)):
        issue(errors, "duplicate Concern IDs")

    hypotheses = trace.get("hypotheses")
    if not isinstance(hypotheses, list):
        issue(errors, "trace.hypotheses must be an array")
        hypotheses = []
    for c in concerns:
        if not isinstance(c, dict) or c.get("challenge") != "done":
            continue
        if not any(isinstance(h, dict) and c.get("id") in h.get("concern_ids", []) and h.get("invariant") and h.get("outcome") in {"candidate", "refuted", "unresolved"} for h in hypotheses):
            issue(errors, f"{c.get('id')}: challenge done needs at least one documented counterexample hypothesis")

    report_findings = unique_strings(trace.get("report_findings"), "trace.report_findings", errors)
    if len(report_findings) != meta.get("findings_total"):
        issue(errors, "trace.report_findings count differs from YAML findings_total")
    if len(report_findings) != len(set(report_findings)):
        issue(errors, "duplicate final finding IDs")
    verdicts = trace.get("candidate_verdicts")
    if not isinstance(verdicts, list):
        issue(errors, "candidate_verdicts must be an array")
        verdicts = []
    approved_ids: list[str] = []
    for v in verdicts:
        if not isinstance(v, dict):
            issue(errors, "candidate verdict must be object")
            continue
        if v.get("verdict") not in {"confirmed", "refuted", "unresolved", "out_of_scope", "merged"}:
            issue(errors, f"candidate {v.get('candidate_id')} invalid verdict")
        if not v.get("candidate_id") or not isinstance(v.get("concern_ids"), list) or not v.get("reason") or not isinstance(v.get("evidence"), list):
            issue(errors, "candidate verdict missing claim linkage/reason/evidence")
        fid = v.get("finding_id")
        if v.get("verdict") == "confirmed":
            if not fid:
                issue(errors, f"confirmed {v.get('candidate_id')} missing finding_id")
            else:
                approved_ids.append(fid)
        elif fid is not None:
            issue(errors, f"non-confirmed {v.get('candidate_id')} should not claim formal finding_id")
    if sorted(set(approved_ids)) != sorted(set(report_findings)):
        issue(errors, "confirmed verdict IDs do not reconcile with trace.report_findings")
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    parser.add_argument("--trace", type=Path, help="V2 review_trace JSON file")
    args = parser.parse_args(argv)
    try:
        content = args.report.read_text(encoding="utf-8")
        meta = metadata_from_report(content)
        errors = check_metadata(meta, content)
        if args.trace:
            trace = json.loads(args.trace.read_text(encoding="utf-8"))
            errors.extend(check_trace(meta, trace))
        else:
            print("WARNING: no --trace provided; V2 concern/evidence coverage not checked", file=sys.stderr)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 2

    for error in errors:
        print(f"FAIL: {error}", file=sys.stderr)
    if errors:
        print(f"{len(errors)} validation error(s)", file=sys.stderr)
        return 1
    print("PASS: report metadata, gate arithmetic, section contract" + (" and V2 trace" if args.trace else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
