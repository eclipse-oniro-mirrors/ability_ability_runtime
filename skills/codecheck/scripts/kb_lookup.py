#!/usr/bin/env python3
"""Read-only candidate search over a locally checked-out abilitykit_sdd repository.

    python3 skills/codecheck/scripts/kb_lookup.py --root /path/to/abilitykit_sdd \
        --query "Ability 终止回调 生命周期" --category both --top 10

Returns JSON matches with git revision, file, line and heading. The matches are
search leads, not validated normative requirements. Always inspect source text.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

VALID_SUFFIX = {".md", ".mdx", ".txt", ".rst", ".yaml", ".yml", ".json"}
CHINESE = re.compile(r"[\u3400-\u9fff]+")
LATIN = re.compile(r"[a-zA-Z_][\w.:-]*")
MAX_FILE_BYTES = 1_500_000
MAX_FILES = 12000


def tokens(phrase: str) -> set[str]:
    text = phrase.lower()
    result = {x.group() for x in LATIN.finditer(text) if len(x.group()) >= 2}
    for m in CHINESE.finditer(text):
        s = m.group()
        if len(s) <= 3:
            result.add(s)
        else:
            result.update(s[i:i+2] for i in range(len(s)-1))
            result.update(s[i:i+3] for i in range(len(s)-2))
    return result


def revision(root: Path) -> str:
    proc = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                          text=True, capture_output=True, check=False)
    return proc.stdout.strip() if proc.returncode == 0 else "unversioned"


def candidate_directories(root: Path, category: str) -> list[tuple[str, Path]]:
    names = ["standards", "specifications"] if category == "both" else ["standards" if category == "standard" else "specifications"]
    result = []
    for name in names:
        p = root / name
        if p.is_dir():
            result.append(("standard" if name == "standards" else "specification", p))
    return result


def line_score(line: str, path: str, query: str, terms: set[str]) -> int:
    text = line.lower()
    path_l = path.lower()
    score = 0
    if query.lower() in text:
        score += 20
    for t in terms:
        if t in text:
            score += 2 if len(t) <= 3 else 3
        if t in path_l:
            score += 1
    return score


def search(root: Path, query: str, category: str = "both", top: int = 10) -> dict:
    root = root.expanduser().resolve(strict=True)
    if not root.is_dir():
        raise ValueError(f"not a directory: {root}")
    dirs = candidate_directories(root, category)
    if not dirs:
        raise ValueError("standards/ or specifications/ not found in --root; check KB path")
    terms = tokens(query)
    if not terms:
        raise ValueError("query must contain searchable terms")
    items = []
    warnings = []
    scanned = 0
    for source_kind, directory in dirs:
        for path in directory.rglob("*"):
            if not path.is_file() or path.is_symlink() or path.suffix.lower() not in VALID_SUFFIX:
                continue
            scanned += 1
            if scanned > MAX_FILES:
                warnings.append(f"max_files={MAX_FILES} reached; retrieval is incomplete")
                break
            if path.stat().st_size > MAX_FILE_BYTES:
                warnings.append(f"skipped large file: {path.relative_to(root)}")
                continue
            try:
                lines = path.read_text(encoding="utf-8").splitlines()
            except (UnicodeError, OSError):
                warnings.append(f"unreadable file: {path.relative_to(root)}")
                continue
            rel = path.relative_to(root).as_posix()
            section = ""
            best = []
            for i, line in enumerate(lines):
                if re.match(r"^#{1,6}\s+", line):
                    section = line.strip("# ")
                score = line_score(line, rel, query.strip(), terms)
                if score <= 0:
                    continue
                # Prioritize text evidence; hits only in the filename should not flood results.
                if not any(t in line.lower() for t in terms):
                    continue
                best.append({
                    "path": rel,
                    "source_kind": source_kind,
                    "line": i + 1,
                    "section": section,
                    "score": score,
                    "snippet": " ".join(lines[max(0, i-1):min(len(lines), i+2)])[:450],
                })
            # A file may have several distinct relevant sections, but don't flood the top-k.
            items.extend(sorted(best, key=lambda v: (-v["score"], v["line"]))[:3])
        if scanned > MAX_FILES:
            break
    items.sort(key=lambda v: (-v["score"], v["path"], v["line"]))
    return {
        "repository": "abilitykit_public/abilitykit_sdd",
        "root": str(root),
        "revision": revision(root),
        "query": query,
        "category": category,
        "files_scanned": scanned,
        "matches": items[:max(1, top)],
        "warnings": warnings,
        "note": "Search leads only; open and verify original source, applicability, and version before citing.",
    }


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--root", type=Path, default=os.environ.get("CODECHECK_KB_ROOT"),
                   help="Local checkout of abilitykit_sdd (or CODECHECK_KB_ROOT)")
    p.add_argument("--query", required=True, help="Behavioral question, symbol or concrete spec topic")
    p.add_argument("--category", choices=["both", "standard", "specification"], default="both")
    p.add_argument("--top", type=int, default=10)
    args = p.parse_args(argv)
    if args.root is None:
        p.error("pass --root or set CODECHECK_KB_ROOT; no network fetching is performed")
    if args.top < 1 or args.top > 50:
        p.error("--top must be between 1 and 50")
    try:
        print(json.dumps(search(args.root, args.query, args.category, args.top),
                         indent=2, ensure_ascii=False))
    except (ValueError, FileNotFoundError, PermissionError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
