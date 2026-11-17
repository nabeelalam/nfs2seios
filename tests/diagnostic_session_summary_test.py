import importlib.util
import json
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location("summary", Path(__file__).resolve().parents[1] / "scripts/summarize_diagnostic_session.py")
summary = importlib.util.module_from_spec(spec)
spec.loader.exec_module(summary)


def fixture(rows):
    return b"".join((json.dumps(row) + "\n").encode() for row in [{"type": "session", "schema": "nfsiise-session-v1", "build": 19}, *rows])


def sample(time, power=False, identity=1, fps=60):
    return dict(type="sample", elapsed_s=time, low_power_mode=power,
                game_cpu_sample_id=identity, game_cpu_ms_per_frame=12,
                presentation_fps=fps, battery_percent=50)


class SummaryTests(unittest.TestCase):
    def test_unlocked_cadence_excludes_frame_lock_transitions_and_old_rows(self):
        def row(second, limit, late=None):
            result = dict(sample(second), frame_limit=limit, swap_intervals_count=50)
            if late is not None:
                result["swap_intervals_over_18ms"] = late
            return result

        rows = [row(0, 0, 50), row(1, 0, 5), row(2, 30, 50), row(3, 30, 50),
                row(4, 0, 50), row(5, 0, 10), row(6, 0)]
        cadence = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertEqual(cadence["unlocked_observed_swap_intervals"], 100)
        self.assertAlmostEqual(cadence["unlocked_swap_fraction_over_18ms"], 0.15)
        old_log = summary.summarize(fixture([row(0, 0), row(1, 0)]))
        self.assertIsNone(old_log["stable_interval_comparison"]["off"]["unlocked_swap_fraction_over_18ms"])

    def test_race_world_comparison_excludes_menu_and_mixed_samples(self):
        rows = [dict(sample(i), interval_s=1, classified_presentations=classified,
                     race_world_presentations=world)
                for i, (classified, world) in enumerate(((60, 0), (60, 30), (60, 60), (45, 45)))]
        race = summary.summarize(fixture(rows))["race_world_interval_comparison"]["off"]
        self.assertEqual(race["samples"], 2)
        self.assertEqual(race["presentation_fps"]["median"], 52.5)
        legacy = summary.summarize(fixture([sample(0), sample(1)]))
        self.assertEqual(legacy["race_world_interval_comparison"]["off"]["samples"], 0)

    def test_display_link_rate_is_independent_of_game_fps(self):
        rows = [sample(0), dict(sample(1, fps=40), interval_s=1.2,
                              display_link_callbacks=72, display_link_mean_target_ms=16.667)]
        cadence = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertAlmostEqual(cadence["display_link_callbacks_per_second"]["median"], 60)
        self.assertEqual(cadence["display_link_mean_target_ms"]["median"], 16.667)

    def test_matched_power_comparison_validates_conditions(self):
        def log(power, width=2360, lock=0, finished=True, commit="a" * 40,
                timing=(None, None, None)):
            rows = [{"type": "session", "schema": "nfsiise-session-v1",
                     "build": 19, "source_commit": commit}]
            for second in (0, 1, 2):
                rows.append(dict(sample(second, power=power, fps=45 if power else 60),
                                 frame_limit=lock, render_width=width, render_height=1640,
                                 hd_timing_requested=timing[second],
                                 interval_s=1, swap_intervals_count=40,
                                 swap_intervals_over_18ms=20 if power else 0))
            if finished:
                rows.append(dict(type="end", reason="manual", elapsed_s=3, attempted_rows=3))
            return b"".join((json.dumps(row) + "\n").encode() for row in rows)

        report = summary.compare_power_sessions(log(False), log(True))
        self.assertEqual(report["render_size"], (2360, 1640))
        self.assertEqual(report["timing_art_mode"], "unavailable")
        hd = (True, True, True)
        hd_report = summary.compare_power_sessions(log(False, timing=hd), log(True, timing=hd))
        self.assertEqual(hd_report["timing_art_mode"], "hd_requested")
        self.assertEqual(report["low_power_on_minus_off"]["median_sample_fps"], -15)
        self.assertEqual(report["low_power_on_minus_off"]["fraction_swaps_over_18ms"], 0.5)
        self.assertIsNone(report["low_power_on_minus_off"]["mean_swap_host_ms"])
        for invalid in (log(False), log(True, width=1280), log(True, lock=30),
                        log(True, finished=False), log(True, commit="b" * 40),
                        log(True, timing=hd), log(True, timing=(None, True, None))):
            with self.assertRaises(ValueError):
                summary.compare_power_sessions(log(False), invalid)

    def test_resize_boundary_excluded_from_stable_intervals(self):
        rows = [dict(sample(second), render_width=width, render_height=height,
                     frame_limit=0, swap_intervals_count=50, swap_intervals_over_18ms=late)
                for second, width, height, late in ((0, 800, 450, 0), (1, 800, 450, 5),
                    (2, 708, 492, 40), (3, 708, 492, 10))]
        cadence = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertEqual(cadence["unlocked_observed_swap_intervals"], 100)
        self.assertAlmostEqual(cadence["unlocked_swap_fraction_over_18ms"], 0.15)

    def test_modes_and_repeated_windows(self):
        result = summary.summarize(fixture([sample(0), sample(1), sample(2, True, 2, 45), {"type": "end", "reason": "manual", "elapsed_s": 3, "attempted_rows": 3}]))
        self.assertEqual(result["samples"], 3)
        self.assertTrue(result["clean_end_record"])
        self.assertEqual(result["low_power_mode"]["off"]["game_cpu_ms_per_frame_unique_windows"]["count"], 1)
        self.assertEqual(result["low_power_mode"]["on"]["fps_observations"]["median"], 45)
        self.assertEqual(result["timing_art_comparison"]["low_power_off"]["unavailable"]["stable_samples"], 2)

    def test_optional_renderer_intervals_by_power_mode(self):
        rows = [sample(0),
                dict(sample(1), renderer_calls_interval=160,
                     renderer_queue_ms_interval=7.25, renderer_execution_ms_interval=11.5,
                     swap_intervals_count=60, swap_intervals_over_20ms=2, swap_interval_max_ms=24),
                dict(sample(2, True, 2), renderer_calls_interval=None,
                     renderer_queue_ms_interval=None, renderer_execution_ms_interval=None),
                dict(sample(3, True, 3), renderer_calls_interval=120,
                     renderer_queue_ms_interval=6, renderer_execution_ms_interval=13)]
        modes = summary.summarize(fixture(rows))["low_power_mode"]
        self.assertEqual(modes["off"]["renderer_calls_interval"]["count"], 1)
        self.assertEqual(modes["off"]["swap_intervals_count"], 60)
        self.assertEqual(modes["off"]["swap_intervals_over_20ms"], 2)
        self.assertEqual(modes["off"]["swap_interval_max_ms"], 24)
        self.assertIsNone(modes["on"]["swap_intervals_count"])
        self.assertEqual(modes["off"]["renderer_queue_ms_interval"]["median"], 7.25)
        self.assertEqual(modes["on"]["renderer_execution_ms_interval"]["median"], 13)

    def test_stable_intervals_exclude_gaps_and_mode_changes(self):
        def timed(time, power, timing, calls, late):
            return dict(sample(time, power), hd_timing_requested=timing,
                        renderer_calls_interval=calls, renderer_queue_ms_interval=12,
                        renderer_execution_ms_interval=20, swap_intervals_count=50,
                        swap_intervals_over_20ms=late, swap_calls_count=calls // 5,
                        swap_call_total_ms=calls, swap_call_max_ms=6)
        rows = [timed(0, False, True, 10, 50),
                timed(1, False, True, 100, 5),
                timed(2, True, True, 10, 50),
                timed(3, True, True, 200, 10),
                timed(8, True, True, 10, 50),
                timed(9, True, False, 10, 50),
                timed(10, True, False, 100, 20),
                timed(11, True, False, 0, 55)]
        modes = summary.summarize(fixture(rows))["stable_interval_comparison"]
        self.assertEqual(modes["off"]["sample_intervals"], 1)
        self.assertEqual(modes["off"]["swap_fraction_over_20ms"], 0.1)
        self.assertEqual(modes["on"]["sample_intervals"], 3)
        self.assertEqual(modes["on"]["observed_swap_intervals"], 100)
        self.assertEqual(modes["on"]["swap_fraction_over_20ms"], 0.3)
        self.assertEqual(modes["on"]["measured_swap_calls"], 60)
        self.assertEqual(modes["on"]["swap_host_ms_per_call"], 5)
        self.assertEqual(modes["on"]["swap_host_max_ms"], 6)
        self.assertEqual(modes["on"]["renderer_execution_ms_per_call"]["count"], 2)
        self.assertAlmostEqual(modes["on"]["renderer_queue_ms_per_call"]["median"], 0.09)

    def test_interruption_and_gap(self):
        result = summary.summarize(fixture([sample(0), sample(8)]) + b'{"type":')
        self.assertTrue(result["ignored_incomplete_tail"])
        self.assertFalse(result["clean_end_record"])
        self.assertEqual(result["sampling_gaps_over_2_5s"], 1)

    def test_source_commit_tracks_build_without_paths(self):
        data = fixture([sample(0)])
        header, samples = data.split(b"\n", 1)
        for value in ("c" * 40, "not-a-commit", None):
            tagged = dict(json.loads(header), source_commit=value)
            log = json.dumps(tagged).encode() + b"\n" + samples
            if value == "c" * 40:
                self.assertEqual(summary.summarize(log)["source_commit"], value)
            else:
                with self.assertRaises(ValueError):
                    summary.summarize(log)

    def test_unknown_readings(self):
        row = sample(0, identity=0, fps=None)
        row["game_cpu_ms_per_frame"] = None
        result = summary.summarize(fixture([row]))["low_power_mode"]["off"]
        self.assertEqual(result["fps_observations"]["count"], 0)
        self.assertEqual(result["game_cpu_ms_per_frame_unique_windows"]["count"], 0)

    def test_malformed_complete_rows_rejected(self):
        for data in [fixture([sample(0)]) + b'bad\n', fixture([sample(1), sample(0)]),
                     fixture([dict(sample(0), low_power_mode=1)]), fixture([sample(0), {"type": "end"}, sample(1)])]:
            with self.assertRaises(ValueError):
                summary.summarize(data)

    def test_schema_bounds_and_boolean_numbers(self):
        with self.assertRaises(ValueError):
            summary.summarize(b'{"type":"session","schema":"other"}\n')
        with self.assertRaises(ValueError):
            summary.summarize(b" " * (summary.MAX_LINE + 1))
        self.assertFalse(summary.number(True))
        self.assertFalse(summary.number(float("inf")))

    def test_partial_save_and_coarse_battery(self):
        rows = [sample(0), {"type": "end", "reason": "writer_backlog", "elapsed_s": 1, "attempted_rows": 2}]
        result = summary.summarize(fixture(rows))
        self.assertTrue(result["has_end_record"])
        self.assertFalse(result["clean_end_record"])
        self.assertNotIn("battery_watts", result["low_power_mode"]["off"])

    def test_full_length(self):
        result = summary.summarize(fixture([sample(i, bool(i % 2), i + 1) for i in range(3600)]))
        self.assertEqual(result["samples"], 3600)
        self.assertEqual(result["low_power_mode"]["on"]["samples"], 1800)

    def test_hd_modes_split_and_transition_excluded(self):
        rows = [dict(sample(0), hd_timing_requested=False),
                dict(sample(1, identity=2, fps=40), hd_timing_requested=True),
                dict(sample(2, identity=3, fps=58), hd_timing_requested=True)]
        result = summary.summarize(fixture(rows))["timing_art_comparison"]["low_power_off"]
        self.assertEqual(result["original"]["stable_samples"], 1)
        self.assertEqual(result["hd_requested"]["stable_samples"], 1)
        self.assertEqual(result["hd_requested"]["fps_observations"]["median"], 58)
        self.assertEqual(result["hd_requested"]["game_cpu_ms_per_frame_unique_windows"]["count"], 1)
        for value in [0, 1, "true", []]:
            with self.assertRaises(ValueError):
                summary.summarize(fixture([dict(sample(0), hd_timing_requested=value)]))


if __name__ == "__main__":
    unittest.main()
