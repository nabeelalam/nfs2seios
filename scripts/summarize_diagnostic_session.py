"""Summarize a privately supplied recorder log without printing paths or identifiers."""
import argparse
import json
import math
import re
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


def stable_interval_metrics(rows):
    slow = total = 0
    unlocked_swaps = unlocked_slow = 0
    swap_calls = 0
    swap_host_ms = 0
    swap_host_maxima = []
    queue = []
    execution = []
    display_rates = []
    display_targets = []
    for row in rows:
        swaps = row.get("swap_intervals_count")
        late = row.get("swap_intervals_over_20ms")
        if number(swaps) and number(late, swaps):
            total += swaps
            slow += late
        late_unlocked = row.get("swap_intervals_over_18ms")
        if (type(row.get("frame_limit")) is int and row["frame_limit"] == 0
                and number(swaps) and number(late_unlocked, swaps)):
            unlocked_swaps += swaps
            unlocked_slow += late_unlocked
        measured_calls = row.get("swap_calls_count")
        host_total = row.get("swap_call_total_ms")
        host_max = row.get("swap_call_max_ms")
        if (number(swaps) and number(measured_calls, swaps) and measured_calls >= 1
                and number(host_total) and number(host_max, host_total)):
            swap_calls += measured_calls
            swap_host_ms += host_total
            swap_host_maxima.append(host_max)
        callbacks = row.get("display_link_callbacks")
        interval = row.get("interval_s")
        target = row.get("display_link_mean_target_ms")
        if number(callbacks, 10000) and number(interval, 2.5) and interval > 0:
            display_rates.append(callbacks / interval)
            if callbacks and number(target, 1000) and target > 0:
                display_targets.append(target)
        calls = row.get("renderer_calls_interval")
        if number(calls) and calls >= 1:
            for key, values in (("renderer_queue_ms_interval", queue),
                                ("renderer_execution_ms_interval", execution)):
                elapsed = row.get(key)
                if number(elapsed):
                    values.append(elapsed / calls)
    return {
        "sample_intervals": len(rows),
        "observed_swap_intervals": total,
        "swap_fraction_over_20ms": slow / total if total else None,
        "unlocked_observed_swap_intervals": unlocked_swaps,
        "unlocked_swap_fraction_over_18ms": unlocked_slow / unlocked_swaps if unlocked_swaps else None,
        "measured_swap_calls": swap_calls,
        "swap_host_ms_per_call": swap_host_ms / swap_calls if swap_calls else None,
        "swap_host_max_ms": max(swap_host_maxima, default=None),
        "display_link_callbacks_per_second": distribution(display_rates),
        "display_link_mean_target_ms": distribution(display_targets),
        "renderer_queue_ms_per_call": distribution(queue),
        "renderer_execution_ms_per_call": distribution(execution),
    }


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
    source = rows[0].get("source_commit")
    if "source_commit" in rows[0] and (not isinstance(source, str) or re.fullmatch("[0-9a-f]{40}", source) is None):
        raise ValueError("Invalid source commit")
    groups = {False: [], True: []}
    windows = {False: [], True: []}
    seen_ids = set()
    previous = -1
    end = None
    gaps = 0
    count = 0
    timing_rows = []
    previous_timing = None
    previous_power = None
    previous_limit = None
    previous_render_size = None
    stable_intervals = {False: [], True: []}
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
        limit = row.get("frame_limit")
        render_size = (row.get("render_width"), row.get("render_height"))
        if (previous >= 0 and elapsed - previous <= 2.5
                and power == previous_power and timing == previous_timing and limit == previous_limit
                and render_size == previous_render_size):
            stable_intervals[power].append(row)
        previous_timing = timing
        previous_power = power
        previous_limit = limit
        previous_render_size = render_size
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
            "render_sizes": sorted({(r["render_width"], r["render_height"]) for r in samples
                                  if all(type(r.get(k)) is int and 0 < r[k] <= 16384
                                         for k in ("render_width", "render_height"))}),
            "frame_limits": sorted({r["frame_limit"] for r in samples
                                    if type(r.get("frame_limit")) is int}),
            "timing_art_modes": [label for mode, label in
                                 ((False, "original"), (True, "hd_requested"), (None, "unavailable"))
                                 if any(r.get("hd_timing_requested") is mode for r in samples)],
            "complete_render_conditions": all(
                type(r.get("frame_limit")) is int
                and all(type(r.get(k)) is int and 0 < r[k] <= 16384
                        for k in ("render_width", "render_height")) for r in samples),
            "brightness": distribution(values("brightness", 1)),
            "thermal_state": distribution(values("thermal_state", 3)),
            "fps_observations": distribution(values("presentation_fps", 10000)),
            "game_cpu_ms_per_frame_unique_windows": distribution(windows[power]),
            "process_cpu_ms_interval": distribution(values("process_cpu_ms_interval")),
            "main_cpu_ms_interval": distribution(values("main_cpu_ms_interval")),
            "swap_intervals_count": sum(values("swap_intervals_count")) if values("swap_intervals_count") else None,
            "swap_intervals_over_20ms": sum(values("swap_intervals_over_20ms")) if values("swap_intervals_over_20ms") else None,
            "swap_interval_max_ms": max(values("swap_interval_max_ms"), default=None),
            "renderer_calls_interval": distribution(values("renderer_calls_interval")),
            "renderer_queue_ms_interval": distribution(values("renderer_queue_ms_interval")),
            "renderer_execution_ms_interval": distribution(values("renderer_execution_ms_interval")),
            "ram_mb": distribution(values("ram_mb")),
            "battery_first_observed_percent": batteries[0] if batteries else None,
            "battery_last_observed_percent": batteries[-1] if batteries else None,
            "battery_caution": "Coarse percentage; pooled mode samples may be noncontiguous. No energy or drain-rate inference."}
    if source is not None:
        result["source_commit"] = source
    result["stable_interval_comparison"] = {
        label: stable_interval_metrics(stable_intervals[power])
        for label, power in (("off", False), ("on", True))
    }
    race_intervals = {
        label: [row for row in stable_intervals[power]
                if type(row.get("classified_presentations")) is int
                and 0 < row["classified_presentations"] <= 16384
                and type(row.get("race_world_presentations")) is int
                and row["race_world_presentations"] == row["classified_presentations"]]
        for label, power in (("off", False), ("on", True))
    }
    result["race_world_interval_comparison"] = {
        label: {
            "samples": len(rows),
            "presentation_fps": distribution([
                row["classified_presentations"] / row["interval_s"] for row in rows
                if number(row.get("interval_s"), 2.5)
                and row["interval_s"] * 10000 >= row["classified_presentations"]]),
            **stable_interval_metrics(rows),
        }
        for label, rows in race_intervals.items()
    }
    result["race_world_interval_scope"] = (
        "Only steady samples with a race-world draw in every classified presentation. "
        "A race-world draw can occur during replays or beneath an overlay. Older logs lack this count."
    )
    result["stable_interval_caution"] = (
        "Excludes the first sample, sampling gaps over 2.5 s and intervals ending at an observed power, timing-art, render-size or frame-lock change. "
        "Renderer values are per-call means within each interval. Compare matched race and graphics settings. "
        "The 18 ms threshold is a near-60 FPS pacing indicator for the unlocked mode. Display-link rates describe main run-loop delivery, not displayed game frames. Swap spacing measures presentations. Swap-call duration is host elapsed time and can include display or driver waiting."
    )
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


def compare_power_sessions(normal, low_power):
    off, on = summarize(normal), summarize(low_power)
    source = off.get("source_commit")
    if (not source or source != on.get("source_commit") or off["build"] != on["build"]
            or not off["clean_end_record"] or not on["clean_end_record"]):
        raise ValueError("Comparison needs complete sessions from the same source and build")
    conditions = []
    for report, label, other in ((off, "off", "on"), (on, "on", "off")):
        mode = report["low_power_mode"][label]
        if (not mode["samples"] or report["low_power_mode"][other]["samples"]
                or mode["frame_limits"] != [0] or len(mode["render_sizes"]) != 1
                or not mode["complete_render_conditions"]):
            raise ValueError("Comparison needs steady power mode, a fixed render size and the 30 FPS lock off")
        conditions.append(mode)
    if conditions[0]["render_sizes"] != conditions[1]["render_sizes"]:
        raise ValueError("Comparison requires matching render sizes")

    timing_art = conditions[0]["timing_art_modes"]
    if len(timing_art) != 1 or timing_art != conditions[1]["timing_art_modes"]:
        raise ValueError("Comparison requires matching, steady timing artwork settings")

    def metrics(report, label):
        mode = report["low_power_mode"][label]
        pacing = report["stable_interval_comparison"][label]
        return {
            "median_sample_fps": mode["fps_observations"]["median"],
            "median_game_cpu_ms": mode["game_cpu_ms_per_frame_unique_windows"]["median"],
            "median_display_callbacks_per_second": pacing["display_link_callbacks_per_second"]["median"],
            "median_display_target_ms": pacing["display_link_mean_target_ms"]["median"],
            "mean_swap_host_ms": pacing["swap_host_ms_per_call"],
            "fraction_swaps_over_18ms": pacing["unlocked_swap_fraction_over_18ms"],
        }

    left, right = metrics(off, "off"), metrics(on, "on")
    return {
        "schema": "nfsiise-power-comparison-v1", "build": off["build"], "source_commit": source,
        "render_size": conditions[0]["render_sizes"][0], "timing_art_mode": timing_art[0],
        "low_power_off": off, "low_power_on": on,
        "low_power_on_minus_off": {
            key: right[key] - value if value is not None and right[key] is not None else None
            for key, value in left.items()
        },
        "scope": ("Confirm matched track, cars, graphics and driving separately. "
                  "Battery percentage and thermal conditions are context, not an energy measurement."),
    }


def read_bounded(path):
    with path.open("rb") as stream:
        return stream.read(MAX_BYTES + 1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path, help="session log, or the Low Power Mode off log for comparison")
    parser.add_argument("--compare-low-power", type=Path, help="matching Low Power Mode on log")
    args = parser.parse_args()
    try:
        data = read_bounded(args.log)
        report = (compare_power_sessions(data, read_bounded(args.compare_low_power))
                  if args.compare_low_power else summarize(data))
        print(json.dumps(report, indent=2, allow_nan=False))
    except (OSError, ValueError):
        parser.exit(1, "Unable to summarize: missing, unsupported, malformed or unmatched recorder log.\n")


if __name__ == "__main__":
    main()
