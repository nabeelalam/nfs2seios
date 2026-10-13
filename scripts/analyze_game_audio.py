#!/usr/bin/env python3
"""Validate locally captured SDL disk PCM and wrap it as a WAV for listening."""
import argparse
from array import array
import json
import math
from pathlib import Path
import re
import sys
import wave


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("raw", type=Path)
    parser.add_argument("log", type=Path)
    parser.add_argument("wav", type=Path)
    args = parser.parse_args()
    metadata = re.search(r"format=S16LE channels=(\d+) freq=(\d+)", args.log.read_text())
    if not metadata:
        parser.error("Run log must identify the recorded S16LE format, channels and rate")
    channels, rate = map(int, metadata.groups())
    if channels not in (1, 2) or not 8000 <= rate <= 192000:
        parser.error("Unexpected channel count or sample rate")
    if args.raw.stat().st_size > 128 * 1024 * 1024:
        parser.error("Capture exceeds the bounded analysis size")
    data = args.raw.read_bytes()
    if not data or len(data) % (channels * 2):
        parser.error("PCM is empty or has an incomplete sample frame")
    samples = array("h", data)
    if sys.byteorder != "little":
        samples.byteswap()
    nonzero = sum(value != 0 for value in samples)
    report = {
        "format": "S16LE", "channels": channels, "sampleRate": rate,
        "bytes": len(data), "durationSeconds": len(data) / (channels * 2 * rate),
        "nonzeroSamples": nonzero, "peak": max(abs(value) for value in samples),
        "rms": math.sqrt(sum(value * value for value in samples) / len(samples)),
        "railSamples": sum(value in (-32768, 32767) for value in samples),
        "scope": "Captured game PCM; no audible correctness or iPad audio claim"
    }
    if args.wav.resolve() in (args.raw.resolve(), args.log.resolve()):
        parser.error("WAV destination must be separate from capture and log")
    with wave.open(str(args.wav), "wb") as output:
        output.setnchannels(channels)
        output.setsampwidth(2)
        output.setframerate(rate)
        output.writeframes(data)
    args.wav.with_suffix(".json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))
    return 0 if nonzero else 1


if __name__ == "__main__":
    raise SystemExit(main())
