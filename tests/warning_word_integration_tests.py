import ast,re,shutil,tempfile,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def body(text,mark):
 a=text.index(mark);b=text.index('{',a);depth=1;c=b+1
 while depth:
  depth+=(text[c]=='{')-(text[c]=='}');c+=1
 return text[a:c]
with tempfile.TemporaryDirectory(prefix='warning-gates-') as td:
 td=Path(td);dest=td/'generated';(dest/'Glide2x').mkdir(parents=True)
 source=(ROOT/'scripts/prepare_renderer.py').read_text();tree=ast.parse(source)
 functions={n.name:n for n in tree.body if isinstance(n,ast.FunctionDef)}
 sys.path.insert(0,str(ROOT/'scripts'))
 from pause_pixel_chunks import pause_pixel_parts
 from frontend_pixel_limits import FRONTEND_PIXEL_MAX_PARTS
 env={'ROOT':ROOT,'shutil':shutil,'dest':dest,'re':re,'pause_pixel_parts':pause_pixel_parts,'FRONTEND_PIXEL_MAX_PARTS':FRONTEND_PIXEL_MAX_PARTS}
 for n in tree.body:
  if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='FRONTEND_PIXEL_MAX_PARTS' for t in n.targets):exec(compile(ast.Module(body=[n],type_ignores=[]),'inventory-constant','exec'),env)
 for name in ('frontend_pixel_parts',):
  exec(compile(ast.Module(body=[functions[name]],type_ignores=[]),str(ROOT/'scripts/prepare_renderer.py'),'exec'),env)
 fn=functions['prepare_hd_atlas'];loop=next(n for n in fn.body if isinstance(n,ast.For) and 'WarningWordPixels.inc' in ast.unparse(n))
 for n in fn.body:
  if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id in ('font_parts','pause_parts') for t in n.targets):exec(compile(ast.Module(body=[n],type_ignores=[]),'inventory','exec'),env)
 exec(compile(ast.Module(body=[loop],type_ignores=[]),'actual-copy-loop','exec'),env)
 for name in ('WarningWordPixels.inc','WarningWordSelect.h','WarningWordShader.inc','SpeedShader.inc','BuiltinTiming.h','HdGlyphSelect.h'):
  assert (dest/'Glide2x'/name).read_bytes()==(ROOT/'src'/name).read_bytes(),name
 for name in ('SpeedShader.inc','BuiltinTiming.h','HdGlyphSelect.h'):
  for inc in re.findall(r'^\s*#include\s+"([^"\n]+)"',(dest/'Glide2x'/name).read_text(),re.M):assert (dest/'Glide2x'/inc).is_file(),inc
 runtime=body((ROOT/'src/RendererHdAtlas.inc').read_text(),'static int nfs_hd_triangle_selected(')
 pre='''#include "BuiltinTiming.h"
#include "WarningWordSelect.h"
#include <cassert>
struct GrVertex {float x,y,oow,r,g,b,a;struct {float sow,tow;}tmuvtx[1];};
static int nfs_hd_selected=1,nfs_hd_builtin_enabled=1,nfs_hd_visible=1,nfs_hd_aux_texture=1,nfs_hd_cell_layout_available=1;
static float VertexSnap=786432;static unsigned nfs_small_font_edge[26],nfs_record_font_edge[11];
static int nfs_front_font_source_page(int){return -1;}
static int nfs_start_light_triangle(const GrVertex*,const GrVertex*,const GrVertex*,const float*){return 0;}
'''
 for name in ('nfs_alternate_logo_triangle','nfs_pause_triangle','nfs_logo_triangle','nfs_hd_digit_triangle'):pre+='static int '+name+'(const float*){return 0;}\n'
 pre+='static int nfs_hd_small_font_triangle(const float*,unsigned){return 0;}\n'
 main='''
int main(){GrVertex v[3]={{786731,786439,1,255,255,255,255,{{.498046875,.498046875}}},{786954,786439,1,255,255,255,255,{{222.12890625,.498046875}}},{786954,786460,1,255,255,255,255,{{222.12890625,20.91796875}}}};
assert(nfs_hd_triangle_selected(v,v+1,v+2)==76);auto saved0=v[0],saved1=v[1],saved2=v[2];v[0].x=786715;v[1].x=v[2].x=786971;v[1].tmuvtx[0].sow=v[2].tmuvtx[0].sow=255;assert(nfs_hd_triangle_selected(v,v+1,v+2)==77);v[0]=saved0;v[1]=saved1;v[2]=saved2;int* gates[]={&nfs_hd_builtin_enabled,&nfs_hd_visible,&nfs_hd_aux_texture,&nfs_hd_cell_layout_available,&nfs_hd_selected};for(auto p:gates){int saved=*p;*p=0;assert(nfs_hd_triangle_selected(v,v+1,v+2)==0);*p=saved;}nfs_hd_selected=2;assert(!nfs_hd_triangle_selected(v,v+1,v+2));nfs_hd_selected=1;
unsigned char foreign[131072]={};NfsHdAtlas atlas={};assert(!nfs_timing_build(&atlas,256,foreign));assert(!atlas.original&&!atlas.rgba);assert(!nfs_timing_build(&atlas,255,foreign));assert(!nfs_timing_build(&atlas,256,nullptr));}
'''
 cpp=td/'gates.cpp';cpp.write_text(pre+runtime+main);exe=td/'gates'
 subprocess.run(['clang++','-std=c++14','-O1','-fsanitize=address,undefined','-I',str(ROOT/'src'),str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
print('PASS actual renderer consumer gates, actual timing producer foreign-source rejection, maintained generation copy inventory and include closure')
