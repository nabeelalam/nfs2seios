// SPDX-License-Identifier: MIT
#pragma once
#include "hd/CompactCaptionWordArtwork.h"
#include <thread>
#include <algorithm>
// Included after the real-GLES fixture helpers; no game data or forged source proof.
namespace location_caption_word_gpu_cases {
struct Owner {
 NfsBackgroundSceneCompositor*scene;EGLContext context;std::thread::id thread;uint64_t generation;bool enabled=true;
 static int current(void*data,const NfsBackgroundSceneStage*t){
  auto&o=*static_cast<Owner*>(data);
  if(!o.enabled||!t||eglGetCurrentContext()!=o.context||std::this_thread::get_id()!=o.thread||t->generation!=o.generation||t->contextIdentity!=uintptr_t(o.context)||o.scene->generation!=o.generation)return 0;
  if(!t->framebuffer)return 1;
  unsigned slot=2;for(unsigned i=0;i<2;++i)if(t->framebuffer==o.scene->fbo[i]&&t->framebuffer)slot=i;
  if(slot>1||t->width!=o.scene->width[slot]||t->height!=o.scene->height[slot])return 0;
  GLint fbo=0,type=0,name=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if(GLuint(fbo)!=t->framebuffer)return 0;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
  return type==GL_TEXTURE&&GLuint(name)==o.scene->color[slot];
 }
};
static NfsCompactCaptionWordView word(const nfs_compact_caption_display::Artwork&a){
 NfsCompactCaptionWordView v{};v.schema=1;v.word=2;v.width=208;v.height=128;v.rgba=a.rgba.data();v.bytes=a.rgba.size();std::copy(a.sha.begin(),a.sha.end(),v.sha256);v.left=116;v.top=162;v.right=168;v.bottom=194;return v;
}
static double alpha(const nfs_compact_caption_display::Artwork&a,double x,double y){
 if(x<116||x>=168||y<162||y>=194)return 0;
 const double sx=std::max(0.,std::min(207.,(x-116)*4-.5)),sy=std::max(0.,std::min(127.,(y-162)*4-.5));
 const unsigned ix=unsigned(sx),iy=unsigned(sy),jx=std::min(ix+1,207u),jy=std::min(iy+1,127u);const double fx=sx-ix,fy=sy-iy;
 auto at=[&](unsigned X,unsigned Y){return double(a.rgba[(size_t(Y)*208+X)*4+3]);};
 return (at(ix,iy)*(1-fx)+at(jx,iy)*fx)*(1-fy)+(at(ix,jy)*(1-fx)+at(jx,jy)*fx)*fy;
}
static void image(const std::vector<unsigned char>&pixels,Target&t,const nfs_compact_caption_display::Artwork&a){
 const auto panel=over({{20,40,60}},{{80,40,0,128}});const double ratio=t.h/480.;unsigned solid=0,edge=0,padding=0;
 // Independent pixel-center/bilinear projection includes the box's exterior,
 // proving fixed placement and transparent padding rather than a selected sample.
 for(unsigned py=unsigned(160*ratio);py<unsigned(196*ratio);++py)for(unsigned px=unsigned(114*ratio);px<unsigned(170*ratio);++px){
  const double A=alpha(a,(px+.5)/ratio,(py+.5)/ratio);solid+=A>254;edge+=A>2&&A<253;padding+=A<1;
  const size_t at=(size_t(t.h-1-py)*t.w+px)*4;
  for(unsigned k=0;k<3;++k){const int expected=int(std::lround(A+panel[k]*(1-A/255.)));ck(std::abs(int(pixels[at+k])-expected)<=2,"actual Location word2 RGBA and fixed-box projection");}
 }
 ck(solid>40&&edge>30&&padding>100,"authored word has opaque glyphs, antialiasing and transparent padding");
}
static void depth(Target&t){
 // GLES2 cannot portably read depth. A real fullscreen GL_EQUAL probe at the
 // original clear value must pass every pixel after COLOR-only publication.
 NfsBgSaved saved;nfs_bg_save(&saved);const char*vs="attribute vec2 p;void main(){gl_Position=vec4(p,1.,1.);}";
 const char*fs="precision highp float;void main(){gl_FragColor=vec4(17./255.,29./255.,43./255.,1.);}";
 const GLuint v=nfs_hd_test::compileShader(GL_VERTEX_SHADER,vs),f=nfs_hd_test::compileShader(GL_FRAGMENT_SHADER,fs),program=glCreateProgram();
 glAttachShader(program,v);glAttachShader(program,f);glBindAttribLocation(program,0,"p");glLinkProgram(program);GLint linked=0;glGetProgramiv(program,GL_LINK_STATUS,&linked);ck(linked!=0,"depth probe shader");
 glBindFramebuffer(GL_FRAMEBUFFER,t.fbo);glViewport(0,0,t.w,t.h);glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_EQUAL);glDepthMask(GL_FALSE);glDepthRangef(0,1);glColorMask(1,1,1,1);glUseProgram(program);glBindBuffer(GL_ARRAY_BUFFER,0);
 const GLfloat q[]={-1,-1,1,-1,-1,1,-1,1,1,-1,1,1};glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,q);glDrawArrays(GL_TRIANGLES,0,6);
 auto pixels=read(t);for(size_t i=0;i<pixels.size();i+=4)for(unsigned k=0;k<3;++k)ck(std::abs(int(pixels[i+k])-int(k==0?17:k==1?29:43))<=1,"native depth clear survives Location caption staging and commit");
 nfs_bg_restore(&saved);glDeleteProgram(program);glDeleteShader(v);glDeleteShader(f);
}
// The fixture already counts draws/uploads. Guard revocation additionally takes
// a read-only GL state snapshot, proving no subsequent state mutation or guard
// re-entry. This is representative loss coverage, not a trace of every GL query.
struct RevokingOwner {
 Owner*owner=nullptr;unsigned guards=0,afterLoss=0,atGuard=0,baseUploads=0,baseDraws=0,drawsAtLoss=0,uploadsAtLoss=0;bool onUpload=false,onDraw=false,lost=false;State state{};
 void arm(unsigned guard,bool upload,bool draw){guards=afterLoss=0;atGuard=guard;baseUploads=uploads;baseDraws=draws;onUpload=upload;onDraw=draw;lost=false;}
 static int current(void*data,const NfsBackgroundSceneStage*t){
  auto&o=*static_cast<RevokingOwner*>(data);if(o.lost){++o.afterLoss;return 0;}if(!Owner::current(o.owner,t))return 0;++o.guards;
  if((o.atGuard&&o.guards==o.atGuard)||(o.onUpload&&uploads>o.baseUploads)||(o.onDraw&&draws>o.baseDraws)){o.state=capture();o.drawsAtLoss=draws;o.uploadsAtLoss=uploads;o.lost=true;return 0;}return 1;
 }
 void stopped(){ck(lost&&!afterLoss&&draws==drawsAtLoss&&uploads==uploadsAtLoss&&equal(state,capture()),"mid-operation revoked owner permits no later guard re-entry, draw/upload or GL state mutation");}
};
static void midOperationLoss(NfsBackgroundSceneCompositor&s,Owner&owner,const NfsCompactCaptionWordView&v,const NfsFrameLegacySampling&sampling){
 NfsBgSaved saved;nfs_bg_save(&saved);NfsBackgroundSceneStage guard{};guard.generation=s.generation;guard.contextIdentity=uintptr_t(owner.context);guard.ownerCurrent=Owner::current;guard.ownerUser=&owner;
 RevokingOwner loss;loss.owner=&owner;auto revoked=guard;revoked.ownerCurrent=RevokingOwner::current;revoked.ownerUser=&loss;
 // A real upload completes, then its post-call guard loses authority. Partial
 // cold resources are deliberately left to this test context's later destruction.
 NfsLocationCaptionGpu cold{};loss.arm(0,true,false);ck(!nfs_location_caption_prepare(&cold,&v,2,&revoked),"cold upload post-call owner loss declines");loss.stopped();ck(!cold.generation&&!cold.program&&!cold.texture,"cold loss forgets unpublished metadata");nfs_bg_restore(&saved);
 // A warm resource query must also stop at its post-call guard. Retained actual
 // names are private test resources and cleaned only after authority is restored.
 NfsLocationCaptionGpu warm{};ck(nfs_location_caption_prepare(&warm,&v,2,&guard),"prepare private word resources for warm loss");GLuint program=warm.program,texture=warm.texture;loss.arm(3,false,false);ck(!nfs_location_caption_prepare(&warm,&v,2,&revoked),"warm IsProgram post-call owner loss declines");loss.stopped();ck(!warm.generation&&!warm.program&&!warm.texture&&glIsProgram(program)&&glIsTexture(texture),"warm loss forgets without deleting prior owned resources");nfs_bg_restore(&saved);glDeleteProgram(program);glDeleteTextures(1,&texture);
 NfsLocationCaptionGpu drawing{};ck(nfs_location_caption_prepare(&drawing,&v,2,&guard),"prepare private word resources for draw loss");program=drawing.program;texture=drawing.texture;const unsigned slot=s.pendingSlot;ck(slot<2&&s.fbo[slot]&&s.width[slot]==unsigned(sampling.viewport[3])*4u/3u,"registered staging slot for draw loss");auto stage=revoked;stage.framebuffer=s.fbo[slot];stage.width=s.width[slot];stage.height=s.height[slot];stage.sampling=sampling;glBindFramebuffer(GL_FRAMEBUFFER,stage.framebuffer);glViewport(sampling.viewport[0],sampling.viewport[1],sampling.viewport[2],sampling.viewport[3]);loss.arm(0,false,true);ck(!nfs_location_caption_draw(&drawing,&v,&stage),"actual draw post-call owner loss declines");loss.stopped();ck(!drawing.generation&&!drawing.program&&!drawing.texture&&glIsProgram(program)&&glIsTexture(texture),"draw loss forgets with no foreign cleanup");nfs_bg_restore(&saved);glDeleteProgram(program);glDeleteTextures(1,&texture);
 NfsLocationCaptionGpu retiring{};ck(nfs_location_caption_prepare(&retiring,&v,2,&guard),"prepare private word resources for retirement loss");program=retiring.program;texture=retiring.texture;loss.arm(3,false,false);ck(!nfs_location_caption_retire(&retiring,&revoked,1),"retirement first delete post-call owner loss declines");loss.stopped();ck(!retiring.generation&&!retiring.program&&!retiring.texture&&!glIsProgram(program)&&glIsTexture(texture),"retirement loss stops before deleting remaining texture");nfs_bg_restore(&saved);glDeleteTextures(1,&texture);
 ck(glGetError()==GL_NO_ERROR,"representative context-loss fixture cleanup");
}
static void run(NfsBackgroundSceneCompositor&s,uint64_t&serial){
 auto authored=nfs_compact_caption_display::artwork(2);ck(bool(authored)&&authored->width==208&&authored->height==128,"actual pinned authored Location word2 decoder");
 for(unsigned height:{480u,1640u}){
  Target t(height*4/3,height);t.seed();NfsFrameLegacySampling sampling{};ck(nfs_frame_gpu_legacy_sampling(t.w,t.h,GL_LINEAR,&sampling),"word2 logical/native sampling");native(t,sampling);
  std::vector<unsigned char>coverage(1024u*512u,128);std::array<NfsSoftwareTextVertex,6>vertices{{{116,167,.2f,.2f,1,1,1,1},{116,186,.2f,.8f,1,1,1,1},{168,167,.8f,.2f,1,1,1,1},{168,167,.8f,.2f,1,1,1,1},{116,186,.2f,.8f,1,1,1,1},{168,186,.8f,.8f,1,1,1,1}}};
  uint16_t preview=0xf800;unsigned char mask=255;NfsLocationSceneFrameView v{};v.schema=NFS_LOCATION_SCENE_RUNTIME_SCHEMA;v.scene=view(++serial,0);v.scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;v.caption.logicalWidth=343;v.caption.logicalHeight=480;v.caption.font={1,1024,512,17,coverage.data(),coverage.size(),{}};ck(nfs_texture_sha256(coverage.data(),coverage.size(),v.caption.font.sha256),"word2 fallback font digest");v.caption.vertices=vertices.data();v.caption.vertexCount=vertices.size();v.preview.schema=1;v.preview.left=400;v.preview.top=300;v.preview.width=v.preview.height=1;v.preview.pixels565=&preview;v.preview.pixelCount=1;v.preview.coverage=&mask;v.preview.coverageCount=1;ck(nfs_texture_sha256((const unsigned char*)&preview,2,v.preview.sha256)&&nfs_texture_sha256(&mask,1,v.preview.coverageSha256),"word2 independent original preview pin");
  Owner owner{&s,eglGetCurrentContext(),std::this_thread::get_id(),s.generation};
  nfs_bg_scene_location_caption_owner(&s,uintptr_t(owner.context),Owner::current,&owner);
  auto q=proof(t,sampling,serial,s.generation);auto before=capture();auto original=read(t);
  // The unchanged empty-word route remains a real software caption publication.
  ck(nfs_bg_scene_begin_location(&s,&v,&q)&&read(t)==original&&equal(before,capture()),"empty word retains genuine original-caption fallback");ck(nfs_bg_scene_finish_location(&s,serial,&q)&&equal(before,capture()),"legacy caption commit preserves complete GL state");auto legacy=read(t);sample(legacy,t,130,175,over(over({{20,40,60}},{{80,40,0,128}}),{{128,128,128,128}}));
  v.captionWord=word(*authored);v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;
  ck(nfs_bg_scene_begin_location(&s,&v,&q)&&s.lastBeginKind==1&&draws&&read(t)==original&&equal(before,capture()),"cold authored Location begins unpublished and restores state");ck(nfs_bg_scene_finish_location(&s,serial,&q)&&equal(before,capture()),"authored Location COLOR commit preserves native state");auto rendered=read(t);image(rendered,t,*authored);ck(rendered!=legacy,"authored Location differs from legacy font");sample(rendered,t,400,300,{{255,0,0}});
  v.scene.serial=++serial;q.depthClearSerial=serial;draws=uploads=0;ck(nfs_bg_scene_begin_location(&s,&v,&q)&&s.lastBeginKind==2&&!draws&&!uploads,"word2 warm cache reuses complete image without GL draw/upload");ck(nfs_bg_scene_finish_location(&s,serial,&q)&&read(t)==rendered,"warm word2 publication exact");
  preview=0x001f;ck(nfs_texture_sha256((const unsigned char*)&preview,2,v.preview.sha256),"word2 changed independent preview pin");v.scene.serial=++serial;q.depthClearSerial=serial;draws=uploads=0;ck(nfs_bg_scene_begin_location(&s,&v,&q)&&s.lastBeginKind==1&&draws&&uploads==1,"preview-only change independently invalidates authored-word cache");ck(nfs_bg_scene_finish_location(&s,serial,&q),"word2 with changed preview publishes");image(read(t),t,*authored);sample(read(t),t,400,300,{{0,0,255}});
  for(unsigned fault=0;fault<18;++fault){
   auto bad=v;auto bytes=authored->rgba;bad.scene.serial=++serial;q.depthClearSerial=serial;
   if(fault==0){bad.schema=2;bad.caption.font.alpha=reinterpret_cast<const unsigned char*>(uintptr_t(1));bad.caption.vertices=reinterpret_cast<const NfsSoftwareTextVertex*>(uintptr_t(1));bad.preview.pixels565=reinterpret_cast<const uint16_t*>(uintptr_t(1));bad.preview.coverage=reinterpret_cast<const unsigned char*>(uintptr_t(1));bad.captionWord.rgba=reinterpret_cast<const unsigned char*>(uintptr_t(1));}
   if(fault==1)bad.captionWord.schema=0;if(fault==2)bad.captionWord.word=1;if(fault==3)--bad.captionWord.width;if(fault==4)--bad.captionWord.height;if(fault==5)--bad.captionWord.bytes;if(fault==6)bad.captionWord.rgba=nullptr;if(fault==7)bad.captionWord.sha256[0]^=1;if(fault==8){bytes[3]^=1;bad.captionWord.rgba=bytes.data();}if(fault==9)bad.captionWord.left+=1;if(fault==10)bad.captionWord.top+=1;if(fault==11)bad.captionWord.right+=1;if(fault==12)bad.captionWord.bottom+=1;if(fault==13)bad.captionWord.left=NAN;if(fault==14)bad.captionWord.top=INFINITY;if(fault==15)q.contextGeneration++;if(fault==16)owner.enabled=false;if(fault==17)owner.thread=std::thread::id{};
   t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;
   ck(!nfs_bg_scene_begin_location(&s,&bad,&q)&&!draws&&!uploads&&equal(before,capture())&&read(t)==original,"malformed/old-schema/context/owner word2 fails without original mutation");q.contextGeneration=s.generation;owner.enabled=true;owner.thread=std::this_thread::get_id();
  }
  // A word GPU name may not alias any independently owned scene resource.
  const GLuint aliases[]={s.caption.texture,s.previewTexture,s.textures[0],s.logo.texture,s.color[0],s.color[1],t.texture};
  for(GLuint alias:aliases){if(!alias)continue;auto owned=s.locationCaption;s.locationCaption.texture=alias;v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;
   ck(!nfs_bg_scene_begin_location(&s,&v,&q)&&!draws&&!uploads&&equal(before,capture())&&read(t)==original&&glIsTexture(alias),"borrowed background/font/preview/logo/staging aliases decline without delete or draw");s.locationCaption=owned;
  }
  {auto owned=s.locationCaption;s.locationCaption.program=GLuint(0xfffffffeu);v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;ck(!nfs_bg_scene_begin_location(&s,&v,&q)&&!draws&&!uploads&&equal(before,capture())&&read(t)==original,"invalid word program cannot authorize a cached image");s.locationCaption=owned;}
  // Force a genuine late word draw failure, after background/preview staging.
  v.preview.left+=1;v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);unsigned victim=s.nextSlot;failDrawProgram=s.locationCaption.program;
  ck(!nfs_bg_scene_begin_location(&s,&v,&q),"authored word draw error refuses entire unpublished image");failDrawProgram=0;ck(!s.pendingSerial&&!s.imageReady[victim]&&equal(before,capture())&&read(t)==original,"late word fault preserves original native color and state");
  // Exact target/depth survives a successful retry after all rejected borrows.
  v.scene.serial=++serial;q.depthClearSerial=serial;ck(nfs_bg_scene_begin_location(&s,&v,&q)&&nfs_bg_scene_finish_location(&s,serial,&q),"word2 recovers after refusals");depth(t);
  const GLuint texture=s.locationCaption.texture,program=s.locationCaption.program,previewName=s.previewTexture,fontName=s.caption.texture;
  ck(nfs_bg_scene_location_caption_owner(&s,0,nullptr,nullptr)&&!s.locationCaption.texture&&!s.locationCaption.program&&!glIsTexture(texture)&&!glIsProgram(program)&&glIsTexture(previewName)&&glIsTexture(fontName),"current owner clear deletes only owned caption resources");
  v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;ck(!nfs_bg_scene_begin_location(&s,&v,&q)&&!draws&&!uploads&&equal(before,capture())&&read(t)==original,"nonempty Location word cannot borrow a missing trusted owner");
  nfs_bg_scene_location_caption_owner(&s,uintptr_t(owner.context),Owner::current,&owner);
  v.scene.serial=++serial;q.depthClearSerial=serial;t.seed();native(t,sampling);before=capture();original=read(t);draws=uploads=0;failUpload=true;
  ck(!nfs_bg_scene_begin_location(&s,&v,&q),"cold word upload failure refuses publication");failUpload=false;ck(!draws&&!s.pendingSerial&&equal(before,capture())&&read(t)==original,"failed word upload cannot mutate original target");
  v.scene.serial=++serial;q.depthClearSerial=serial;ck(nfs_bg_scene_begin_location(&s,&v,&q)&&s.lastBeginKind==1&&nfs_bg_scene_finish_location(&s,serial,&q),"retired caption is recreated without reusing a stale image");image(read(t),t,*authored);
  midOperationLoss(s,owner,v.captionWord,sampling);
  ck(nfs_bg_scene_location_caption_owner(&s,0,nullptr,nullptr),"release exact test caption owner before target lifetime ends");
 }
}
} // namespace location_caption_word_gpu_cases
