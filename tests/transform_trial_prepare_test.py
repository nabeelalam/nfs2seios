# SPDX-License-Identifier: MIT
"""Public source/guard checks; actual Game replay is separate private evidence."""
from pathlib import Path
import os
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import prepare_transform_trial as trial
import prepare_ci_sources as ci
ROOT=Path(__file__).resolve().parents[1]

class TrialPreparation(unittest.TestCase):
    def test_unknown_and_duplicate_methods_rejected(self):
        for data in (b'void example(){}',b'Fn(void) Game::_sub_480330(){}',
                     b'Fn(void) Game::_sub_480330(){}\nFn(void) Game::_sub_480330(){}',
                     b'Fn(void) Game::_sub_480330()\r\n{}'):
            with self.assertRaises(ValueError):trial.transform_unit(data)

    def test_output_stays_in_project_build_storage(self):
        for path in (ROOT,ROOT/'.build',ROOT/'src',ROOT/'.build'/'..'/'outside'):
            with self.assertRaises(ValueError):trial.build_directory(path)
        self.assertEqual(trial.build_directory(ROOT/'.build/trial-safe'),ROOT/'.build/trial-safe')

    def test_existing_output_and_nested_output_rejected_before_writes(self):
        with tempfile.TemporaryDirectory()as directory:
            root=Path(directory);source=root/'.build/source';source.mkdir(parents=True)
            existing=root/'.build/existing';existing.mkdir()
            with patch.object(trial,'ROOT',root):
                for output in (existing,source/'nested',root/'.build'):
                    with self.assertRaises(ValueError):trial.prepare(source,output)
            self.assertEqual(list(source.iterdir()),[])

    def test_frozen_closure_is_complete(self):
        pins=trial.dependencies()
        self.assertEqual(len(pins),20)
        self.assertIn('src/TransformTrial.h',pins)
        self.assertIn('tests/fixtures/frame_span_cache/EngineMemory.cpp',pins)

    def test_ci_trial_configuration_gate_before_preparation(self):
        base=dict(NFS_CLASSIC_UI_REQUESTED='false',NFS_ADAPTIVE_RACE_REQUESTED='false',
                  NFS_TRANSFORM_TRIAL_REQUESTED='false',NFS_BUILD_CONFIGURATION='Debug',
                  NFS_THIN_LTO='OFF',NFS_PUBLISH_RELEASE_REQUESTED='false',NFS_HD_ARTWORK_SHA256='a'*64)
        for changes in ({'NFS_TRANSFORM_TRIAL_REQUESTED':'TRUE'},
                        {'NFS_TRANSFORM_TRIAL_REQUESTED':'true'},
                        {'NFS_TRANSFORM_TRIAL_REQUESTED':'true','NFS_CLASSIC_UI_REQUESTED':'true'},
                        {'NFS_TRANSFORM_TRIAL_REQUESTED':'true','NFS_CLASSIC_UI_REQUESTED':'true','NFS_BUILD_CONFIGURATION':'Release'},
                        {'NFS_TRANSFORM_TRIAL_REQUESTED':'true','NFS_CLASSIC_UI_REQUESTED':'true','NFS_BUILD_CONFIGURATION':'Release','NFS_THIN_LTO':'ON'}):
            with patch.dict(os.environ,{**base,**changes},clear=True),patch.object(sys,'argv',['prepare','--check-only']):
                with self.assertRaises(ValueError):ci.main()
        with patch.dict(os.environ,base,clear=True),patch.object(sys,'argv',['prepare','--check-only']):ci.main()
        valid={**base,'NFS_TRANSFORM_TRIAL_REQUESTED':'true','NFS_CLASSIC_UI_REQUESTED':'true',
               'NFS_BUILD_CONFIGURATION':'Release','NFS_THIN_LTO':'ON','NFS_ADAPTIVE_RACE_REQUESTED':'true'}
        with tempfile.TemporaryDirectory()as directory,patch.object(ci,'ROOT',Path(directory)),patch.object(ci,'approval',return_value='a'*64),patch.object(ci,'metadata_approval',return_value='b'*64),patch.dict(os.environ,valid,clear=True),patch.object(sys,'argv',['prepare','--check-only']):
            ci.main()


    def test_workflow_default_and_explicit_runtime_propagation(self):
        for name in ('ipados.yml','apple-device.yml'):
            text=(ROOT/'.github/workflows'/name).read_text()
            section=text.split('      transform_trial:',1)[1].split('      adaptive_race:',1)[0]
            self.assertIn('        default: false',section)
            self.assertIn("NFS_TRANSFORM_TRIAL_REQUESTED: ${{ inputs.transform_trial && 'true' || 'false' }}",text)
            for line in text.splitlines():
                if 'cmake -S .'in line and '-DNFS_FULL_MAPPED_ENGINE_DIR='in line:
                    self.assertIn('-DNFS_TRANSFORM_TRIAL_DIR="$NFS_TRANSFORM_TRIAL_DIR"',line)
        parent=(ROOT/'.github/workflows/ipados.yml').read_text()
        self.assertEqual(parent.count('      transform_trial: ${{ inputs.transform_trial }}'),2)
        self.assertIn('inputs.stop_slot_probe && !inputs.transform_trial',parent)
        cmake=(ROOT/'CMakeLists.txt').read_text()
        self.assertIn('Instrumented engine requires its verified transform-trial runtime',cmake)
        self.assertIn('Prepared transform trial verification failed',cmake)

if __name__=='__main__':unittest.main()
