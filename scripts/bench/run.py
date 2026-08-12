#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later

import argparse
import gzip
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import subprocess
import time


def output(command):
    try:
        return subprocess.check_output(command, text=True,
                                       stderr=subprocess.DEVNULL).strip()
    except (OSError, subprocess.CalledProcessError):
        return None


def dirty(path):
    status = output(["git", "-C", str(path), "status", "--porcelain"])
    return None if status is None else bool(status)


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", required=True)
    parser.add_argument("--output", type=Path, default=root / "bench/results")
    parser.add_argument("--number")
    parser.add_argument("--q0", type=int)
    parser.add_argument("--q1", type=int)
    parser.add_argument("--parameters", type=Path)
    parser.add_argument("--relations", type=Path)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command[:1] == ["--"]:
        command = command[1:]
    if not command or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]*", args.name):
        parser.error("provide --name and a command after --")

    revision = output(["git", "-C", str(root), "rev-parse", "HEAD"])
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
    run = args.output / (stamp + "-" + (revision or "uncommitted")[:12]
                         + "-" + args.name)
    record = {
        "git_revision": revision,
        "git_dirty": dirty(root),
        "cado_revision": output(["git", "-C", str(root / "third_party/cado-nfs"),
                                 "rev-parse", "HEAD"]),
        "cado_dirty": dirty(root / "third_party/cado-nfs"),
        "gpu_inventory": output(["nvidia-smi", "--query-gpu=name,uuid,driver_version",
                                 "--format=csv,noheader"]),
        "cuda_version": output([os.environ.get("CUDACXX", "nvcc"), "--version"]),
        "name": args.name,
        "start_utc": stamp,
        "cwd": str(Path.cwd()),
        "command": command,
        "number": args.number,
        "q0": args.q0,
        "q1": args.q1,
        "parameters": None,
        "relations_file": str(args.relations.resolve()) if args.relations else None,
        "relations": None,
        "relations_per_second": None,
        "gpu_seconds": None,
        "peak_gpu_memory_bytes": None,
    }
    if args.parameters:
        contents = args.parameters.read_bytes()
        record["parameters"] = {
            "path": str(args.parameters.resolve()),
            "sha256": hashlib.sha256(contents).hexdigest(),
            "contents": contents.decode(),
        }
    if args.relations and args.relations.exists():
        parser.error("--relations must name a new output file")
    run.mkdir(parents=True)
    with (run / "stdout").open("wb") as stdout, (run / "stderr").open("wb") as stderr:
        before = resource.getrusage(resource.RUSAGE_CHILDREN)
        start = time.monotonic()
        try:
            result = subprocess.run(command, stdout=stdout, stderr=stderr)
            record["exit_code"] = result.returncode
        except OSError as error:
            record["exit_code"] = 127
            record["error"] = str(error)
        record["wall_seconds"] = time.monotonic() - start
        after = resource.getrusage(resource.RUSAGE_CHILDREN)
    record["cpu_user_seconds"] = after.ru_utime - before.ru_utime
    record["cpu_system_seconds"] = after.ru_stime - before.ru_stime
    # One fully occupied CPU core is 100%; threaded runs may exceed 100%.
    record["cpu_utilization_percent"] = 100 * (
        record["cpu_user_seconds"] + record["cpu_system_seconds"]
    ) / record["wall_seconds"]
    if record["exit_code"] == 0 and args.relations:
        try:
            open_relations = gzip.open if args.relations.suffix == ".gz" else open
            with open_relations(args.relations, "rt") as relations:
                record["relations"] = sum(
                    1 for line in relations
                    if line.strip() and not line.lstrip().startswith("#"))
            record["relations_per_second"] = record["relations"] / record["wall_seconds"]
        except (OSError, UnicodeError) as error:
            record["error"] = "cannot count relations: " + str(error)
    (run / "result.json").write_text(json.dumps(record, indent=2) + "\n")
    print(run / "result.json")
    return record["exit_code"] if record["exit_code"] >= 0 else 128 - record["exit_code"]


if __name__ == "__main__":
    raise SystemExit(main())
