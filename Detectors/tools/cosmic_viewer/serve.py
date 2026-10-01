#!/usr/bin/env python3
"""Open the locally built viewer; no third-party Python packages are needed."""

import argparse
import contextlib
import webbrowser
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--port", type=int, default=0, help="0 chooses a free local port.")
parser.add_argument("--no-browser", action="store_true")
args = parser.parse_args()
directory = Path(__file__).resolve().parent / "build"
if not (directory / "cosmic_ray_interactive.html").exists():
    parser.error("Build the viewer first: python build.py --render")
with ThreadingHTTPServer(
    ("127.0.0.1", args.port),
    partial(SimpleHTTPRequestHandler, directory=str(directory)),
) as server:
    url = f"http://127.0.0.1:{server.server_port}/cosmic_ray_interactive.html"
    print(url + "\nCtrl+C stops the viewer.", flush=True)
    if not args.no_browser:
        webbrowser.open(url)
    with contextlib.suppress(KeyboardInterrupt):
        server.serve_forever()
