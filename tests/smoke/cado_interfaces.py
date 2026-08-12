#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later

import argparse
import hashlib
import json
from pathlib import Path
import shlex
from string import Template
import subprocess
import sys
import tempfile


def check_interfaces(source, binary):
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(source / "scripts"))
    from cadofactor import cadoprograms
    from cadofactor.workunit import Workunit

    poly = source / "tests/sieve/c30_30012.poly"
    fb = source / "tests/sieve/c30_30012.roots1.gz"
    las = cadoprograms.Las(
        poly=str(poly), factorbase1=str(fb), q0=50000, q1=50100, I=9,
        lim0=30000, lim1=30000, lpb0=17, lpb1=17, mfb0=18, mfb1=18,
        sqside=1, threads="1", out="relations.gz", stats_stderr=True,
        execpath=str(binary) if binary else None,
        skip_check_binary_exists=binary is None)
    command = las.make_command_array()
    for key, value in (("q0", "50000"), ("q1", "50100"),
                       ("fb1", str(fb)), ("out", "relations.gz"),
                       ("sqside", "1"), ("t", "1")):
        assert command[command.index("-" + key) + 1] == value
    assert "-stats-stderr" in command

    gps1_reference = cadoprograms.Polyselect(
        N=999073468111577057576445816581, degree=3, P=1000,
        admin=60, admax=120, incr=60, nq=100, sopteffort=0,
        skip_check_binary_exists=True)
    command = gps1_reference.make_command_array()
    assert command[command.index("-admin") + 1] == "60"
    assert command[command.index("-sopteffort") + 1] == "0"

    for invalid in ({}, {"id": "bad", "q0": 50000},
                    {"id": "bad", "files": {"FILE0": {
                        "filename": "x", "checksum": "0" * 40}}},
                    {"id": "bad", "files": {"RESULT0": {
                        "filename": "x", "upload": True,
                        "checksum": "0" * 40, "algorithm": "sha1"}}}):
        try:
            Workunit(json.dumps(invalid))
        except Exception:
            continue
        raise AssertionError("CADO accepted malformed workunit")

    if binary is None:
        return

    # Use CADO's command builder and checksum rules, including EXECFILE.
    wu = las.make_wu("sieve-50000-50100")
    assert Workunit(str(wu)) == wu
    translations = {}
    for fid, entry in wu["files"].items():
        if fid.startswith("EXECFILE"):
            path = Path(las.get_exec_file())
        elif fid.startswith("FILE"):
            path = poly if entry["filename"] == poly.name else fb
        else:
            path = Path(entry["filename"])
        translations[fid] = str(path)
        if entry.get("download"):
            assert entry["checksum"] == hashlib.new(
                entry["algorithm"], path.read_bytes()).hexdigest()
    expanded = Template(wu["commands"][0]).substitute(translations)
    assert shlex.split(expanded) == las.make_command_array()
    assert wu["files"]["RESULT0"]["upload"] is True

    with tempfile.TemporaryDirectory() as tmp:
        roots = Path(tmp) / "small.roots"
        makefb = cadoprograms.MakeFB(
            poly=str(poly), lim=1000, maxbits=7, out=str(roots),
            threads=1, execpath=str(binary))
        subprocess.run(makefb.make_command_array(), check=True,
                       stdout=subprocess.DEVNULL)
        assert roots.stat().st_size > 0
        args = las.make_command_array()
        args[args.index("-out") + 1] = str(Path(tmp) / "todo")
        args.append("-print-todo-list")
        subprocess.run(args, check=True, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        first = (Path(tmp) / "todo").read_text()
        subprocess.run(args, check=True, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        second = (Path(tmp) / "todo").read_text()
        rows = [line.split() for line in first.splitlines()
                if line.strip() and not line.startswith("#")]
        assert rows == [line.split() for line in second.splitlines()
                        if line.strip() and not line.startswith("#")]
        assert rows
        for side, q, rho in rows:
            assert int(side) == 1 and 50000 <= int(q) < 50100
            assert (3000 * int(rho)**3 - 1157101 * int(rho)**2
                    + 385681286 * int(rho) + 98715698075) % int(q) == 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cado-source", type=Path, required=True)
    parser.add_argument("--cado-binary", type=Path)
    options = parser.parse_args()
    check_interfaces(options.cado_source.resolve(),
                     options.cado_binary.resolve() if options.cado_binary else None)
    print("CADO interface checks passed")
