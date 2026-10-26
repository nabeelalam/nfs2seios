"""Synthetic-only owner artwork reconstruction and filesystem safety checks."""
# SPDX-License-Identifier: MIT
import copy
from dataclasses import FrozenInstanceError
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import assemble_hd_artwork as art


def literal_qfs(data):
    stream, at = bytearray(b"\x10\xfb" + len(data).to_bytes(3, "big")), 0
    while len(data) - at >= 4:
        count = min(112, (len(data) - at) // 4 * 4)
        stream.append(0xe0 | (count // 4 - 1))
        stream.extend(data[at:at + count])
        at += count
    stream.append(0xfc | (len(data) - at))
    stream.extend(data[at:])
    return bytes(stream)


def fixture():
    count, table_bytes = 24, 16 + 24 * 8
    archive = bytearray(struct.pack("<4sII4s", b"SHPI", table_bytes + count * 20,
                                    count, b"TEST"))
    names = ["T%03d" % i for i in range(count)]
    for i, name in enumerate(names):
        archive.extend(struct.pack("<4sI", name.encode(), table_bytes + i * 20))
    for i in range(count):
        archive.extend(b"TEST" + struct.pack("<III", i, i + 1000, i + 2000)
                       + b"FAKE")
    archive = bytes(archive)
    owner = literal_qfs(archive)
    output = bytearray(b"SYNTHETIC-ONLY!!!")
    holes = []
    for name, (offset, span) in art.inspect_fsh(archive).items():
        holes.append(dict(outputOffset=len(output), bytes=16, entry=name,
                          sourceOffset=offset, span=span,
                          entrySha256=art.sha256(archive[offset:offset + span])))
        output.extend(archive[offset:offset + 16])
        output.extend(b"ART!")
    output.extend(b"SYNTHETIC-TAIL")
    payload, cursor = bytearray(), 0
    for hole in holes:
        payload.extend(output[cursor:hole["outputOffset"]])
        cursor = hole["outputOffset"] + 16
    payload.extend(output[cursor:])
    metadata = dict(schema=1, outputBytes=len(output), outputSha256=art.sha256(output),
                    ownerFile=art.OWNER_FILE, ownerSha256=art.sha256(owner),
                    decodedSha256=art.sha256(archive), holes=holes,
                    payloadSha256=art.sha256(payload))
    return owner, archive, bytes(output), metadata, bytes(payload)


def encoded(metadata, payload):
    body = json.dumps(metadata, sort_keys=True, separators=(",", ":")).encode()
    return art.MAGIC + struct.pack("<Q", len(body)) + body + payload


class OwnerArtwork(unittest.TestCase):
    def setUp(self):
        self.owner, self.archive, self.output, self.metadata, self.payload = fixture()
        self.template = encoded(self.metadata, self.payload)
        self.recipe = art.TrustedRecipe(art.sha256(self.template), art.sha256(self.output))
        self.temporary = tempfile.TemporaryDirectory(prefix="owner-art-synthetic-")
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name).resolve()
        self.game = self.base / "owner"
        self.asset = self.game / "FeData" / "Pc" / "ART" / "MAIN.QFS"
        self.asset.parent.mkdir(parents=True)
        self.asset.write_bytes(self.owner)
        self.path = self.base / "art.ownerart"
        self.path.write_bytes(self.template)

    def parse_changed(self, metadata, payload=None):
        value = encoded(metadata, self.payload if payload is None else payload)
        recipe = art.TrustedRecipe(art.sha256(value), art.sha256(self.output))
        return art.parse_template(value, recipe)

    def test_exact_reconstruction_and_omitted_headers(self):
        self.assertEqual(art.reconstruct(self.template, self.owner, self.recipe), self.output)
        self.assertEqual(art.decode_qfs(self.owner), self.archive)
        self.assertFalse(self.template.startswith(b"NFSHDPK"))
        self.assertEqual(len(self.output) - len(self.payload), 384)
        for hole in self.metadata["holes"]:
            header = self.archive[hole["sourceOffset"]:hole["sourceOffset"] + 16]
            self.assertNotIn(header, self.template)
        with self.assertRaises(FrozenInstanceError):
            self.recipe.output_sha256 = "0" * 64

    def test_case_insensitive_owner_and_atomic_new_output(self):
        before = self.asset.read_bytes()
        result = art.assemble(self.path, self.game, self.base / "result.nfshd", self.recipe)
        self.assertTrue(result["ownerUnchanged"])
        self.assertEqual((self.base / "result.nfshd").read_bytes(), self.output)
        self.assertEqual(self.asset.read_bytes(), before)
        with self.assertRaises(ValueError):
            art.assemble(self.path, self.game, self.base / "result.nfshd", self.recipe)
        self.assertEqual((self.base / "result.nfshd").read_bytes(), self.output)
        self.assertFalse(list(self.base.glob(".owner-art-*.tmp")))

    def test_missing_wrong_mutated_and_ambiguous_owner(self):
        self.asset.unlink()
        with self.assertRaises(ValueError):
            art.assemble(self.path, self.game, self.base / "absent", self.recipe)
        for data in (b"wrong asset", self.owner[:-1] + bytes([self.owner[-1] ^ 1])):
            self.asset.write_bytes(data)
            with self.assertRaises(ValueError):
                art.assemble(self.path, self.game, self.base / "absent", self.recipe)
            self.assertFalse((self.base / "absent").exists())
        self.asset.write_bytes(self.owner)
        if os.name != "nt":
            (self.game / "fedata").mkdir()
            with self.assertRaises(ValueError):
                art.resolve_owner_asset(self.game)

    def test_bounded_directory_inventory(self):
        with patch.object(art, "MAX_DIRECTORY_ENTRIES", 0):
            with self.assertRaises(ValueError):
                art.resolve_owner_asset(self.game)

    def test_symlinks_and_output_inside_owner_are_rejected(self):
        with self.assertRaises(ValueError):
            art.assemble(self.path, self.game, self.game / "forbidden", self.recipe)
        if os.name == "nt":
            return  # The same cases run on Unix without Windows symlink privilege.
        source_link = self.base / "source-link"
        source_link.symlink_to(self.game, target_is_directory=True)
        with self.assertRaises(ValueError):
            art.resolve_owner_asset(source_link)
        self.asset.unlink()
        self.asset.symlink_to(self.path)
        with self.assertRaises(ValueError):
            art.resolve_owner_asset(self.game)
        self.asset.unlink()
        self.asset.write_bytes(self.owner)
        template_link = self.base / "template-link"
        template_link.symlink_to(self.path)
        with self.assertRaises(ValueError):
            art.assemble(template_link, self.game, self.base / "absent", self.recipe)
        out_link = self.base / "output-link"
        out_link.symlink_to(self.game, target_is_directory=True)
        with self.assertRaises(ValueError):
            art.assemble(self.path, self.game, out_link / "absent", self.recipe)
        self.assertFalse((self.game / "absent").exists())

    def test_no_partial_output_on_stage_or_publish_failure(self):
        target = self.base / "absent"
        with patch.object(art.os, "link", side_effect=OSError("injected publish refusal")):
            with self.assertRaises(OSError):
                art.assemble(self.path, self.game, target, self.recipe)
        self.assertFalse(target.exists())
        self.assertFalse(list(self.base.glob(".owner-art-*.tmp")))
        real_read = art.read_regular
        def corrupt_stage(path, limit):
            if Path(path).name.startswith(".owner-art-"):
                return b"corrupt staged output"
            return real_read(path, limit)
        with patch.object(art, "read_regular", side_effect=corrupt_stage):
            with self.assertRaises(ValueError):
                art.assemble(self.path, self.game, target, self.recipe)
        self.assertFalse(target.exists())
        self.assertFalse(list(self.base.glob(".owner-art-*.tmp")))

    def test_untrusted_template_and_owner_identity(self):
        for value in (self.template[:-1], self.template[:-1] + b"X"):
            with self.assertRaises(ValueError):
                art.reconstruct(value, self.owner, self.recipe)
        for field in ("decodedSha256", "ownerSha256"):
            metadata = copy.deepcopy(self.metadata)
            metadata[field] = "0" * 64
            value = encoded(metadata, self.payload)
            recipe = art.TrustedRecipe(art.sha256(value), art.sha256(self.output))
            with self.assertRaises(ValueError):
                art.reconstruct(value, self.owner, recipe)
        for field, value in (("sourceOffset", 0), ("entrySha256", "0" * 64)):
            metadata = copy.deepcopy(self.metadata)
            metadata["holes"][0][field] = value
            blob = encoded(metadata, self.payload)
            recipe = art.TrustedRecipe(art.sha256(blob), art.sha256(self.output))
            with self.assertRaises(ValueError):
                art.reconstruct(blob, self.owner, recipe)
        with self.assertRaises(ValueError):
            art.parse_template(self.template, None)

    def test_json_types_bounds_and_complete_inventory(self):
        mutations = [("schema", True), ("outputBytes", True), ("outputBytes", -1),
                     ("outputBytes", art.MAX_TEMPLATE + 1), ("ownerSha256", 123),
                     ("ownerFile", "../../elsewhere"), ("holes", {}),
                     ("holes", self.metadata["holes"][:-1])]
        for field, value in mutations:
            metadata = copy.deepcopy(self.metadata)
            metadata[field] = value
            with self.subTest(field=field, value=str(value)[:40]), self.assertRaises(ValueError):
                self.parse_changed(metadata)
        for field, value in (("outputOffset", -1), ("outputOffset", 99999),
                             ("outputOffset", True), ("bytes", 15), ("span", 0),
                             ("sourceOffset", art.MAX_ORIGINAL), ("entry", "../x"),
                             ("entrySha256", [0] * 32)):
            metadata = copy.deepcopy(self.metadata)
            metadata["holes"][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.parse_changed(metadata)
        for kind in ("overlap", "duplicate", "unknown"):
            metadata = copy.deepcopy(self.metadata)
            if kind == "overlap":
                metadata["holes"][1]["outputOffset"] = metadata["holes"][0]["outputOffset"]
            elif kind == "duplicate":
                metadata["holes"][1]["entry"] = metadata["holes"][0]["entry"]
            else:
                metadata["holes"][0]["unexpected"] = 1
            with self.assertRaises(ValueError):
                self.parse_changed(metadata)
        with self.assertRaises(ValueError):
            self.parse_changed(self.metadata, self.payload + b"x")
        raw = b'{"schema":1,"schema":1}'
        blob = art.MAGIC + struct.pack("<Q", len(raw)) + raw + b"x"
        with self.assertRaises(ValueError):
            art.parse_template(blob, art.TrustedRecipe(art.sha256(blob), art.sha256(self.output)))

    def test_qfs_copy_forms_and_reject_malformed_streams(self):
        for command, expected in ((b"\x00\x03", b"abcdefghijklmnopmno"),
                                  (b"\x80\x00\x03", b"abcdefghijklmnopmnop"),
                                  (b"\xc0\x00\x03\x00", b"abcdefghijklmnopmnopm")):
            stream = b"\x10\xfb" + len(expected).to_bytes(3, "big") + b"\xe3abcdefghijklmnop" + command + b"\xfc"
            self.assertEqual(art.decode_qfs(stream), expected)
        for data in (b"bad!!", b"\x10\xfb\xff\xff\xff\xfc",
                     b"\x10\xfb\x00\x00\x10\x80", b"\x10\xfb\x00\x00\x10\x00\x00",
                     self.owner[:-1], self.owner + b"trailer"):
            with self.assertRaises(ValueError):
                art.decode_qfs(data)
        for offset, value in ((4, 1), (8, 4097), (20, 0), (28, 208)):
            data = bytearray(self.archive)
            struct.pack_into("<I", data, offset, value)
            with self.assertRaises(ValueError):
                art.inspect_fsh(bytes(data))
        data = bytearray(self.archive)
        data[24:28] = data[16:20]
        with self.assertRaises(ValueError):
            art.inspect_fsh(bytes(data))

    def test_trusted_budget_defaults_bounds_and_immutability(self):
        self.assertEqual(self.recipe.max_template_bytes, 96 * 1024 * 1024)
        self.assertEqual(art.KNOWN_RELEASE.max_template_bytes, art.MAX_TEMPLATE)
        large = art.TrustedRecipe(self.recipe.template_sha256, self.recipe.output_sha256,
                                  104 * 1024 * 1024)
        self.assertEqual(large.max_template_bytes, art.MAX_TRUSTED_TEMPLATE)
        for value in (True, False, None, 104.0, "104", 0, 15,
                      art.MAX_TRUSTED_TEMPLATE + 1):
            with self.subTest(value=value), self.assertRaises(ValueError):
                art.TrustedRecipe(self.recipe.template_sha256,
                                  self.recipe.output_sha256, value)
        with self.assertRaises(FrozenInstanceError):
            large.max_template_bytes = art.MAX_TRUSTED_TEMPLATE + 1

    def test_exact_trusted_budget_and_one_byte_under(self):
        exact = art.TrustedRecipe(self.recipe.template_sha256, self.recipe.output_sha256,
                                  len(self.template))
        self.assertEqual(art.reconstruct(self.template, self.owner, exact), self.output)
        small = art.TrustedRecipe(self.recipe.template_sha256, self.recipe.output_sha256,
                                  len(self.template) - 1)
        with self.assertRaisesRegex(ValueError, "Template bounds"):
            art.parse_template(self.template, small)
        with self.assertRaisesRegex(ValueError, "Input file bounds"):
            art.assemble(self.path, self.game, self.base / "too-small", small)
        self.assertFalse((self.base / "too-small").exists())
        metadata = copy.deepcopy(self.metadata)
        metadata["outputBytes"] = exact.max_template_bytes + 1
        for _ in range(3):
            blob = encoded(metadata, self.payload)
            metadata["outputBytes"] = len(blob) + 1
        blob = encoded(metadata, self.payload)
        self.assertEqual(metadata["outputBytes"], len(blob) + 1)
        trusted = art.TrustedRecipe(art.sha256(blob), self.recipe.output_sha256,
                                    len(blob))
        with self.assertRaisesRegex(ValueError, "Template output bounds"):
            art.parse_template(blob, trusted)

    def test_budget_applies_to_template_and_staged_output_reads(self):
        recipe = art.TrustedRecipe(self.recipe.template_sha256, self.recipe.output_sha256,
                                   len(self.template))
        original = art.read_regular
        calls = []
        def tracked(path, limit):
            calls.append((Path(path), limit))
            return original(path, limit)
        with patch.object(art, "read_regular", side_effect=tracked):
            result = art.assemble(self.path, self.game, self.base / "bounded", recipe)
        self.assertEqual(result["bytes"], len(self.output))
        self.assertIn((self.path, recipe.max_template_bytes), calls)
        staged = [limit for path, limit in calls if path.name.startswith(".owner-art-")]
        self.assertEqual(staged, [recipe.max_template_bytes])
        self.assertEqual(self.asset.read_bytes(), self.owner)
        metadata = copy.deepcopy(self.metadata)
        metadata["maxTemplateBytes"] = art.MAX_TRUSTED_TEMPLATE
        with self.assertRaisesRegex(ValueError, "Template fields"):
            self.parse_changed(metadata)

    def test_cli_cannot_raise_trusted_budget(self):
        command = [sys.executable, str(ROOT / "scripts/assemble_hd_artwork.py"),
                   "--template", str(self.path), "--owner-root", str(self.game),
                   "--output", str(self.base / "untrusted-limit"),
                   "--max-template-bytes", str(art.MAX_TRUSTED_TEMPLATE)]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.base / "untrusted-limit").exists())

    def test_cli_has_no_untrusted_recipe_or_hash_override(self):
        command = [sys.executable, str(ROOT / "scripts/assemble_hd_artwork.py"),
                   "--template", str(self.path), "--owner-root", str(self.game),
                   "--output", str(self.base / "absent")]
        refused = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(refused.returncode, 0)
        self.assertFalse((self.base / "absent").exists())
        refused = subprocess.run(command + ["--template-sha256", self.recipe.template_sha256],
                                 capture_output=True, text=True)
        self.assertEqual(refused.returncode, 2)
        self.assertNotIn(str(self.asset), refused.stdout)


if __name__ == "__main__":
    unittest.main()
