"""CMake metadata checks: configure a compiler, but never build or read game data."""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "src/hd/ClassicScreens/HistoryArtworkRelease.json"


@unittest.skipUnless(shutil.which("cmake"), "CMake is required for metadata checks")
class HistoryReleaseConfiguration(unittest.TestCase):
    def configure(self, options=()):
        workspace = ROOT / ".build"
        workspace.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="history-release-cmake-", dir=workspace) as temporary:
            folder = Path(temporary)
            self.assertTrue(folder.resolve().is_relative_to(workspace.resolve()))
            source = folder / "source"
            metadata = source / "src/hd/ClassicScreens"
            metadata.mkdir(parents=True)
            shutil.copyfile(CATALOG, metadata / CATALOG.name)
            for name in ("VictoryArtworkRelease.json", "SlideshowArtworkRelease.json"):
                shutil.copyfile(CATALOG.parent / name, metadata / name)
            shutil.copyfile(ROOT / "src/hd/ClassicScreens/HistoryPanoramaConfiguration.template.h",
                            metadata / "HistoryPanoramaConfiguration.template.h")
            shutil.copyfile(CATALOG.parent / "VictoryArtProvider.cpp",
                            metadata / "VictoryArtProvider.cpp")
            (source / "modules").mkdir()
            (source / "modules/FindZLIB.cmake").write_text(
                "set(ZLIB_FOUND TRUE)\nadd_library(ZLIB::ZLIB INTERFACE IMPORTED)\n")
            (source / "CMakeLists.txt").write_text(
                'cmake_minimum_required(VERSION 3.24)\n'
                'project(HistoryReleaseMetadata LANGUAGES CXX)\n'
                'add_library(Threads::Threads INTERFACE IMPORTED)\n'
                'set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")\n'
                'set(NFS_CLASSIC_UI ON)\n'
                'set(NFS_CLASSIC_BUNDLE_OWNED_ARTWORK ON)\n'
                'set(NFS_CLASSIC_DEFER_OWNER_TEMPLATE ON)\n'
                'set(NFS_HD_ARTWORK_SHA256 "8e83caefa2c88691b9d020d7193148f4789681e7918ab74df7cef67f5a8609b0")\n'
                'include("' + (ROOT / "cmake/BundledOwnedArtwork.cmake").as_posix() + '")\n')
            build = folder / "build"
            process = subprocess.run([shutil.which("cmake"), "-S", str(source), "-B", str(build), *options],
                                     capture_output=True, text=True, timeout=20)
            header = build / "generated/history-panorama/hd/HistoryPanoramaConfiguration.h"
            text = header.read_text() if header.is_file() else ""
            files = [path.name for path in build.rglob("*.ownedz")] if build.is_dir() else []
            return process, text, files

    def test_fresh_deferred_base_embeds_all_pins_without_private_bytes(self):
        process, text, files = self.configure()
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        rows = json.loads(CATALOG.read_text())["panoramas"]
        for row in rows:
            self.assertIn(row["filename"], text)
            pin = row["sha256"]
            self.assertIn(",".join("0x" + pin[i:i+2] for i in range(0, 64, 2)), text)
        self.assertFalse(files)

    def test_explicit_empty_pin_keeps_developer_opt_out(self):
        process, text, files = self.configure(["-DNFS_CLASSIC_HISTORY_PANORAMA_0_SHA256:STRING="])
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        self.assertNotIn('"history-panorama-0.ownedz"', text)
        self.assertIn('"history-panorama-42.ownedz"', text)
        self.assertFalse(files)

    def test_invalid_explicit_pin_fails_before_build(self):
        process, _, files = self.configure(["-DNFS_CLASSIC_HISTORY_PANORAMA_0_SHA256:STRING=" + "0" * 64])
        self.assertNotEqual(process.returncode, 0)
        self.assertIn("nonzero", process.stdout + process.stderr)
        self.assertFalse(files)


if __name__ == "__main__":
    unittest.main()
