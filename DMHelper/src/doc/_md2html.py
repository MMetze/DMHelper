"""Convert a markdown file to HTML using python-markdown."""
import sys
import markdown

src = sys.argv[1]
dst = sys.argv[2]
with open(src, "r", encoding="utf-8") as f:
    text = f.read()
html_body = markdown.markdown(
    text,
    extensions=["extra", "tables", "fenced_code", "toc", "sane_lists"],
)
html = f"""<!doctype html>
<html><head><meta charset="utf-8"><title>DMHelper Ruleset and Template System</title>
<style>
 body {{ font-family: Calibri, Arial, sans-serif; font-size: 11pt; }}
 h1 {{ font-size: 22pt; }}
 h2 {{ font-size: 16pt; margin-top: 18pt; }}
 h3 {{ font-size: 13pt; margin-top: 14pt; }}
 h4 {{ font-size: 11pt; margin-top: 10pt; }}
 code, pre {{ font-family: Consolas, "Courier New", monospace; font-size: 10pt; }}
 pre {{ background: #f4f4f4; border: 1px solid #ddd; padding: 8pt; }}
 table {{ border-collapse: collapse; }}
 th, td {{ border: 1px solid #888; padding: 4pt 8pt; vertical-align: top; }}
 th {{ background: #eee; }}
 blockquote {{ border-left: 3pt solid #888; padding-left: 8pt; color: #555; }}
</style></head><body>
{html_body}
</body></html>
"""
with open(dst, "w", encoding="utf-8") as f:
    f.write(html)
print(dst)
