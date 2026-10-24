"""Verify the maintained preparation ENTRY installs the child Controls pipeline.

No private generated source is read. AST admission links real entry wiring to
public synthetic hook composition; it does not claim original-game execution.
"""
# SPDX-License-Identifier: MIT
import ast
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'scripts'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from controls_setup_text_hooks import add_controls_setup_text_hooks, remove_controls_setup_text_hooks
from controls_setup_sidebar_hooks import add_controls_setup_sidebar_hooks, remove_controls_setup_sidebar_hooks
from controls_setup_live_hooks import add_controls_setup_live_hooks, remove_controls_setup_live_hooks
from controls_setup_scene_hooks import add_controls_setup_scene_hooks, remove_controls_setup_scene_hooks
from controls_setup_live_hooks_test import fixtures as live_fixtures
from controls_setup_scene_hooks_test import composed as parent_scene_fixture
from controls_text_hooks_test import HEADER, section, call

STEPS = (
    ('controls_setup_text_hooks', 'add_controls_setup_text_hooks'),
    ('controls_setup_sidebar_hooks', 'add_controls_setup_sidebar_hooks'),
    ('controls_setup_live_hooks', 'add_controls_setup_live_hooks'),
    ('controls_setup_scene_hooks', 'add_controls_setup_scene_hooks'),
)
ADDITIONS = dict((name, globals()[name]) for _, name in STEPS)
REMOVALS = (remove_controls_setup_scene_hooks, remove_controls_setup_live_hooks,
            remove_controls_setup_sidebar_hooks, remove_controls_setup_text_hooks)
UNITS = ('Methods_01.cpp', 'Methods_03.cpp', 'Methods_09.cpp',
         'Methods_10.cpp', 'Methods_13.cpp', 'Methods_14.cpp')


def normal_steps(source):
    """Admit direct real-loop wiring; never import/execute preparation itself."""
    tree = ast.parse(source)
    for module, name in STEPS:
        imports = [(node.module, item.name, item.asname)
                   for node in tree.body if isinstance(node, ast.ImportFrom)
                   for item in node.names if (item.asname or item.name) == name]
        if imports != [(module, name, None)]:
            raise ValueError('missing/ambiguous/wrong child hook import: '+name)
    functions = [n for n in tree.body if isinstance(n, ast.FunctionDef)
                 and n.name == 'prepare_full']
    if len(functions) != 1:
        raise ValueError('maintained prepare_full entry missing/ambiguous')
    fn = functions[0]
    loops = [n for n in fn.body if isinstance(n, ast.For)
             and isinstance(n.target, ast.Name) and n.target.id == 'path'
             and any(isinstance(c, ast.Call) and isinstance(c.func, ast.Attribute)
                     and c.func.attr == 'glob'
                     and c.args and isinstance(c.args[0], ast.Constant)
                     and c.args[0].value == 'Methods_*.cpp'
                     for c in ast.walk(n.iter))]
    if len(loops) != 1:
        raise ValueError('real generated Methods loop missing/ambiguous')
    loop = loops[0]
    names = [name for _, name in STEPS]
    positions = []
    for name in names:
        calls = [n for n in ast.walk(fn) if isinstance(n, ast.Call)
                 and isinstance(n.func, ast.Name) and n.func.id == name]
        direct = [(i, n) for i, n in enumerate(loop.body)
                  if isinstance(n, ast.Assign) and len(n.targets) == 1
                  and isinstance(n.targets[0], ast.Name) and n.targets[0].id == 'text'
                  and isinstance(n.value, ast.Call)
                  and isinstance(n.value.func, ast.Name) and n.value.func.id == name]
        if len(calls) != 1 or len(direct) != 1 or direct[0][1].value is not calls[0]:
            raise ValueError('child hook must run once unconditionally on the emitted text: '+name)
        call = calls[0]
        if call.keywords or len(call.args) != 2 or ast.dump(call.args[0]) != ast.dump(ast.Name(id='text', ctx=ast.Load())) or ast.dump(call.args[1]) != ast.dump(ast.Attribute(value=ast.Name(id='path', ctx=ast.Load()), attr='name', ctx=ast.Load())):
            raise ValueError('child hook must receive current text and actual unit name: '+name)
        positions.append(direct[0][0])
    if positions != list(range(positions[0], positions[0]+len(STEPS))):
        raise ValueError('required child Text -> Sidebar -> Live -> Scene order changed')
    # Parent dependencies must already have modified this same text value.
    for name in ('add_controls_text_hooks', 'add_controls_sidebar_hooks',
                 'add_controls_live_hooks', 'add_controls_scene_hooks'):
        parent = [i for i, n in enumerate(loop.body)
                  if isinstance(n, ast.Assign) and len(n.targets) == 1
                  and isinstance(n.targets[0], ast.Name) and n.targets[0].id == 'text'
                  and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name)
                  and n.value.func.id == name]
        if len(parent) != 1 or parent[0] >= positions[0]:
            raise ValueError('parent dependency must precede child pipeline: '+name)
    after = loop.body[positions[-1]+1:]
    # Require the chain output itself to reach the maintained unit write. A
    # dead helper call or overwrite/reset before the write must not pass.
    if not after or not isinstance(after[0], ast.Expr):
        raise ValueError('child pipeline output is not written directly')
    call = after[0].value
    if not isinstance(call, ast.Call) or not isinstance(call.func, ast.Attribute) or call.func.attr != 'write_text' or call.keywords or len(call.args) != 1 or not isinstance(call.args[0], ast.Name) or call.args[0].id != 'text':
        raise ValueError('child pipeline output is not the emitted unit text')
    expected = ast.BinOp(left=ast.Name(id='dest', ctx=ast.Load()), op=ast.Div(), right=ast.Attribute(value=ast.Name(id='path', ctx=ast.Load()), attr='name', ctx=ast.Load()))
    if ast.dump(call.func.value) != ast.dump(expected):
        raise ValueError('child pipeline writes a different destination/unit')
    return tuple(names)


def synthetic_parent_sources():
    # This fixture already contains the independently tested original-register
    # reset/copy/tail spellings. Remove only child families to obtain the normal
    # pipeline input, preserving all existing parent text/sidebar observations.
    bases = {}
    for name, source in live_fixtures().items():
        source = remove_controls_setup_sidebar_hooks(source)
        source = remove_controls_setup_text_hooks(source)
        if name == 'Methods_01.cpp':
            # The older live fixture spells both stack reads as to32i. The
            # maintained checked-copy output uses a transaction load BEFORE
            # reset and exactly one to32i form argument AFTER the sidebar copy.
            # Model that actual source distinction; do not loosen hook anchors.
            part = section(source, '403FC0')
            legacy = (call('428A20')+'\tedx = 0x400; //mov\n'
                      '\teax = to32i(esp+0x2C); //mov\n'
                      '\teax = to32i(eax+4); //mov\n'
                      '\tecx = edi; //mov\n\tebx = edi; //mov\n')
            checked = (call('428A20')+'\t{ // checked copy transaction begin\n'
                       '\tnfs::GuestCopyTransaction guest_transaction;\n'
                       '\teax = guest_transaction.load<int32_t>(esp+0x2C); //mov\n'
                       '\tedx = 0x400; //mov\n'
                       '\tecx = edi; //mov\n\tebx = edi; //mov\n'
                       '\teax = guest_transaction.load<int32_t>(eax+4); //mov\n'
                       '\t} // checked copy transaction end\n')
            if part.count(legacy) != 1:
                raise ValueError('public prompt reset fixture changed')
            updated = part.replace(legacy, checked, 1)
            if updated.count('\teax = to32i(esp+0x2C); //mov\n') != 1:
                raise ValueError('public prompt requires its unique post-copy form argument')
            source = source.replace(part, updated, 1)
        bases[name] = source
    # Methods03 also contains the separately tested native original-pass loop,
    # carrying Audio/Graphics/parent Controls scene hooks with their real order.
    scene = parent_scene_fixture()
    if not scene.startswith(HEADER):
        raise ValueError('public scene fixture header changed')
    bases['Methods_03.cpp'] += scene[len(HEADER):]
    return bases


class ControlsSetupPipeline(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entry = (ROOT/'scripts'/'prepare_full_mapped.py').read_text(encoding='utf-8-sig')

    def test_actual_normal_entry_imports_and_unconditional_order(self):
        self.assertEqual(normal_steps(self.entry), tuple(name for _, name in STEPS))

    def test_mutations_cannot_hide_missing_wrong_or_dead_registration(self):
        # Mutate the REAL entry AST, preserving all unrelated preparation code.
        # This proves this test fails for the gap that private-only builds hid.
        normal_steps(self.entry)
        for module, name in STEPS:
            tree = ast.parse(self.entry)
            hook = next(n for n in ast.walk(tree) if isinstance(n, ast.Assign)
                        and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name)
                        and n.value.func.id == name)
            for kind in ('missing', 'wrong-args', 'wrong-output', 'conditional'):
                changed = ast.parse(self.entry)
                fn = next(n for n in changed.body if isinstance(n, ast.FunctionDef) and n.name == 'prepare_full')
                loop = next(n for n in fn.body if isinstance(n, ast.For)
                            and any(isinstance(c, ast.Call) and isinstance(c.func, ast.Attribute)
                                    and c.func.attr == 'glob' and c.args and isinstance(c.args[0], ast.Constant)
                                    and c.args[0].value == 'Methods_*.cpp' for c in ast.walk(n.iter)))
                index = next(i for i, n in enumerate(loop.body) if isinstance(n, ast.Assign)
                             and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name)
                             and n.value.func.id == name)
                node = loop.body[index]
                if kind == 'missing':
                    del loop.body[index]
                elif kind == 'wrong-args':
                    node.value.args[1] = ast.Constant(value='Methods_01.cpp')
                elif kind == 'wrong-output':
                    node.targets[0] = ast.Name(id='unused_text', ctx=ast.Store())
                else:
                    loop.body[index] = ast.If(test=ast.Constant(value=False), body=[node], orelse=[])
                with self.subTest(name=name, mutation=kind), self.assertRaises(ValueError):
                    normal_steps(ast.unparse(ast.fix_missing_locations(changed)))
            wrong = self.entry.replace('from '+module+' import '+name,
                                       'from controls_scene_hooks import '+name, 1)
            with self.subTest(name=name, mutation='wrong-import'), self.assertRaises(ValueError):
                normal_steps(wrong)
        tree = ast.parse(self.entry)
        fn = next(n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'prepare_full')
        loop = next(n for n in fn.body if isinstance(n, ast.For) and any(isinstance(c, ast.Constant) and c.value == 'Methods_*.cpp' for c in ast.walk(n.iter)))
        positions = [i for i, n in enumerate(loop.body) if isinstance(n, ast.Assign)
                     and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name)
                     and n.value.func.id in (STEPS[0][1], STEPS[1][1])]
        loop.body[positions[0]], loop.body[positions[1]] = loop.body[positions[1]], loop.body[positions[0]]
        with self.assertRaises(ValueError):
            normal_steps(ast.unparse(ast.fix_missing_locations(tree)))

    def test_real_entry_order_composes_and_reverses_all_six_units(self):
        names = normal_steps(self.entry)
        bases = synthetic_parent_sources()
        self.assertEqual(set(bases), set(UNITS))
        calls = r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call'
        parent_tokens = ('nfs_controls_text::', 'nfs_controls_sidebar_text::',
                         'nfs_controls_sidebar_live::', 'nfs_controls_scene_live::',
                         'nfs_graphics_scene_live::', 'nfs_audio_scene_live::',
                         'guest_store<', 'GuestCopyTransaction', 'existing_observation')
        for name in UNITS:
            for crlf in (False, True):
                source = bases[name].replace('\n', '\r\n') if crlf else bases[name]
                with self.subTest(name=name, crlf=crlf):
                    after = source
                    for hook in names:
                        after = ADDITIONS[hook](after, name)
                    restored = after
                    for remove in REMOVALS:
                        restored = remove(restored)
                    self.assertEqual(restored.encode(), source.encode())
                    self.assertEqual(re.findall(calls, after), re.findall(calls, source))
                    for token in parent_tokens:
                        self.assertEqual(after.count(token), source.count(token), token)
                    self.assertIn('hd/ControlsSetupTextGameHooks.inc', after)
                    if name == 'Methods_03.cpp':
                        self.assertIn('hd/ControlsSetupSceneQueueAdapter.inc', after)
                    else:
                        self.assertIn('hd/ControlsSetupSidebarLiveGameHooks.inc', after)
                        self.assertIn('hd/ControlsSetupLiveGameHooks.inc', after)

    def test_live_and_sidebar_dependency_failures_are_not_silent_noops(self):
        bases = synthetic_parent_sources()
        for name in UNITS:
            if name == 'Methods_03.cpp':
                continue
            source = bases[name]
            with self.subTest(name=name, missing='text+sidebar'), self.assertRaises(ValueError):
                add_controls_setup_live_hooks(source, name)
            text = add_controls_setup_text_hooks(source, name)
            with self.subTest(name=name, missing='sidebar'), self.assertRaises(ValueError):
                add_controls_setup_live_hooks(text, name)
        # Sidebar's lexical owner dependency lives in Methods01; the other units
        # independently observe stores/selection and need not have that owner.
        with self.assertRaises(ValueError):
            add_controls_setup_sidebar_hooks(bases['Methods_01.cpp'], 'Methods_01.cpp')


if __name__ == '__main__':
    unittest.main()
