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
    def test_modes_and_repeated_windows(self):
        result = summary.summarize(fixture([sample(0), sample(1), sample(2, True, 2, 45), {"type": "end", "reason": "manual", "elapsed_s": 3, "attempted_rows": 3}]))
        self.assertEqual(result["samples"], 3)
        self.assertTrue(result["clean_end_record"])
        self.assertEqual(result["low_power_mode"]["off"]["game_cpu_ms_per_frame_unique_windows"]["count"], 1)
        self.assertEqual(result["low_power_mode"]["on"]["fps_observations"]["median"], 45)
        self.assertEqual(result["timing_art_comparison"]["low_power_off"]["unavailable"]["stable_samples"], 2)

    def test_interruption_and_gap(self):
        result = summary.summarize(fixture([sample(0), sample(8)]) + b'{"type":')
        self.assertTrue(result["ignored_incomplete_tail"])
        self.assertFalse(result["clean_end_record"])
        self.assertEqual(result["sampling_gaps_over_2_5s"], 1)

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
