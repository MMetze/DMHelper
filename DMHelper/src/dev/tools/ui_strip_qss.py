#!/usr/bin/env python3
"""Remove inline <property name="styleSheet"> blocks from Qt Designer .ui files.

Textual edit so the rest of the Designer XML (formatting, ordering, line endings) is untouched.
Prints every removed value so it can be recreated in a central theme stylesheet.

Usage: python dev/tools/ui_strip_qss.py [--dry-run] file.ui [file.ui ...]
"""

import argparse
import re
from pathlib import Path

BLOCK = re.compile(
    r'^[ \t]*<property name="styleSheet">\s*<string[^>]*?(?:/>|>(?P<value>.*?)</string>)\s*</property>[ \t]*\r?\n',
    re.MULTILINE | re.DOTALL,
)
OWNER = re.compile(r'<widget class="(?P<cls>[^"]+)" name="(?P<name>[^"]+)"')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("files", nargs="+", type=Path)
    a = ap.parse_args()
    for path in a.files:
        with open(path, encoding="utf-8", newline="") as f:
            text = f.read()
        removed = []
        for m in BLOCK.finditer(text):
            owners = list(OWNER.finditer(text, 0, m.start()))
            owner = f"{owners[-1]['cls']} {owners[-1]['name']}" if owners else "?"
            removed.append((owner, " ".join((m.group("value") or "").split())))
        if not removed:
            print(f"{path}: nothing to strip")
            continue
        for owner, value in removed:
            print(f"{path}: {owner}: {value or '(empty)'}")
        if not a.dry_run:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(BLOCK.sub("", text))


if __name__ == "__main__":
    main()
