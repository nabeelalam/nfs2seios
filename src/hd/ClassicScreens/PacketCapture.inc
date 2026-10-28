// PRIVATE read-only once-only payload capture; inserted only after accepted begin retains view.
#include <errno.h>
static void nfs_ls_capture_packet(const NfsLoadSaveLiveView*v,const NfsNativeLogoTargetProof*t){
 static int attempted=0;const int saved=errno;FILE*f=NULL;int ok=0;const char*path=getenv("NFS_PRIVATE_LOADSAVE_PACKET_CAPTURE");
 if(attempted||!path||!path[0]){errno=saved;return;}
 attempted=1;
 if(!v||!t||(v->schema!=1&&v->schema!=2)||v->width!=640||v->height!=480||v->pixels!=307200||!v->request||!v->content||!v->owner||!v->clean||!v->original||!v->text.vertices||!v->text.vertexCount||v->text.vertexCount>16384||sizeof(NfsSoftwareTextVertex)!=32||v->text.font.schema!=1||v->text.font.width!=1024||v->text.font.height!=512||v->text.font.bytes!=524288||!v->text.font.alpha||!t->width||!t->height||t->width>4096||t->height>4096)goto done;
 f=fopen(path,"wbx");if(!f)goto done;
 {
  const uint32_t h[16]={1,640,480,t->width,t->height,t->framebuffer,1024,512,307200,(uint32_t)v->text.vertexCount,32,524288,(uint32_t)t->sampling.magFilter,0,0,0};
  const uint64_t ids[4]={v->request,v->content,v->text.font.identity,t->contextGeneration};
  const size_t vertexBytes=v->text.vertexCount*32;
  ok=fwrite("LSIPKT01",1,8,f)==8&&fwrite(h,1,sizeof h,f)==sizeof h&&fwrite(ids,1,sizeof ids,f)==sizeof ids&&fwrite(t->sampling.projection,1,64,f)==64&&fwrite(t->sampling.viewport,1,16,f)==16&&fwrite(v->text.font.sha256,1,32,f)==32&&fwrite(v->clean,1,614400,f)==614400&&fwrite(v->original,1,614400,f)==614400&&fwrite(v->text.vertices,1,vertexBytes,f)==vertexBytes&&fwrite(v->text.font.alpha,1,524288,f)==524288&&fwrite("LSIPEND1",1,8,f)==8;
 }
 if(fflush(f))ok=0;if(fclose(f))ok=0;f=NULL;
 done:if(f)fclose(f);fprintf(stderr,"LOADSAVE PACKET_CAPTURE status=%s request=%llu content=%llu\n",ok?"complete":"failed",(unsigned long long)(v?v->request:0),(unsigned long long)(v?v->content:0));errno=saved;
}
