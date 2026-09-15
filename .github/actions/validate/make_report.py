"""Build a static HTML validation report from compare.py outputs

Each input directory is named after its sample and holds the unpacked
plots archive (results.json, fail/*.png, pass/*.png) and optionally the
status.txt and check_messages.txt of that sample.

The output directory gets an index.html summarizing all samples and
one page per sample with the histograms ranked worst KS distance first.
Only python standard library is used so this can run on a bare runner.
"""

import argparse
import datetime
import html
import json
import os
import shutil


# ranking of statuses, worst first
STATUS_ORDER = {"missing": 0, "fail": 1, "new": 2, "pass": 3}

HERE = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(HERE, "report.css")) as f:
    STYLE = f.read()
with open(os.path.join(HERE, "report.js")) as f:
    SAMPLE_JS = f.read()


def esc(s):
    return html.escape(str(s))


def page(title, meta, body):
    return (
        f"<!doctype html><html><head><meta charset='utf-8'>"
        f"<meta name='viewport' content='width=device-width,initial-scale=1'>"
        f"<title>{esc(title)}</title><style>{STYLE}</style></head><body>"
        f"<header><h1>{esc(title)}</h1><div class='meta'>{meta}</div></header>"
        f"<main>{body}</main></body></html>"
    )


def entries(r):
    return r.get("gold_entries", 0) + r.get("test_entries", 0)


def sort_key(r):
    return (
        STATUS_ORDER.get(r["status"], 9),
        -r.get("ks_dist", 0.0),
        -entries(r),
        r["key"],
    )


def read_lines(path):
    if not os.path.isfile(path):
        return []
    with open(path) as f:
        return [line.rstrip("\n") for line in f if line.strip()]


def load_sample(sample_dir, out_dir, with_pass):
    """Load results of one sample and copy its plots into the site"""
    name = os.path.basename(os.path.normpath(sample_dir))
    with open(os.path.join(sample_dir, "results.json")) as f:
        rows = json.load(f)
    dest = os.path.join(out_dir, name)
    for r in rows:
        if r["status"] not in ("fail", "pass") or "name" not in r:
            continue
        if r["status"] == "pass" and not with_pass:
            continue
        src = os.path.join(sample_dir, r["status"], r["name"] + ".png")
        if os.path.isfile(src):
            rel = f"{r['status']}/{r['name']}.png"
            os.makedirs(os.path.join(dest, r["status"]), exist_ok=True)
            shutil.copyfile(src, os.path.join(dest, rel))
            r["img"] = rel
    rows.sort(key=sort_key)
    status = read_lines(os.path.join(sample_dir, "status.txt"))
    return {
        "name": name,
        "rows": rows,
        "status": status[0] if status else None,
        "messages": read_lines(os.path.join(sample_dir, "check_messages.txt")),
    }


def counts(rows):
    c = dict.fromkeys(STATUS_ORDER, 0)
    for r in rows:
        c[r["status"]] = c.get(r["status"], 0) + 1
    return c


def write_sample_page(s, out_dir, title, meta):
    data = json.dumps({"rows": s["rows"]}).replace("</", "<\\/")
    c = counts(s["rows"])
    body = f"""
<p><a href="../index.html">&larr; all samples</a> &middot;
<span class="fail">{c["fail"]} fail</span> &middot;
<span class="missing">{c["missing"]} missing</span> &middot;
<span class="new">{c["new"]} new</span> &middot;
<span class="pass">{c["pass"]} pass</span></p>
<ul class="msgs">{"".join(f"<li>{esc(m)}</li>" for m in s["messages"])}</ul>
<div class="layout"><aside>
<label>Status</label><select id="status">
<option value="fail">fail</option><option value="missing">missing in new</option>
<option value="new">new (not in gold)</option><option value="pass">pass</option>
<option value="all">all</option></select>
<label>Sort</label><select id="sort">
<option value="dist">KS distance D (worst first)</option>
<option value="sig">significance D&middot;sqrt(n_eff)</option>
<option value="prob">KS probability p (worst first)</option>
<option value="name">name</option></select>
<label>Search</label><input id="q" type="search" placeholder="histogram name">
<label>Min D: <span id="dminv">0.00</span></label>
<input id="dmin" type="range" min="0" max="1" step="0.01" value="0">
<label>Folders (non-pass / total)</label><div class="folders" id="folders"></div>
</aside><section><div id="count"></div><div class="grid" id="grid"></div>
<button id="more">show more</button></section></div>
<script>const DATA = {data};{SAMPLE_JS}</script>
"""
    os.makedirs(os.path.join(out_dir, s["name"]), exist_ok=True)
    with open(os.path.join(out_dir, s["name"], "index.html"), "w") as f:
        f.write(page(f"{title}: {s['name']}", meta, body))


def write_index(samples, out_dir, title, meta):
    trs = []
    for s in samples:
        c = counts(s["rows"])
        failing = s["status"] == "FAIL" or c["fail"] or c["missing"]
        worst = [r for r in s["rows"] if r["status"] == "fail"][:3]
        worst_html = "<br>".join(
            f"<code>{esc(r['key'])}</code> D={r['ks_dist']:.3f}" for r in worst
        )
        cls, label = ("fail", "FAIL") if failing else ("pass", "PASS")
        trs.append(
            f"<tr><td><a href='{esc(s['name'])}/index.html'>{esc(s['name'])}</a></td>"
            f"<td class='{cls}'>{label}</td>"
            f"<td class='num'>{c['fail']}</td><td class='num'>{c['missing']}</td>"
            f"<td class='num'>{c['new']}</td><td class='num'>{len(s['rows'])}</td>"
            f"<td>{worst_html}<ul class='msgs'>"
            f"{''.join(f'<li>{esc(m)}</li>' for m in s['messages'])}</ul></td></tr>"
        )
    body = (
        "<p><a href='../index.html'>&larr; all runs of this PR</a></p>"
        "<div class='wrap'><table><tr><th>Sample</th><th>Status</th>"
        "<th class='num'>Fail</th><th class='num'>Missing</th>"
        "<th class='num'>New</th><th class='num'>Total</th>"
        "<th>Worst histograms &amp; checks</th></tr>" + "".join(trs) + "</table></div>"
    )
    with open(os.path.join(out_dir, "index.html"), "w") as f:
        f.write(page(title, meta, body))


def write_comment_md(samples, path, base_url, n_worst):
    """Markdown listing the worst histograms of each failing sample"""
    lines = []
    for s in samples:
        worst = [r for r in s["rows"] if r["status"] == "fail"][:n_worst]
        missing = [r["key"] for r in s["rows"] if r["status"] == "missing"]
        if not worst and not missing:
            continue
        link = (
            f"[`{s['name']}`]({base_url}/{s['name']}/)"
            if base_url
            else f"`{s['name']}`"
        )
        lines.append(f"\n**{link}**")
        if worst:
            lines.append(
                "\n| Histogram | KS D | KS p | Entries gold/new |\n|---|---|---|---|"
            )
            for r in worst:
                lines.append(
                    f"| `{r['key']}` | {r['ks_dist']:.3f} | {r['ks_prob']:.2g}"
                    f" | {r['gold_entries']:.0f}/{r['test_entries']:.0f} |"
                )
        if missing:
            shown = ", ".join(f"`{k}`" for k in missing[:3])
            more = f" and {len(missing) - 3} more" if len(missing) > 3 else ""
            lines.append(f"\n{len(missing)} histograms missing in new: {shown}{more}")
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")


def run_sort_key(name):
    # run-<id>-<attempt>
    parts = name.split("-")
    return tuple(int(x) if x.isdigit() else 0 for x in parts[1:])


def write_runs_index(pr_dir, keep, title):
    """Drop old runs of a PR and list the remaining ones newest first"""
    runs = sorted(
        (d for d in os.listdir(pr_dir) if d.startswith("run-")),
        key=run_sort_key,
        reverse=True,
    )
    for old in runs[keep:]:
        shutil.rmtree(os.path.join(pr_dir, old))
    trs = []
    for i, r in enumerate(runs[:keep]):
        summary = {}
        path = os.path.join(pr_dir, r, "summary.json")
        if os.path.isfile(path):
            with open(path) as f:
                summary = json.load(f)
        bad = [s for s in summary.get("samples", []) if s["fail"] or s["missing"]]
        bad_html = ", ".join(
            f"{esc(s['name'])} ({s['fail'] + s['missing']})" for s in bad
        )
        latest = " (latest)" if i == 0 else ""
        cls = "fail" if bad else "pass"
        trs.append(
            f"<tr><td><a href='{esc(r)}/index.html'>{esc(r)}</a>{latest}</td>"
            f"<td>{esc(summary.get('generated', ''))}</td>"
            f"<td>{summary.get('meta', '')}</td>"
            f"<td class='{cls}'>{bad_html or 'all pass'}</td></tr>"
        )
    body = (
        "<div class='wrap'><table><tr><th>Run</th><th>Generated (UTC)</th>"
        "<th>Details</th><th>Failing samples (histograms)</th></tr>"
        + "".join(trs)
        + "</table></div>"
        f"<p class='msgs'>Only the newest {keep} runs are kept.</p>"
    )
    with open(os.path.join(pr_dir, "index.html"), "w") as f:
        f.write(page(title, "", body))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("samples", nargs="*", help="per-sample directories")
    p.add_argument("--out", help="output site directory")
    p.add_argument("--title", default="Validation Report")
    p.add_argument("--meta", default="", help="html subtitle, e.g. run link")
    p.add_argument(
        "--with-pass", action="store_true", help="also publish passing plots"
    )
    p.add_argument("--comment-md", help="write worst-histogram markdown here")
    p.add_argument("--base-url", default="", help="public url of --out")
    p.add_argument("--n-worst", type=int, default=5)
    p.add_argument("--runs-index", help="PR directory to index instead of building")
    p.add_argument("--keep", type=int, default=5, help="runs kept by --runs-index")
    args = p.parse_args()

    if args.runs_index:
        write_runs_index(args.runs_index, args.keep, args.title)
        return
    if not args.out or not args.samples:
        p.error("--out and sample directories are required")

    os.makedirs(args.out, exist_ok=True)
    samples = []
    for d in args.samples:
        if not os.path.isfile(os.path.join(d, "results.json")):
            print(f"::warning::no results.json in {d}, skipping")
            continue
        samples.append(load_sample(d, args.out, args.with_pass))
    # failing samples first, most failures first
    samples.sort(
        key=lambda s: (
            -sum(r["status"] in ("fail", "missing") for r in s["rows"]),
            s["name"],
        )
    )
    for s in samples:
        write_sample_page(s, args.out, args.title, args.meta)
    write_index(samples, args.out, args.title, args.meta)
    # read by --runs-index
    with open(os.path.join(args.out, "summary.json"), "w") as f:
        json.dump(
            {
                "title": args.title,
                "meta": args.meta,
                "generated": datetime.datetime.now(datetime.timezone.utc).strftime(
                    "%Y-%m-%d %H:%M"
                ),
                "samples": [{"name": s["name"], **counts(s["rows"])} for s in samples],
            },
            f,
        )
    if args.comment_md:
        write_comment_md(
            samples, args.comment_md, args.base_url.rstrip("/"), args.n_worst
        )


if __name__ == "__main__":
    main()
