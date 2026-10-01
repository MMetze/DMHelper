#!/usr/bin/env python3
"""Build an AI-readable map of every Qt Designer .ui file in DMHelper/src.

Per .ui file it records:
  - widget tree (class, objectName, layout type, visible text/tooltip, inline styleSheet)
  - Designer <connections> (signal/slot)
  - C++ files that include ui_<name>.h, the ui-> members they touch and the
    connect() calls made on those members (plus where forwarded signals land)
  - for runtime-loaded templates (resources/ui), the files that reference them

Outputs (default dev/ui-map/):
  ui-manifest.json  full data for tooling
  ui-manifest.md    compact indented text for LLM context

Usage: python dev/tools/ui_manifest.py [--src <src dir>] [--out <out dir>]
"""

import argparse
import json
import re
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict
from pathlib import Path

SKIP_DIRS = {"out", "build", ".git", ".vs", ".qtcreator", "docs", "doc", "installer", "release notes"}
SKIP_PREFIXES = ("bin-", "vlc")
TEXT_LIMIT = 60

CONNECT_START = re.compile(r"\bconnect\s*\(")
UI_MEMBER = re.compile(r"\bui->(\w+)")
AUTO_SLOT = re.compile(r"\bvoid\s+\w+::on_(\w+?)_(\w+)\s*\(")
SIGNAL_MACRO = re.compile(r"SIGNAL\s*\(\s*(\w+)\s*\(")
PMF = re.compile(r"&\s*(\w+)::(\w+)")


def skip_dir(part: str) -> bool:
    return part in SKIP_DIRS or part.startswith(SKIP_PREFIXES)


def find_files(src: Path, suffixes):
    for p in src.rglob("*"):
        if p.suffix.lower() not in suffixes or not p.is_file():
            continue
        rel = p.relative_to(src)
        if any(skip_dir(part) for part in rel.parts[:-1]):
            continue
        yield p


def short(text, limit=TEXT_LIMIT):
    if text is None:
        return None
    text = " ".join(text.split())
    return text if len(text) <= limit else text[: limit - 3] + "..."


def prop_value(prop):
    for child in prop:
        if child.tag in ("string", "enum", "set", "number", "bool", "cstring"):
            return child.text or ""
        if child.tag == "iconset":
            normal = child.find("normaloff")
            return (normal.text if normal is not None else child.text) or ""
    return None


def parse_widget(elem):
    node = {"kind": elem.tag, "class": elem.get("class"), "name": elem.get("name")}
    props = {p.get("name"): prop_value(p) for p in elem.findall("property")}
    attrs = {a.get("name"): prop_value(a) for a in elem.findall("attribute")}
    for key in ("text", "title", "toolTip", "windowTitle", "placeholderText"):
        if props.get(key):
            node[key] = short(props[key])
    if attrs.get("title"):
        node["tabTitle"] = short(attrs["title"])
    if props.get("icon"):
        node["icon"] = props["icon"]
    if props.get("styleSheet"):
        node["styleSheet"] = " ".join(props["styleSheet"].split())
    if props.get("checkable") == "true":
        node["checkable"] = True
    for key in ("minimumSize", "maximumSize"):
        p = elem.find(f"property[@name='{key}']/size")
        if p is not None:
            node[key] = f"{p.findtext('width')}x{p.findtext('height')}"
    children = []
    for child in elem:
        if child.tag in ("widget", "layout"):
            children.append(parse_widget(child))
        elif child.tag == "item":
            for sub in child:
                if sub.tag in ("widget", "layout"):
                    children.append(parse_widget(sub))
                elif sub.tag == "spacer":
                    children.append({"kind": "spacer", "name": sub.get("name")})
        elif child.tag == "addaction":
            children.append({"kind": "addaction", "name": child.get("name")})
        elif child.tag == "action":
            children.append(parse_widget(child))
    if children:
        node["children"] = children
    return node


def walk_names(node, out):
    if node.get("name") and node["kind"] in ("widget", "action", "layout"):
        out[node["name"]] = node.get("class") or node["kind"]
    for c in node.get("children", []):
        walk_names(c, out)


def split_args(text, start):
    """Given index just after 'connect(', return (list of top-level args, end index)."""
    depth, args, cur, i = 1, [], [], start
    while i < len(text) and depth > 0:
        ch = text[i]
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
            if depth == 0:
                break
        if ch == "," and depth == 1:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(ch)
        i += 1
    args.append("".join(cur).strip())
    return [" ".join(a.split()) for a in args], i


def parse_connects(text):
    for m in CONNECT_START.finditer(text):
        args, _ = split_args(text, m.end())
        if len(args) >= 3:
            yield text.count("\n", 0, m.start()) + 1, args


def parse_ui(path: Path, src: Path):
    root = ET.parse(path).getroot()
    top = root.find("widget")
    tree = parse_widget(top) if top is not None else {}
    names = {}
    walk_names(tree, names)
    connections = []
    for c in root.findall("connections/connection"):
        connections.append({
            "sender": c.findtext("sender"), "signal": c.findtext("signal"),
            "receiver": c.findtext("receiver"), "slot": c.findtext("slot"),
        })
    custom = [{"class": cw.findtext("class"), "extends": cw.findtext("extends"), "header": cw.findtext("header")}
              for cw in root.findall("customwidgets/customwidget")]
    return {
        "file": path.relative_to(src).as_posix(),
        "uiClass": root.findtext("class"),
        "topClass": top.get("class") if top is not None else None,
        "tree": tree,
        "names": names,
        "connections": connections,
        "customWidgets": custom,
    }


def build(src: Path):
    ui_files = sorted(find_files(src, {".ui"}))
    code_files = sorted(find_files(src, {".cpp", ".h"}))
    data_files = sorted(find_files(src, {".cpp", ".h", ".xml", ".qrc"}))
    code_text = {p: p.read_text(encoding="utf-8", errors="replace") for p in code_files}
    data_text = {p: (code_text.get(p) or p.read_text(encoding="utf-8", errors="replace")) for p in data_files}

    # Index where every named signal is connected, to follow ribbon/frame signal forwarding.
    signal_uses = defaultdict(list)
    for p, text in code_text.items():
        for line, args in parse_connects(text):
            sig = args[1]
            m = PMF.search(sig) or SIGNAL_MACRO.search(sig)
            if m:
                key = m.group(m.lastindex)
                signal_uses[key].append(f"{p.relative_to(src).as_posix()}:{line} -> {args[-1] if len(args) == 3 else args[3]}")

    results = []
    for ui_path in ui_files:
        info = parse_ui(ui_path, src)
        stem = ui_path.stem
        include_re = re.compile(rf'#\s*include\s*[<"]ui_{re.escape(stem)}\.h[>"]', re.IGNORECASE)
        owners = [p for p, t in code_text.items() if include_re.search(t)]
        cpp = []
        for p in owners:
            text = code_text[p]
            members = Counter(UI_MEMBER.findall(text))
            conns = []
            for line, args in parse_connects(text):
                m = UI_MEMBER.search(args[0])
                if not m:
                    continue
                entry = {"line": line, "sender": m.group(1), "signal": args[1],
                         "receiver": args[2], "target": args[3] if len(args) > 3 else None}
                target = entry["target"] or ""
                fwd = SIGNAL_MACRO.search(target) or (PMF.search(target) if args[2] == "this" else None)
                if fwd:
                    uses = [u for u in signal_uses.get(fwd.group(fwd.lastindex), [])
                            if not u.startswith(p.relative_to(src).as_posix() + ":")]
                    if uses:
                        entry["forwardedTo"] = uses[:4]
                conns.append(entry)
            auto = [f"on_{w}_{s}" for w, s in AUTO_SLOT.findall(text)]
            cpp.append({
                "file": p.relative_to(src).as_posix(),
                "members": dict(sorted(members.items())),
                "connects": conns,
                "autoSlots": auto,
            })
        info["cpp"] = cpp
        referenced = set()
        for c in cpp:
            referenced.update(c["members"])
        info["unreferencedWidgets"] = sorted(
            n for n, cls in info["names"].items()
            if n not in referenced and n != info["tree"].get("name")
            and not cls.endswith("Layout") and cls != "QWidget"
            and not n.startswith(("layoutWidget", "label")))
        if not owners:
            needle = ui_path.name
            info["loadedBy"] = [p.relative_to(src).as_posix() for p, t in data_text.items() if needle in t]
        results.append(info)
    return results


def stylesheet_census(results):
    counts = Counter()
    for r in results:
        stack = [r["tree"]]
        while stack:
            n = stack.pop()
            if n.get("styleSheet"):
                counts[n["styleSheet"]] += 1
            stack.extend(n.get("children", []))
    return counts


def render_node(node, depth, lines):
    kind = node["kind"]
    if kind == "spacer":
        lines.append("  " * depth + "~spacer")
        return
    if kind == "addaction":
        lines.append("  " * depth + f"+action {node['name']}")
        return
    label = f"{node.get('class') or kind} {node.get('name') or ''}".rstrip()
    extras = []
    for key in ("text", "title", "tabTitle", "windowTitle", "toolTip", "placeholderText"):
        if node.get(key):
            extras.append(f'{key}="{node[key]}"')
    if node.get("icon"):
        extras.append(f"icon={Path(node['icon']).name}")
    if node.get("checkable"):
        extras.append("checkable")
    if node.get("minimumSize") and node.get("minimumSize") == node.get("maximumSize"):
        extras.append(f"fixed={node['minimumSize']}")
    if node.get("styleSheet"):
        extras.append(f"qss={short(node['styleSheet'], 40)!r}")
    lines.append("  " * depth + label + (" [" + ", ".join(extras) + "]" if extras else ""))
    for c in node.get("children", []):
        render_node(c, depth + 1, lines)


def render_md(results, census):
    lines = [
        "# DMHelper UI manifest (generated by dev/tools/ui_manifest.py - do not edit)",
        "",
        f"{len(results)} .ui files. Tree: `Class name [props]`, indentation = containment.",
        "Sections per file: TREE, DESIGNER CONNECTIONS, CPP (ui-> members, connect() calls, forwarded signal sinks).",
        "",
        "## Inline styleSheet census (distinct value -> count)",
    ]
    for qss, n in census.most_common(25):
        lines.append(f"- {n:4d}  {short(qss, 110)}")
    lines.append(f"- ({len(census)} distinct values total)")
    for r in results:
        lines += ["", f"## {r['file']}  ({r['uiClass']} : {r['topClass']})"]
        if r["customWidgets"]:
            lines.append("custom: " + ", ".join(f"{c['class']}<{c['extends']}>" for c in r["customWidgets"]))
        lines.append("TREE")
        render_node(r["tree"], 1, lines)
        if r["connections"]:
            lines.append("DESIGNER CONNECTIONS")
            for c in r["connections"]:
                lines.append(f"  {c['sender']}.{c['signal']} -> {c['receiver']}.{c['slot']}")
        for c in r["cpp"]:
            lines.append(f"CPP {c['file']}  members={len(c['members'])}")
            for conn in c["connects"]:
                tgt = f"{conn['receiver']}, {conn['target']}" if conn["target"] else conn["receiver"]
                lines.append(f"  L{conn['line']} ui->{conn['sender']} {conn['signal']} -> {tgt}")
                for f in conn.get("forwardedTo", []):
                    lines.append(f"      => {f}")
            if c["autoSlots"]:
                lines.append("  autoSlots: " + ", ".join(c["autoSlots"]))
        if r.get("loadedBy") is not None:
            lines.append("LOADED BY: " + (", ".join(r["loadedBy"]) or "(no references found)"))
        if r["unreferencedWidgets"] and r["cpp"]:
            lines.append("NOT TOUCHED FROM C++: " + ", ".join(r["unreferencedWidgets"]))
    return "\n".join(lines) + "\n"


def main():
    here = Path(__file__).resolve()
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", type=Path, default=here.parents[2])
    ap.add_argument("--out", type=Path, default=here.parents[1] / "ui-map")
    a = ap.parse_args()
    results = build(a.src)
    census = stylesheet_census(results)
    a.out.mkdir(parents=True, exist_ok=True)
    for r in results:
        r.pop("names", None)
    (a.out / "ui-manifest.json").write_text(
        json.dumps({"stylesheetCensus": census.most_common(), "files": results}, indent=1), encoding="utf-8")
    (a.out / "ui-manifest.md").write_text(render_md(results, census), encoding="utf-8")
    print(f"{len(results)} ui files -> {a.out}")


if __name__ == "__main__":
    main()
