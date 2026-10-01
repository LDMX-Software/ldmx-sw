#!/usr/bin/env python3
"""Expand the ESA scalar expressions and GDML loops into numeric modules.
The source snapshot remains unchanged. Indexed geometry names use an _N suffix.
Logical active-volume names, physical copy numbers, regions and materials survive.
"""

import argparse
import copy
import hashlib
import json
import math
import re
from pathlib import Path

from lxml import etree as etree


class Matrix:
    def __init__(self, values, coldim):
        self.values, self.coldim = values, coldim

    def __getitem__(self, index):
        if isinstance(index, tuple):
            row, col = map(int, index)
            return self.values[(row - 1) * self.coldim + col - 1]
        return self.values[int(index) - 1]


def evaluate(expr, env):
    result = eval(expr.replace("^", "**"), {"__builtins__": {}}, env)
    if not isinstance(result, (float, int)) or not math.isfinite(result):
        raise ValueError(f"Non-finite/non-numeric: {expr}")
    return result


def number(value):
    return format(value, ".16g")


def definitions(root):
    env = {
        "mm": 1.0,
        "cm": 10.0,
        "m": 1000.0,
        "um": 0.001,
        "nm": 0.000001,
        "pi": math.pi,
        "twopi": 2 * math.pi,
        "deg": 1.0,
        "rad": 180 / math.pi,
        "sqrt": math.sqrt,
        "sin": math.sin,
        "cos": math.cos,
        "tan": math.tan,
        "abs": abs,
    }
    pending = [
        e
        for e in root.find("define")
        if isinstance(e.tag, str)
        and e.tag in ("constant", "variable", "quantity", "matrix", "expression")
    ]
    for _ in range(len(pending) + 1):
        later = []
        for e in pending:
            try:
                if e.tag == "matrix":
                    value = Matrix(
                        [
                            evaluate(t, env)
                            for t in re.split(
                                r"\s+(?![^\[]*\])", e.get("values").strip()
                            )
                        ],
                        int(e.get("coldim")),
                    )
                else:
                    value = evaluate(e.get("value") or e.text, env)
                env[e.get("name")] = value
            except (NameError, KeyError):
                later.append(e)
        if not later:
            return env
        if len(later) == len(pending):
            raise ValueError(
                "Unresolved definitions "
                + str([(e.get("name"), e.get("value")) for e in later])
            )
        pending = later
    raise ValueError("Could not resolve definitions")


NUMERIC = {
    "x",
    "y",
    "z",
    "x1",
    "x2",
    "y1",
    "y2",
    "z1",
    "z2",
    "rmin",
    "rmax",
    "rmin1",
    "rmax1",
    "rmin2",
    "rmax2",
    "startphi",
    "deltaphi",
    "starttheta",
    "deltatheta",
    "numsides",
    "alpha",
    "theta",
    "phi",
    "zOrder",
    "zPosition",
    "xOffset",
    "yOffset",
    "scalingFactor",
    "value",
    "n",
    "Z",
    "N",
    "copynumber",
    "dx",
    "dy",
    "dz",
    "ax",
    "ay",
    "bx",
    "by",
    "cx",
    "cy",
    "x3",
    "x4",
    "rlo",
    "rhi",
    "a",
    "b",
    "c",
}


def compile_tree(root, env, module):
    count = [0]

    def clone(element, local):
        if not isinstance(element.tag, str):
            return []
        if element.tag == "loop":
            var = element.get("for")
            start = int(evaluate(element.get("from", str(local.get(var, 1))), local))
            stop = int(evaluate(element.get("to"), local))
            step = int(evaluate(element.get("step", "1"), local))
            out = []
            for i in range(start, stop + (1 if step > 0 else -1), step):
                scope = dict(local)
                scope[var] = i
                for child in element:
                    out.extend(clone(child, scope))
            return out
        result = etree.Element(
            element.tag, nsmap=element.nsmap if element.tag == "gdml" else None
        )
        for key, value in element.attrib.items():
            if key in NUMERIC and element.tag != "auxiliary":
                value = number(evaluate(value, local))
            elif key in ("name", "ref") and "[" in value:
                value = re.sub(
                    r"\[([^\]]+)\]",
                    lambda m: "_" + number(evaluate(m.group(1), local)),
                    value,
                )
            result.set(key, value)
        if (
            element.tag in ("position", "rotation", "scale")
            and element.getparent() is not None
            and element.getparent().tag != "define"
        ):
            count[0] += 1
            result.set("name", f"{module}_{element.tag}_{count[0]}")
        for child in element:
            if element.tag == "define" and child.tag in (
                "constant",
                "variable",
                "quantity",
                "matrix",
                "expression",
            ):
                continue
            result.extend(clone(child, local))
        return [result]

    return clone(root, env)[0]


def prune(root):
    structure = root.find("structure")
    defs = {x.get("name"): x for x in structure if isinstance(x.tag, str)}
    keep = set()
    todo = [root.find("setup/world").get("ref")]
    while todo:
        key = todo.pop()
        if key in keep:
            continue
        keep.add(key)
        if key not in defs:
            raise ValueError("Unresolved volume " + key)
        todo.extend(x.get("ref") for x in defs[key].findall(".//volumeref"))
    removed = []
    for x in list(structure):
        if x.get("name") not in keep:
            removed.append(x.get("name"))
            structure.remove(x)
    solids = root.find("solids")
    sdefs = {x.get("name"): x for x in solids}
    needed = set()
    todo = [x.get("ref") for x in structure.findall(".//solidref")]
    while todo:
        key = todo.pop()
        if key in needed:
            continue
        needed.add(key)
        if key not in sdefs:
            raise ValueError("Unresolved solid " + key)
        todo.extend(
            x.get("ref")
            for x in sdefs[key].iter()
            if x.tag in ("first", "second", "solidref")
        )
    for x in list(solids):
        if x.get("name") not in needed:
            removed.append(x.get("name"))
            solids.remove(x)
    return removed


def compile_file(path, outdir, materials, pruning=True):
    parser = etree.XMLParser(
        load_dtd=True, resolve_entities=True, no_network=True, remove_comments=True
    )
    root = etree.parse(str(path), parser).getroot()
    env = definitions(root)
    compiled = compile_tree(root, env, path.stem)
    if compiled.find("materials") is None:
        compiled.insert(1, copy.deepcopy(materials))
    for e in compiled.iter():
        if e.tag == "position" and not e.get("unit"):
            e.set("unit", "mm")
        if e.tag == "rotation" and not e.get("unit"):
            e.set("unit", "radian")
    for e in compiled.find("solids"):
        if e.tag not in (
            "union",
            "subtraction",
            "intersection",
            "multiUnion",
            "scaledSolid",
            "tessellated",
        ) and not e.get("lunit"):
            e.set("lunit", "mm")
    removed = []
    if pruning and path.stem != "detector":
        removed = prune(compiled)
    # Definitions that name the reusable center and identity are scoped to the module.
    names = {
        e.get("name"): path.stem + "_" + e.get("name")
        for e in compiled.find("define")
        if e.get("name")
    }
    for e in compiled.find("define"):
        if e.get("name"):
            e.set("name", names[e.get("name")])
    for e in compiled.iter():
        if (
            e.tag in ("positionref", "rotationref", "scaleref")
            and e.get("ref") in names
        ):
            e.set("ref", names[e.get("ref")])
    # GDML XSD orders auxiliaries after daughters, and position before rotation.
    for volume in compiled.findall("structure/volume"):
        for child in list(volume):
            if child.tag == "auxiliary":
                volume.remove(child)
                volume.append(child)
    for pv in compiled.findall(".//physvol"):
        order = {
            "volumeref": 0,
            "file": 0,
            "position": 1,
            "positionref": 1,
            "rotation": 2,
            "rotationref": 2,
            "scale": 3,
            "scaleref": 3,
        }
        pv[:] = sorted(pv, key=lambda x: order.get(x.tag, 9))
    out = outdir / path.name
    etree.ElementTree(compiled).write(
        str(out), encoding="UTF-8", xml_declaration=True, pretty_print=True
    )
    return {
        "file": path.name,
        "sha256": hashlib.sha256(out.read_bytes()).hexdigest(),
        "source_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "remaining_loops": len(compiled.findall(".//loop")),
        "physical_placements": len(compiled.findall(".//physvol")),
        "logical_volumes": len(compiled.findall("structure/volume")),
        "removed_unreachable_geometry": removed,
    }


# This is a helper for build.py. It intentionally has no standalone entry point.
