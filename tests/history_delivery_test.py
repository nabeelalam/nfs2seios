"""Synthetic matched-History delivery checks; no runnable app or original assets."""
from contextlib import contextmanager
import hashlib
import importlib.util
import json
from pathlib import Path
import plistlib
import re
import struct
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("history_delivery", ROOT / "scripts/inject_personal_artwork.py")
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
digest = lambda data: hashlib.sha256(data).hexdigest()


@contextmanager
def fixture(omit=(), notice_pins=()):
    with tempfile.TemporaryDirectory(prefix="history-delivery-") as temporary:
        root = Path(temporary)
        template = b"OWNZ0001" + bytes(24)
        history = b"NFSHIST1" + bytes(range(32))
        panoramas = {"history-panorama-" + str(i) + ".ownedz": b"NFSHWID2" + bytes([i]) * 48
                     for i in range(0, 49, 7)}
        pins = {name: (len(data), digest(data)) for name, data in panoramas.items()}
        release = "42" * 32
        hashes = {m.HISTORY_RESOURCE: digest(history), **{name: pin[1] for name, pin in pins.items()}}
        native = struct.pack("<IiiIIIII6I", 0xfeedfacf, 0x0100000c, 0, 2, 1, 24, 0, 0,
                             0x32, 24, 2, 0, 0, 0) + bytes.fromhex(release)
        for name, value in hashes.items():
            if name not in omit:
                native += value.encode() if name == m.HISTORY_RESOURCE else bytes.fromhex(value)
        native += b"unchanged-native"
        members = {"Payload/Test.app/Info.plist": plistlib.dumps({"CFBundleExecutable": "Test"}),
                   "Payload/Test.app/Test": native,
                   "Payload/Test.app/notice.txt": b"unchanged notice" + b"".join(hashes[name].encode() for name in notice_pins)}
        base = root / "base.ipa"
        with zipfile.ZipFile(base, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            for name, value in members.items():
                archive.writestr(name, value)
        (root / "template").write_bytes(template)
        (root / "history").write_bytes(history)
        art = root / "panoramas"
        art.mkdir()
        for name, data in panoramas.items():
            (art / name).write_bytes(data)
        with patch.multiple(m, TEMPLATE_BYTES=len(template), TEMPLATE_SHA256=digest(template),
                            OUTPUT_PACK_SHA256=release, HISTORY_BYTES=len(history),
                            HISTORY_SHA256=digest(history), HISTORY_PANORAMA_PINS=pins):
            yield root, base, members, history, panoramas


def inject(root, base, output="out.ipa"):
    return m.inject(base, digest(base.read_bytes()), root / "template", root / output,
                    history=root / "history", history_panoramas_dir=root / "panoramas")


class HistoryDelivery(unittest.TestCase):
    def test_release_catalog_matches_loader(self):
        catalog = json.loads((ROOT / "src/hd/ClassicScreens/HistoryArtworkRelease.json").read_text())
        header = (ROOT / "src/hd/ClassicScreens/HistoryArtworkPin.h").read_text()
        for macro, value in (("SHA256", catalog["history"]["sha256"]),
                             ("FILE_BYTES", str(catalog["history"]["bytes"])),
                             ("PAYLOAD_BYTES", str(catalog["history"]["payloadBytes"]))):
            defined = re.search(r"^#define NFS_HISTORY_ARTWORK_" + macro + r" (.+)$",
                                header, re.MULTILINE)[1]
            self.assertEqual(defined, '"' + value + '"' if macro == "SHA256" else value + "u")
        self.assertEqual(len(m.resource_pins()), 22 + bool(m._SLIDESHOW_PANORAMA_RELEASE))
        self.assertEqual([row["sourceIndex"] for row in catalog["panoramas"]], list(range(0, 49, 7)))

    def test_pin_matching_across_chunks_and_encodings(self):
        hashes = {"raw": "ab" * 32, "ascii": "cd" * 32, "upper": "ef" * 32}
        matcher = m.ReleasePinMatcher(hashes)
        stream = bytes.fromhex(hashes["raw"]) + hashes["ascii"].encode() + hashes["upper"].upper().encode()
        for start in range(0, len(stream), 7):
            matcher.feed(stream[start:start + 7])
        self.assertFalse(matcher.missing)
        absent = m.ReleasePinMatcher({"missing": "01" * 32})
        absent.feed(stream)
        self.assertEqual(absent.missing, {"missing"})

    def test_matched_injection_preserves_original_members_and_storage(self):
        with fixture() as (root, base, members, history, panoramas):
            before = base.read_bytes()
            result = inject(root, base)
            self.assertEqual(result["addedMembers"], 9)
            self.assertTrue(result["historyReleasePinsVerified"])
            self.assertTrue(result["allExistingMemberBytesIdentical"])
            self.assertTrue(result["allExistingCompressedStorageIdentical"])
            self.assertEqual(base.read_bytes(), before)
            self.assertEqual((root / "history").read_bytes(), history)
            with zipfile.ZipFile(root / "out.ipa") as archive:
                for name, value in members.items():
                    self.assertEqual(archive.read(name), value)
                for name, value in panoramas.items():
                    self.assertEqual(archive.read("Payload/Test.app/" + name), value)
                self.assertEqual(archive.read("Payload/Test.app/history-ui.ownedz"), history)

    def test_older_base_missing_one_panorama_is_rejected(self):
        name = "history-panorama-42.ownedz"
        with fixture(omit=(name,)) as (root, base, *_):
            with self.assertRaisesRegex(ValueError, "artwork release pin"):
                inject(root, base)
            self.assertFalse((root / "out.ipa").exists())

    def test_pin_in_other_member_does_not_qualify_native_code(self):
        with fixture(omit=(m.HISTORY_RESOURCE,), notice_pins=(m.HISTORY_RESOURCE,)) as (root, base, *_):
            with self.assertRaisesRegex(ValueError, "artwork release pin"):
                inject(root, base)
            self.assertFalse((root / "out.ipa").exists())

    def test_incomplete_pair_is_rejected_before_inputs_are_opened(self):
        for options in ({"history": "history"}, {"history_panoramas_dir": "panoramas"}):
            with self.assertRaisesRegex(ValueError, "supplied together"):
                m.inject("absent", "01" * 32, "absent-template", "absent-output", **options)

    def test_wrong_or_missing_panorama_cannot_publish_output(self):
        for missing in (False, True):
            with fixture() as (root, base, _, _, panoramas):
                name = next(iter(panoramas))
                path = root / "panoramas" / name
                if missing:
                    path.unlink()
                else:
                    path.write_bytes(panoramas[name][:-1] + b"X")
                before = base.read_bytes()
                with self.assertRaises((ValueError, OSError)):
                    inject(root, base)
                self.assertFalse((root / "out.ipa").exists())
                self.assertEqual(base.read_bytes(), before)

    def test_existing_output_is_never_overwritten(self):
        with fixture() as (root, base, *_):
            output = root / "out.ipa"
            output.write_bytes(b"preserved")
            with self.assertRaisesRegex(ValueError, "new file"):
                inject(root, base)
            self.assertEqual(output.read_bytes(), b"preserved")


if __name__ == "__main__":
    unittest.main()
