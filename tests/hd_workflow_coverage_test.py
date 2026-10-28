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
        # This native GL-fault verifier is optional NFS_HD_CPU_TESTS coverage,
        # not a target in the default host/pixel configurations used by CI.
        self.assertRegex(cmake, r'option\(NFS_HD_CPU_TESTS [^\n]+ OFF\)')
        optional = re.search(r'if\(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND NFS_HD_CPU_TESTS\)(?:(?!endif\(\)).)*?add_executable\(frontend_font_upload_tests',cmake,re.S)
        self.assertIsNotNone(optional)
        self.required.discard('frontend_font_upload_tests')
        self.assertIn('add_test(NAME hd_frontend_font_upload_source COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/frontend_font_upload_source_test.py")',cmake)


    def test_all_ui_targets_built_in_linux_host_gate(self):
        workflow=(ROOT/'.github/workflows/ipados.yml').read_text()
        build=re.search(r'cmake --build \.build/host --target ([^\n]+) -j ',workflow)
        self.assertIsNotNone(build)
        self.assertFalse(self.required-set(build[1].split()))
        self.assertIn('ctest --test-dir .build/host --output-on-failure',workflow)
        # The source check runs explicitly even when its optional CTest block
        # is disabled; it is not misrepresented as a compiled executable.
        self.assertIn('python3 tests/frontend_font_upload_source_test.py',workflow)
        for build in re.findall(r'cmake --build \.build/(?:host|pixels) --target ([^\n]+)',workflow):
            self.assertNotIn('frontend_font_upload_tests',build.split())


    def test_linux_gate_builds_and_executes_ui_checks(self):
        workflow=(ROOT/'.github/workflows/ipados.yml').read_text()
        build=re.search(r'cmake --build \.build/pixels --target ([^\n]+) -j ',workflow)
        self.assertIsNotNone(build)
        self.assertFalse(self.required-set(build[1].split()))
        executed=set(re.findall(r'\.build/pixels/(\w+_tests)\b',workflow))
        for group in re.findall(r'for test in ([^;\n]+); do',workflow):
            executed.update(group.split())
        self.assertFalse(self.required-executed)

    def test_linux_pointer_adapter_is_built_and_executed(self):
        workflow=(ROOT/'.github/workflows/ipados.yml').read_text()
        build=re.search(r'cmake --build \.build/pixels --target ([^\n]+) -j ',workflow)
        self.assertIsNotNone(build)
        self.assertIn('desktop_pointer_tests',build[1].split())
        self.assertRegex(workflow,r'timeout 15s \.build/pixels/desktop_pointer_tests\b')

    def test_fast_preparation_and_policy_checks_are_scheduled(self):
        for name in ('ipados.yml',):
            workflow=(ROOT/'.github/workflows'/name).read_text()
            for test in ('frontend_word_hooks','frontend_asset_review','frontend_font_inspection','hd_workflow_coverage','assemble_hd_artwork'):
                with self.subTest(workflow=name,test=test):
                    self.assertIn('python3 tests/'+test+'_test.py',workflow)


    def test_apple_uses_successful_linux_gate_and_only_apple_host_checks(self):
        parent=(ROOT/'.github/workflows/ipados.yml').read_text()
        apple=(ROOT/'.github/workflows/apple-device.yml').read_text()
        self.assertEqual(parent.count("needs: renderer-pixels"),2)
        self.assertEqual(parent.count("linux_gate_passed: ${{ needs.renderer-pixels.result == 'success' }}"),2)
        self.assertIn("if: inputs.linux_gate_passed &&",apple)
        self.assertIn("--target apple_main_thread_tests system_cpu_provider_tests -j",apple)
        self.assertIn("-R '^(actual_apple_main_queue|system_cpu_native_provider)$'",apple)
        self.assertNotIn("cmake --build .build/host",apple)


if __name__=='__main__':unittest.main()
