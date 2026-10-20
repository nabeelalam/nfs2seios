"""Keep registered UI tests in the explicit free-runner build/run lists."""
from pathlib import Path
import re
import unittest

ROOT=Path(__file__).resolve().parents[1]


class HdWorkflowCoverage(unittest.TestCase):
    def setUp(self):
        cmake=(ROOT/'CMakeLists.txt').read_text()
        families=re.search(r'foreach\(target (texture_digest_tests[^)]*)\)',cmake)
        self.assertIsNotNone(families)
        self.required=set(families[1].split())|{'hd_atlas_tests','glide_tests'}
        self.required.update(re.findall(r'add_executable\((frontend_\w+)',cmake))

    def test_all_ui_targets_built_before_apple_ctest(self):
        workflow=(ROOT/'.github/workflows/apple-device.yml').read_text()
        build=re.search(r'cmake --build \.build/host --target ([^\n]+) -j ',workflow)
        self.assertIsNotNone(build)
        self.assertFalse(self.required-set(build[1].split()))
        self.assertIn('ctest --test-dir .build/host --output-on-failure',workflow)

    def test_linux_gate_builds_and_executes_ui_checks(self):
        workflow=(ROOT/'.github/workflows/ipados.yml').read_text()
        build=re.search(r'cmake --build \.build/pixels --target ([^\n]+) -j ',workflow)
        self.assertIsNotNone(build)
        self.assertFalse(self.required-set(build[1].split()))
        executed=set(re.findall(r'\.build/pixels/(\w+_tests)\b',workflow))
        for group in re.findall(r'for test in ([^;\n]+); do',workflow):
            executed.update(group.split())
        self.assertFalse(self.required-executed)

    def test_fast_preparation_and_policy_checks_are_scheduled(self):
        for name in ('ipados.yml','apple-device.yml'):
            workflow=(ROOT/'.github/workflows'/name).read_text()
            for test in ('frontend_word_hooks','frontend_asset_review','frontend_font_inspection','hd_workflow_coverage'):
                with self.subTest(workflow=name,test=test):
                    self.assertIn('python3 tests/'+test+'_test.py',workflow)


if __name__=='__main__':unittest.main()
