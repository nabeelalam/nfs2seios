"""Summarize a privately supplied recorder log without printing paths or identifiers."""
import argparse
import json
import math
import statistics
from pathlib import Path

MAX_BYTES = 32 * 1024 * 1024
MAX_LINE = 8192


def number(value, maximum=1e15):
    return type(value) in (int, float) and math.isfinite(value) and 0 <= value <= maximum


def distribution(values):
    if not values:
        return {"count": 0, "minimum": None, "median": None, "maximum": None}
    return {"count": len(values), "minimum": min(values),
            "median": statistics.median(values), "maximum": max(values)}


def summarize(data):
    if len(data) > MAX_BYTES:
        raise ValueError("Log exceeds the recorder size bound")
    lines = data.splitlines(keepends=True)
    if not lines or len(lines) > 3602 or any(len(line) > MAX_LINE for line in lines):
        raise ValueError("Invalid log bounds")
    rows = []
    partial_tail = False
    for index, line in enumerate(lines):
        try:
            def invalid_constant(_):
                raise ValueError("Nonfinite JSON constant")
            row = json.loads(line, parse_constant=invalid_constant)
        except (ValueError, UnicodeError):
            if index == len(lines) - 1 and not line.endswith(b"\n"):
                partial_tail = True
                break
            raise ValueError("Malformed complete log row") from None
        if not isinstance(row, dict):
            raise ValueError("Expected a JSON object")
        rows.append(row)
    if not rows or rows[0].get("type") != "session" or rows[0].get("schema") != "nfsiise-session-v1":
        raise ValueError("Unsupported session schema")
    build = rows[0].get("build")
    if type(build) is not int or not 0 < build < 1000000:
        raise ValueError("Invalid build")
    groups = {False: [], True: []}
    windows = {False: [], True: []}
    seen_ids = set()
    previous = -1
    end = None
    gaps = 0
    count = 0
    timing_rows = []
    previous_timing = None
    for index, row in enumerate(rows[1:], start=1):
        if row.get("type") == "end":
            if index != len(rows) - 1 or partial_tail:
                raise ValueError("End record is not final")
            if row.get("reason") not in {"manual", "background", "window_changed", "time_limit", "write_error", "writer_backlog", "stopped"}:
                raise ValueError("Invalid end reason")
            if not number(row.get("elapsed_s")) or row["elapsed_s"] < previous:
                raise ValueError("Invalid end time")
            attempted = row.get("attempted_rows")
            if type(attempted) is not int or not count <= attempted <= 3600:
                raise ValueError("Invalid end row count")
            end = row
            continue
        if row.get("type") != "sample":
            raise ValueError("Unexpected row type")
        elapsed = row.get("elapsed_s")
        power = row.get("low_power_mode")
        identity = row.get("game_cpu_sample_id")
        if not number(elapsed, 3600) or elapsed >= 3600 or elapsed <= previous or type(power) is not bool:
            raise ValueError("Invalid sample time or power mode")
        if type(identity) is not int or not 0 <= identity < 2**64:
            raise ValueError("Invalid CPU window identity")
        timing = row.get("hd_timing_requested")
        if timing is not None and type(timing) is not bool:
            raise ValueError("Invalid HD timing mode")
        # Do not assign a transition interval to either visual mode. Old logs
        # omit the additive field and remain valid with unavailable mode.
        timing_rows.append((power, timing, row, count == 0 or timing == previous_timing))
        previous_timing = timing
        if previous >= 0 and elapsed - previous > 2.5:
            gaps += 1
        previous = elapsed
        count += 1
        groups[power].append(row)
        cpu = row.get("game_cpu_ms_per_frame")
        if identity > 0 and identity not in seen_ids and number(cpu):
            windows[power].append(cpu)
            seen_ids.add(identity)
    if count > 3600:
        raise ValueError("Too many sample rows")
    result = {"schema": "nfsiise-session-summary-v1", "build": build,
              "samples": count, "last_sample_elapsed_s": previous if count else None,
              "has_end_record": end is not None,
              "clean_end_record": end is not None and end["reason"] not in {"write_error", "writer_backlog"} and end["attempted_rows"] == count,
              "ignored_incomplete_tail": partial_tail,
              "end_reason": end.get("reason") if end else None,
              "sampling_gaps_over_2_5s": gaps,
              "scope": "Sampled FPS and CPU windows; not exact race frame counts or measured energy",
              "low_power_mode": {}}
    for power, samples in groups.items():
        def values(key, maximum=1e15):
            return [r[key] for r in samples if number(r.get(key), maximum)]
        batteries = values("battery_percent", 100)
        result["low_power_mode"]["on" if power else "off"] = {
            "samples": len(samples),
            "fps_observations": distribution(values("presentation_fps", 10000)),
            "game_cpu_ms_per_frame_unique_windows": distribution(windows[power]),
            "process_cpu_ms_interval": distribution(values("process_cpu_ms_interval")),
            "main_cpu_ms_interval": distribution(values("main_cpu_ms_interval")),
            "ram_mb": distribution(values("ram_mb")),
            "battery_first_observed_percent": batteries[0] if batteries else None,
            "battery_last_observed_percent": batteries[-1] if batteries else None,
            "battery_caution": "Coarse percentage; pooled mode samples may be noncontiguous. No energy or drain-rate inference."}
    result["timing_art_comparison"] = {}
    for power in (False, True):
        modes = {}
        for mode, label in ((False, "original"), (True, "hd_requested"), (None, "unavailable")):
            selected = [r for p, m, r, stable in timing_rows if p == power and m is mode and stable]
            ids = set()
            cpu = []
            for row in selected:
                identity = row["game_cpu_sample_id"]
                value = row.get("game_cpu_ms_per_frame")
                if identity > 0 and identity not in ids and number(value):
                    ids.add(identity)
                    cpu.append(value)
            modes[label] = {
                "stable_samples": len(selected),
                "fps_observations": distribution([r["presentation_fps"] for r in selected if number(r.get("presentation_fps"), 10000)]),
                "game_cpu_ms_per_frame_unique_windows": distribution(cpu)}
        result["timing_art_comparison"]["low_power_on" if power else "low_power_off"] = modes
    result["timing_art_caution"] = "Requested timing art only; unknown atlases fall back. First sample after mode changes excluded. Partial FPS/CPU windows are not energy measurements."
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    args = parser.parse_args()
    try:
        with args.log.open("rb") as stream:
            data = stream.read(MAX_BYTES + 1)
        print(json.dumps(summarize(data), indent=2, allow_nan=False))
    except (OSError, ValueError):
        parser.exit(1, "Unable to summarize: missing, unsupported or malformed recorder log.\n")


if __name__ == "__main__":
    main()
