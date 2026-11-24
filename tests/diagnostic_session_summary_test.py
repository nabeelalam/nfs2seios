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

    def test_trial_comparison_excludes_power_transitions_and_gaps(self):
        def row(t, power):
            return dict(sample(t, power=power, identity=t+1), transform_trial_requested="cache",
                        transform_trial_generation=2, transform_trial_interval_stable=t>0)
        rows = [row(0, False), row(1, False), row(2, True), row(3, True), row(8, True)]
        result = summary.summarize(fixture(rows))["transform_trial_comparison"]
        self.assertEqual(result["off"]["cache"]["stable_samples"], 1)
        self.assertEqual(result["on"]["cache"]["stable_samples"], 1)

    def test_transform_trial_generation_and_transition_grouping(self):
        def row(t, mode, generation, stable):
            return dict(sample(t, identity=t+1), transform_trial_requested=mode,
                        transform_trial_generation=generation,
                        transform_trial_interval_stable=stable)
        rows = [row(0, "baseline", 1, False), row(1, "baseline", 1, True),
                row(2, "cache", 2, False), row(3, "cache", 2, True),
                row(4, "cache", 4, False), row(5, "cache", 4, True)]
        result = summary.summarize(fixture(rows))["transform_trial_comparison"]["off"]
        self.assertEqual(result["baseline"]["stable_samples"], 1)
        self.assertEqual(result["cache"]["stable_samples"], 2)
        invalid = [dict(rows[1], **change) for change in (
            {"transform_trial_generation": True}, {"transform_trial_requested": "other"},
            {"transform_trial_requested": "unavailable"}, {"transform_trial_generation": 0},
            {"transform_trial_interval_stable": 1})]
        for bad in invalid:
            with self.assertRaises(ValueError):
                summary.summarize(fixture([rows[0], bad]))
        with self.assertRaises(ValueError):
            summary.summarize(fixture([rows[0], dict(rows[2], transform_trial_interval_stable=True)]))
        with self.assertRaises(ValueError):
            summary.summarize(fixture([rows[2], row(3, "baseline", 1, False)]))
        with self.assertRaises(ValueError):
            summary.summarize(fixture([dict(sample(0), transform_trial_requested="baseline")]))



    def test_main_wait_metrics_use_matched_durations_and_reason_counts(self):
        def row(time, duration, count, total, maximum):
            return dict(sample(time), interval_s=duration, main_wait_calls=count,
                        main_wait_elapsed_ms=total, main_wait_max_ms=maximum,
                        main_wait_finished=0, main_wait_stopped=0,
                        main_wait_timed_out=count / 2, main_wait_handled_source=count / 2,
                        main_wait_other=0)
        rows = [row(0, 1, 100, 100, 2), row(1, 1, 100, 100, 2),
                row(3, 2, 200, 400, 3)]
        result = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertEqual(result["main_wait_measured_seconds"], 3)
        self.assertEqual(result["main_wait_calls_per_second"], 100)
        self.assertAlmostEqual(result["main_wait_elapsed_ms_per_call"], 500 / 300)
        self.assertEqual(result["main_wait_timed_out_fraction"], 0.5)
        self.assertEqual(result["main_wait_max_ms"], 3)
        bad = [dict(rows[1], **changes) for changes in (
            {"main_wait_calls": True}, {"main_wait_calls": 0.5},
            {"main_wait_other": 1}, {"main_wait_timed_out": None},
            {"main_wait_elapsed_ms": 1001}, {"main_wait_max_ms": 101},
            {"interval_s": 0}, {"main_wait_finished": float("nan")})]
        result = summary.main_wait_metrics(bad + [sample(1)])
        self.assertEqual(result["main_wait_measured_seconds"], 0)
        self.assertIsNone(result["main_wait_calls_per_second"])
        zero = row(1, 1, 0, 0, 0)
        result = summary.main_wait_metrics([zero])
        self.assertEqual(result["main_wait_calls_per_second"], 0)
        self.assertIsNone(result["main_wait_timed_out_fraction"])


    def test_event_pump_rate_uses_matched_durations_and_preserves_missing_data(self):
        rows = [dict(sample(0), interval_s=1, foreground_event_pumps_interval=999),
                dict(sample(1), interval_s=1, foreground_event_pumps_interval=1000),
                dict(sample(3), interval_s=2, foreground_event_pumps_interval=1000),
                dict(sample(4), interval_s=1, foreground_event_pumps_interval=True),
                dict(sample(5), interval_s=0, foreground_event_pumps_interval=500),
                dict(sample(6), interval_s=1, foreground_event_pumps_interval=1.5)]
        rate = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertEqual(rate["foreground_event_pump_samples_seconds"], 3)
        self.assertAlmostEqual(rate["foreground_event_pumps_per_second"], 2000 / 3)
        old = summary.summarize(fixture([sample(0), sample(1)]))
        self.assertIsNone(old["stable_interval_comparison"]["off"]["foreground_event_pumps_per_second"])

    def test_power_comparison_reports_absolute_metrics_alongside_deltas(self):
        def recording(power, fps):
            rows = [dict(sample(i, power=power, fps=fps), frame_limit=0,
                         render_width=2360, render_height=1640) for i in range(2)]
            rows.append(dict(type="end", reason="manual", elapsed_s=2, attempted_rows=2))
            return fixture(rows).replace(b'"build": 19', b'"build": 19, "source_commit": "' + b'a' * 40 + b'"')

        report = summary.compare_power_sessions(recording(False, 60), recording(True, 45))
        self.assertEqual(report["paired_metrics"]["low_power_off"]["median_sample_fps"], 60)
        self.assertEqual(report["paired_metrics"]["low_power_on"]["median_sample_fps"], 45)
        self.assertEqual(report["low_power_on_minus_off"]["median_sample_fps"], -15)

    def test_oversized_optional_metric_is_ignored(self):
        report = summary.summarize(fixture([sample(0, fps=10**400)]))
        self.assertEqual(report["low_power_mode"]["off"]["fps_observations"]["count"], 0)

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
        rows = [dict(sample(i), interval_s=1, classified_presentations=float(classified),
                     race_world_presentations=float(world))
                for i, (classified, world) in enumerate(((60, 0), (60, 30), (60, 60), (45, 45), (40.5, 40.5)))]
        race = summary.summarize(fixture(rows))["race_world_interval_comparison"]["off"]
        self.assertEqual(race["samples"], 2)
        self.assertEqual(race["presentation_fps"]["median"], 52.5)
        legacy = summary.summarize(fixture([sample(0), sample(1)]))
        self.assertEqual(legacy["race_world_interval_comparison"]["off"]["samples"], 0)

    def test_observed_race_fps_uses_duration_and_excludes_invalid_intervals(self):
        def row(second, duration, frames):
            return dict(sample(second), interval_s=duration,
                        classified_presentations=frames, race_world_presentations=frames)
        rows = [row(0, 1, 60), row(1, 1, 60), row(3, 2, 60), row(4, 0, 60), row(5, None, 60)]
        race = summary.summarize(fixture(rows))["race_world_interval_comparison"]["off"]
        self.assertEqual(race["samples"], 2)
        self.assertEqual(race["observed_race_seconds"], 3)
        self.assertEqual(race["observed_race_presentations"], 120)
        self.assertEqual(race["overall_presentation_fps"], 40)
        self.assertEqual(race["presentation_fps"]["median"], 45)

    def test_race_cpu_per_presentation_uses_matching_cpu_and_frame_counts(self):
        def row(second, frames, world, process, main, power=False):
            return dict(sample(second, power=power), interval_s=1,
                        classified_presentations=float(frames),
                        race_world_presentations=float(world),
                        process_cpu_ms_interval=process, main_cpu_ms_interval=main)

        rows = [row(0, 60, 60, 900, 200), row(1, 60, 60, 600, 120),
                row(2, 30, 30, 450, None), row(3, 60, 30, 1200, 500),
                row(4, 60, 60, 1800, 500, power=True), row(5, 40, 40, None, None, power=True)]
        race = summary.summarize(fixture(rows))["race_world_interval_comparison"]
        self.assertAlmostEqual(race["off"]["process_cpu_ms_per_presentation"], 1050 / 90)
        self.assertEqual(race["off"]["main_cpu_ms_per_presentation"], 2)
        self.assertIsNone(race["on"]["process_cpu_ms_per_presentation"])

    def test_desktop_summary_is_separate_from_device_comparison(self):
        session = {"type": "session", "schema": "nfsiise-session-v1", "build": 0}
        rows = [dict(sample(i, power=None), interval_s=1, frame_limit=0,
                     classified_presentations=45.0, race_world_presentations=45.0,
                     swap_intervals_count=44.0, swap_intervals_over_18ms=40.0)
                for i in range(3)]
        log = b"".join((json.dumps(row) + "\n").encode() for row in [session, *rows])
        report = summary.summarize(log, desktop=True)
        self.assertNotIn("low_power_mode", report)
        self.assertEqual(report["race_world_interval_comparison"]["all"]["samples"], 2)
        self.assertEqual(report["race_world_interval_comparison"]["all"]["unlocked_longest_majority_slow_run_s"], 2)
        with self.assertRaisesRegex(ValueError, "Invalid build"):
            summary.compare_power_sessions(log, log)

    def test_majority_slow_runs_are_broken_by_filtering_and_single_dips(self):
        def row(second, slow, world=60):
            return dict(sample(second), interval_s=1, frame_limit=0,
                        classified_presentations=60, race_world_presentations=world,
                        swap_intervals_count=50, swap_intervals_over_18ms=slow)

        rows = [row(0, 50), row(1, 25), row(2, 27), row(3, 50, world=10),
                row(4, 45), row(5, 1), row(6, 50), row(7, 0), row(8, 50)]
        result = summary.summarize(fixture(rows))["race_world_interval_comparison"]["off"]
        self.assertEqual(result["unlocked_pacing_samples"], 7)
        self.assertAlmostEqual(result["unlocked_majority_slow_sample_fraction"], 5 / 7)
        self.assertEqual(result["unlocked_longest_majority_slow_run_s"], 2)
        legacy = summary.summarize(fixture([sample(0), sample(1)]))
        self.assertIsNone(legacy["stable_interval_comparison"]["off"]["unlocked_longest_majority_slow_run_s"])

    def test_hitch_fraction_uses_only_eligible_samples_with_a_maximum(self):
        def row(second, slow, longest):
            return dict(sample(second), interval_s=1, frame_limit=0,
                        classified_presentations=60, race_world_presentations=60,
                        swap_intervals_count=50, swap_intervals_over_18ms=slow,
                        swap_interval_max_ms=longest)

        rows = [row(0, 1, 100), row(1, 1, 50), row(2, 1, 49.9),
                row(3, 25, 100), row(4, 1, None), row(5, 0, 0)]
        pacing = summary.summarize(fixture(rows))["race_world_interval_comparison"]["off"]
        self.assertEqual(pacing["unlocked_non_majority_slow_samples_with_maximum"], 2)
        self.assertEqual(pacing["unlocked_non_majority_slow_sample_fraction_with_50ms_hitch"], .5)
        legacy = summary.summarize(fixture(rows[-2:]))["race_world_interval_comparison"]["off"]
        self.assertIsNone(legacy["unlocked_non_majority_slow_sample_fraction_with_50ms_hitch"])

    def test_display_link_rate_is_independent_of_game_fps(self):
        rows = [sample(0), dict(sample(1, fps=40), interval_s=1.2,
                              display_link_callbacks=72, display_link_mean_target_ms=16.667)]
        cadence = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertAlmostEqual(cadence["display_link_callbacks_per_second"]["median"], 60)
        self.assertEqual(cadence["display_link_mean_target_ms"]["median"], 16.667)

    def test_display_callback_lateness_uses_measured_callbacks(self):
        rows = [sample(0),
                dict(sample(1), interval_s=1, display_link_callbacks=60,
                     display_link_late_callbacks=3, display_link_max_late_ms=6),
                dict(sample(2), interval_s=1, display_link_callbacks=30,
                     display_link_late_callbacks=6, display_link_max_late_ms=15),
                dict(sample(3), interval_s=1, display_link_callbacks=60)]
        pacing = summary.summarize(fixture(rows))["stable_interval_comparison"]["off"]
        self.assertAlmostEqual(pacing["display_link_late_callback_fraction"], 0.1)
        self.assertEqual(pacing["display_link_max_late_ms"], 15)
        legacy = summary.summarize(fixture(rows[-1:]))["stable_interval_comparison"]["off"]
        self.assertIsNone(legacy["display_link_late_callback_fraction"])

    def test_power_comparison_uses_race_extent_and_pacing(self):
        def log(power, race_width=800, race_lock=0, tag_world=True):
            rows = [{"type": "session", "schema": "nfsiise-session-v1",
                     "build": 23, "source_commit": "a" * 40}]
            for i in range(5):
                race = i >= 2
                count = (45 if power else 60) if race else 10
                rows.append(dict(sample(i, power=power, fps=10), interval_s=1,
                                 render_width=race_width if race else 640,
                                 render_height=450 if race else 480,
                                 frame_limit=race_lock if race else 0,
                                 classified_presentations=count,
                                 race_world_presentations=count if race and tag_world else 0,
                                 swap_intervals_count=count,
                                 swap_intervals_over_18ms=count if race and power else 0,
                                 renderer_calls_interval=count * (3 if race else 99),
                                 renderer_queue_ms_interval=count / 2 if i != 3 else None,
                                 renderer_execution_ms_interval=count * (2 if power else 1),
                                 renderer_execution_max_ms=(7 if power else 4) if i != 4 else 200))
            rows.append(dict(type="end", reason="manual", elapsed_s=5, attempted_rows=5))
            return b"".join((json.dumps(row) + "\n").encode() for row in rows)

        normal = log(False)
        result = summary.compare_power_sessions(normal, log(True))
        self.assertEqual(result["comparison_scope"], "race_world")
        self.assertEqual(result["render_size"], (800, 450))
        self.assertEqual(result["low_power_on_minus_off"]["median_sample_fps"], -15)
        self.assertEqual(result["low_power_on_minus_off"]["fraction_swaps_over_18ms"], 1)
        self.assertIsNone(result["low_power_on_minus_off"]["median_game_cpu_ms"])
        race_on = result["low_power_on"]["race_world_interval_comparison"]["on"]
        self.assertEqual(race_on["renderer_calls_per_presentation"], 3)
        self.assertEqual(race_on["renderer_execution_max_ms"], 7)
        self.assertEqual(result["low_power_on_minus_off"]["renderer_execution_max_ms"], 3)
        self.assertEqual(race_on["renderer_queue_ms_per_presentation"], 0.5)
        self.assertEqual(result["low_power_on_minus_off"]["renderer_execution_ms_per_presentation"], 1)
        for invalid in (log(True, race_width=900), log(True, race_lock=30),
                        log(True, tag_world=False)):
            with self.assertRaises(ValueError):
                summary.compare_power_sessions(normal, invalid)

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
        self.assertEqual(report["comparison_scope"], "whole_session")
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
