"""Run a focused graphics test on a temporary, process-owned X server."""
import os
import select
import shutil
import subprocess
import sys
import tempfile
import time


def main():
    xvfb = shutil.which("Xvfb")
    if sys.platform != "linux" or not xvfb:
        print("SKIP: the isolated graphics test needs Linux and Xvfb")
        return 77
    if len(sys.argv) != 2:
        return 2
    read_fd, write_fd = os.pipe()
    server = None
    child = None
    try:
        with tempfile.TemporaryFile(mode="w+b") as log:
            server = subprocess.Popen(
                [xvfb, ":190", "-displayfd", str(write_fd), "-screen", "0", "128x128x24",
                 "-nolisten", "tcp", "-nolisten", "unix"],
                pass_fds=(write_fd,), stdout=log, stderr=log,
            )
            os.close(write_fd)
            write_fd = -1
            deadline = time.monotonic() + 10
            response = b""
            while b"\n" not in response and len(response) <= 16:
                if server.poll() is not None or time.monotonic() >= deadline:
                    break
                if select.select([read_fd], [], [], 0.1)[0]:
                    chunk = os.read(read_fd, 32)
                    if not chunk:
                        break
                    response += chunk
            display_number = response.strip()
            if (b"\n" not in response or not display_number.isdigit() or
                    int(display_number) < 190 or server.poll() is not None):
                print("FAIL: isolated graphics display startup")
                log.seek(0)
                print(log.read(2048).decode(errors="replace"))
                return 1
            env = dict(os.environ, DISPLAY=":" + display_number.decode("ascii"),
                       SDL_VIDEODRIVER="x11", LIBGL_ALWAYS_SOFTWARE="1",
                       MESA_GLES_VERSION_OVERRIDE="2.0", ASAN_OPTIONS="detect_leaks=0")
            child = subprocess.Popen([sys.argv[1]], env=env)
            try:
                return child.wait(timeout=45)
            except subprocess.TimeoutExpired:
                print("FAIL: focused graphics test exceeded its time limit")
                return 1
    finally:
        # Only terminate retained children created by this invocation.
        if child is not None and child.poll() is None:
            child.kill()
            child.wait()
        if server is not None and server.poll() is None:
            server.terminate()
            try:
                server.wait(timeout=3)
            except subprocess.TimeoutExpired:
                server.kill()
                server.wait()
        os.close(read_fd)
        if write_fd >= 0:
            os.close(write_fd)


if __name__ == "__main__":
    raise SystemExit(main())
