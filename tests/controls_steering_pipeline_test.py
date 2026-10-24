# SPDX-License-Identifier: MIT
"""Maintained entry wiring and reversible multi-family Steering composition."""
import ast
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'));sys.path.insert(0,str(ROOT/'tests'))
from controls_steering_live_hooks_test import bases as live_bases
from controls_steering_sidebar_hooks_test import fixtures as sidebar_fixtures
from controls_steering_scene_hooks_test import composed as scene_base
from controls_setup_scene_hooks import add_controls_setup_scene_hooks
STEPS=tuple(('controls_steering_'+part+'_hooks','add_controls_steering_'+part+'_hooks')for part in ('text','chart','sidebar','live','scene'))
import importlib
ADDITIONS={};REMOVALS=[]
for module,name in STEPS:
    loaded=importlib.import_module(module);ADDITIONS[name]=getattr(loaded,name)
    REMOVALS.insert(0,getattr(loaded,name.replace('add_','remove_',1)))

def bases():
    result=sidebar_fixtures();result.update(live_bases())
    for name,source in result.items():
        for remove in REMOVALS:source=remove(source)
        result[name]=source
    result['Methods_03.cpp']=add_controls_setup_scene_hooks(scene_base(),'Methods_03.cpp')
    return result

def normal_steps(source):
    """Admit direct real-loop wiring; never import/execute preparation itself."""
    tree = ast.parse(source)
    for module, name in STEPS:
        imports = [(node.module, item.name, item.asname)
                   for node in tree.body if isinstance(node, ast.ImportFrom)
                   for item in node.names if (item.asname or item.name) == name]
        if imports != [(module, name, None)]:
            raise ValueError('missing/ambiguous/wrong Steering hook import: '+name)
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
            raise ValueError('Steering hook must run once unconditionally on the emitted text: '+name)
        call = calls[0]
        if call.keywords or len(call.args) != 2 or ast.dump(call.args[0]) != ast.dump(ast.Name(id='text', ctx=ast.Load())) or ast.dump(call.args[1]) != ast.dump(ast.Attribute(value=ast.Name(id='path', ctx=ast.Load()), attr='name', ctx=ast.Load())):
            raise ValueError('Steering hook must receive current text and actual unit name: '+name)
        positions.append(direct[0][0])
    if positions != list(range(positions[0], positions[0]+len(STEPS))):
        raise ValueError('required Steering Text -> Chart -> Sidebar -> Live -> Scene order changed')
    # Parent dependencies must already have modified this same text value.
    for name in ('add_controls_setup_text_hooks', 'add_controls_setup_sidebar_hooks',
                 'add_controls_setup_live_hooks', 'add_controls_setup_scene_hooks',
                 'add_controls_text_hooks', 'add_controls_sidebar_hooks',
                 'add_controls_live_hooks', 'add_controls_scene_hooks'):
        parent = [i for i, n in enumerate(loop.body)
                  if isinstance(n, ast.Assign) and len(n.targets) == 1
                  and isinstance(n.targets[0], ast.Name) and n.targets[0].id == 'text'
                  and isinstance(n.value, ast.Call) and isinstance(n.value.func, ast.Name)
                  and n.value.func.id == name]
        if len(parent) != 1 or parent[0] >= positions[0]:
            raise ValueError('parent dependency must precede Steering pipeline: '+name)
    after = loop.body[positions[-1]+1:]
    # Require the chain output itself to reach the maintained unit write. A
    # dead helper call or overwrite/reset before the write must not pass.
    if not after or not isinstance(after[0], ast.Expr):
        raise ValueError('Steering pipeline output is not written directly')
    call = after[0].value
    if not isinstance(call, ast.Call) or not isinstance(call.func, ast.Attribute) or call.func.attr != 'write_text' or call.keywords or len(call.args) != 1 or not isinstance(call.args[0], ast.Name) or call.args[0].id != 'text':
        raise ValueError('Steering pipeline output is not the emitted unit text')
    expected = ast.BinOp(left=ast.Name(id='dest', ctx=ast.Load()), op=ast.Div(), right=ast.Attribute(value=ast.Name(id='path', ctx=ast.Load()), attr='name', ctx=ast.Load()))
    if ast.dump(call.func.value) != ast.dump(expected):
        raise ValueError('Steering pipeline writes a different destination/unit')
    return tuple(names)


class SteeringPipeline(unittest.TestCase):
    def test_real_entry_and_reversible_six_unit_composition(self):
        source=(ROOT/'scripts/prepare_full_mapped.py').read_text()
        names=normal_steps(source)
        fixture=bases()
        self.assertEqual(set(fixture),{'Methods_01.cpp','Methods_03.cpp','Methods_09.cpp','Methods_10.cpp','Methods_13.cpp','Methods_14.cpp'})
        for name,original in fixture.items():
            for text in (original,original.replace('\n','\r\n')):
                out=text
                for step in names:out=ADDITIONS[step](out,name)
                restored=out
                for remove in REMOVALS:restored=remove(restored)
                self.assertEqual(restored,text)
                self.assertEqual(re.findall(r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call',out),re.findall(r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call',text))
                for token in ('guest_store<','GuestCopyTransaction','nfs_controls_setup_','nfs_controls_scene_live::'):
                    self.assertEqual(out.count(token),text.count(token))
    def test_entry_bypass_missing_or_wrong_result_declines(self):
        source=(ROOT/'scripts/prepare_full_mapped.py').read_text()
        for _,name in STEPS:
            line='        text = '+name+'(text, path.name)'
            for replacement in ('',line.replace('text =','ignored ='),line.replace('path.name','"Methods_01.cpp"'),'        if False:\n    '+line):
                with self.subTest(name=name,mutation=replacement),self.assertRaises(ValueError):normal_steps(source.replace(line,replacement))
    def test_renderer_installs_hooks_and_exact_helper_copy(self):
        tree=ast.parse((ROOT/'scripts/prepare_renderer.py').read_text())
        fn=next(n for n in tree.body if isinstance(n,ast.FunctionDef)and n.name=='prepare_renderer')
        call=[(i,n)for i,n in enumerate(fn.body)if isinstance(n,ast.Assign)and isinstance(n.value,ast.Call)and isinstance(n.value.func,ast.Name)and n.value.func.id=='add_controls_steering_renderer_hooks']
        self.assertEqual(len(call),1);i,n=call[0]
        self.assertEqual(ast.unparse(n),'text = add_controls_steering_renderer_hooks(text)')
        loop=fn.body[i+1];self.assertIsInstance(loop,ast.For);self.assertEqual(ast.unparse(loop.iter),'CONTROLS_STEERING_HELPERS')
        self.assertIn("dest / 'hd' / name",ast.unparse(loop))
        from controls_steering_renderer_hooks import HELPERS
        self.assertEqual(len(HELPERS),len(set(HELPERS)))
        for name in HELPERS:self.assertTrue((ROOT/'src/hd'/name).is_file(),name)
    def test_compiled_artwork_has_all_four_authored_levels(self):
        source=(ROOT/'CMakeLists.txt').read_text();loop=source[source.index('foreach(target glide_gles2 renderer_pixel_tests'):]
        self.assertIn('src/hd/ControlsSteeringBarData.c',loop[:loop.index('endforeach()')])
        data=(ROOT/'src/hd/ControlsSteeringBarData.c').read_text()
        for level in range(4):
            name='ControlsSteeringBarLevel%d.inc'%level
            self.assertIn('#include "'+name+'"',data)
            self.assertTrue((ROOT/'src/hd'/name).is_file())
if __name__=='__main__':unittest.main()
