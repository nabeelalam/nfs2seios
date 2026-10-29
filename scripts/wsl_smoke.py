#!/usr/bin/env python3
"""Bounded Linux/Xvfb game run with screenshots; no cloud or original-data edits.

Run inside xvfb-run with a 640x480x24 screen. Supply an imported working copy.
Screenshots observe the X display; they do not prove gameplay correctness.
"""
import argparse
import ctypes as c
import os
from pathlib import Path
import signal
import struct
import subprocess
import time


def press_key(display, name):
    held = name.endswith("+")
    released = name.endswith("-")
    if held or released:
        name = name[:-1]
    x = c.CDLL("libX11.so.6")
    xt = c.CDLL("libXtst.so.6")
    x.XOpenDisplay.argtypes = [c.c_char_p]
    x.XOpenDisplay.restype = c.c_void_p
    x.XStringToKeysym.argtypes = [c.c_char_p]
    x.XStringToKeysym.restype = c.c_ulong
    x.XKeysymToKeycode.argtypes = [c.c_void_p, c.c_ulong]
    x.XKeysymToKeycode.restype = c.c_uint
    x.XFlush.argtypes = [c.c_void_p]
    x.XCloseDisplay.argtypes = [c.c_void_p]
    xt.XTestFakeKeyEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    connection = x.XOpenDisplay(display.encode())
    if not connection:
        raise RuntimeError("Cannot open X display")
    try:
        key = x.XKeysymToKeycode(connection, x.XStringToKeysym(name.encode()))
        if not key:
            raise ValueError(f"Unknown X key: {name}")
        xt.XTestFakeKeyEvent(connection, key, 0 if released else 1, 0)
        x.XFlush(connection)
        if not held and not released:
            time.sleep(0.1)
            xt.XTestFakeKeyEvent(connection, key, 0, 0)
            x.XFlush(connection)
    finally:
        x.XCloseDisplay(connection)


def capture(display, output):
    x = c.CDLL("libX11.so.6")
    x.XOpenDisplay.argtypes = [c.c_char_p]
    x.XOpenDisplay.restype = c.c_void_p
    x.XDefaultRootWindow.argtypes = [c.c_void_p]
    x.XDefaultRootWindow.restype = c.c_ulong
    x.XGetImage.argtypes = [c.c_void_p, c.c_ulong, c.c_int, c.c_int,
                           c.c_uint, c.c_uint, c.c_ulong, c.c_int]
    x.XGetImage.restype = c.c_void_p
    x.XGetPixel.argtypes = [c.c_void_p, c.c_int, c.c_int]
    x.XGetPixel.restype = c.c_ulong
    x.XDestroyImage.argtypes = [c.c_void_p]
    x.XCloseDisplay.argtypes = [c.c_void_p]
    connection = x.XOpenDisplay(display.encode())
    if not connection:
        raise RuntimeError("Cannot open X display")
    image = None
    try:
        image = x.XGetImage(connection, x.XDefaultRootWindow(connection),
                            0, 0, 640, 480, c.c_ulong(-1).value, 2)
        if not image:
            raise RuntimeError("Cannot capture X display")
        pixels = bytearray()
        for row in range(479, -1, -1):
            for column in range(640):
                value = x.XGetPixel(image, column, row)
                pixels.extend((value & 255, (value >> 8) & 255, (value >> 16) & 255))
        header = b"BM" + struct.pack("<IHHI", 54 + len(pixels), 0, 0, 54)
        header += struct.pack("<IiiHHIIiiII", 40, 640, 480, 1, 24, 0,
                              len(pixels), 0, 0, 0, 0)
        output.write_bytes(header + pixels)
    finally:
        if image:
            x.XDestroyImage(image)
        x.XCloseDisplay(connection)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("assets", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--keys", nargs="*", default=[],
                        help="X key names sent after the first capture, one per stage")
    parser.add_argument("--stages", type=int, default=3)
    parser.add_argument("--audio-driver", choices=("dummy", "disk"), default="dummy")
    parser.add_argument("--graphics", choices=("software", "d3d12"), default="software",
                        help="Per-process Mesa driver request; d3d12 uses existing WSL GPU support")
    parser.add_argument("--shutdown-timeout", type=int, default=30,
                        help="Bounded shutdown allowance, including sanitizer exit checks")
    args = parser.parse_args()
    if not 1 <= args.stages <= 40 or len(args.keys) > 39:
        parser.error("Use 1 to 40 capture stages and at most 39 keys")
    if not 1 <= args.shutdown_timeout <= 60:
        parser.error("Shutdown timeout must be between 1 and 60 seconds")
    args.output.mkdir(parents=True, exist_ok=True)
    environment = dict(os.environ, MESA_GLES_VERSION_OVERRIDE="2.0",
                       SDL_AUDIODRIVER=args.audio_driver)
    if args.graphics == "software":
        environment["LIBGL_ALWAYS_SOFTWARE"] = "1"
        # Keep the software driver resident for meaningful leak checks.
        driver = Path("/usr/lib/x86_64-linux-gnu/dri/swrast_dri.so")
        if driver.exists():
            environment["LD_PRELOAD"] = str(driver)
    else:
        if not Path("/dev/dxg").exists():
            parser.error("d3d12 requires existing WSL GPU support (/dev/dxg)")
        for name in ("LIBGL_ALWAYS_SOFTWARE", "MESA_LOADER_DRIVER_OVERRIDE",
                     "LIBGL_ALWAYS_INDIRECT", "MESA_D3D12_DEFAULT_ADAPTER_NAME"):
            environment.pop(name, None)
        environment.update(EGL_PLATFORM="x11", GALLIUM_DRIVER="d3d12")
    print(f"Local graphics request: {args.graphics}; actual renderer must be verified separately")
    if args.audio_driver == "disk":
        environment["SDL_AUDIO_DISK_OUTPUT_FILE"] = str((args.output / "game-audio.raw").resolve())
    with (args.output / "run.log").open("w") as log:
        process = subprocess.Popen([str(args.executable.resolve()), str(args.assets.resolve())],
                                   env=environment, stdout=log, stderr=subprocess.STDOUT,
                                   start_new_session=True)
        try:
            for stage in range(max(args.stages, len(args.keys) + 1)):
                if stage and stage <= len(args.keys) and args.keys[stage - 1] != "-":
                    press_key(os.environ["DISPLAY"], args.keys[stage - 1])
                time.sleep(5)
                if process.poll() is not None:
                    break
                capture(os.environ["DISPLAY"], args.output / f"screen-{stage + 1}.bmp")
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGINT)
                shutdown_start = time.monotonic()
                try:
                    process.wait(timeout=min(5, args.shutdown_timeout))
                except subprocess.TimeoutExpired:
                    # Observe only our own child, including any sanitizer tracer
                    # it spawned. No debugger attachment or system setting change.
                    details = []
                    root = Path(f"/proc/{process.pid}")
                    try:
                        pids = [str(process.pid)] + (root / "task" / str(process.pid) / "children").read_text().split()
                        for pid in pids:
                            for task in Path(f"/proc/{pid}/task").iterdir():
                                state = [line for line in (task / "status").read_text().splitlines()
                                         if line.startswith(("Name:", "State:", "Pid:"))]
                                details.append(" | ".join(state) + " | wait=" + (task / "wchan").read_text())
                    except OSError as error:
                        details.append(str(error))
                    (args.output / "shutdown-threads.txt").write_text("\n".join(details) + "\n")
                    try:
                        remaining = max(0, args.shutdown_timeout - (time.monotonic() - shutdown_start))
                        process.wait(timeout=remaining)
                    except subprocess.TimeoutExpired:
                        print(f"Forced stop after {args.shutdown_timeout}s shutdown allowance")
                        os.killpg(process.pid, signal.SIGKILL)
                        process.wait()
        print(f"Local smoke run exit: {process.returncode}; evidence: {args.output}")
        output = (args.output / "run.log").read_text()
        print(output)
        # A debugger can exit zero after interrupting or killing its inferior.
        # Require the application itself to attest completed SDL shutdown too.
        clean_shutdown = "Native app: SDL shutdown completed; status=0" in output
        if process.returncode == 0 and not clean_shutdown:
            print("Application cleanup was not confirmed; wrapper exit is not a pass")
        return 0 if process.returncode == 0 and clean_shutdown else 1


if __name__ == "__main__":
    raise SystemExit(main())
