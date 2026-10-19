#include <math.h>
static int nfs_hd_cell_layout_available;
// Natural W/1 cells and exact original record rows use copied vertices only.
// Unsupported layouts retain original rendering; guest attributes are immutable.
static int nfs_hd_label_geometry(int selector,const GrVertex *a,const GrVertex *b,const GrVertex *c,GrVertex output[3]){
 if(!nfs_hd_cell_layout_available)return 0;
 const int wide=selector==34 || selector==38;
 if(!wide && (selector<12 || selector>48))return 0;
 const GrVertex *v[3]={a,b,c};float x0=1000,y0=1000,x1=-1,y1=-1;
 for(unsigned i=0;i<3;++i){float x=v[i]->x-VertexSnap,y=v[i]->y-VertexSnap;if(v[i]->oow!=1 || !(x>=0 && x<=640 && y>=0 && y<=480))return -1;x0=fminf(x0,x);x1=fmaxf(x1,x);y0=fminf(y0,y);y1=fmaxf(y1,y);}
 // Only the six proven RECORD caption cells in the two original HUD rows.
 // Reposition glyph centers; never scale the font artwork or guest geometry.
 float headingShift=0;int heading=0;
 const int timeGlyph=selector==26 || (selector>=38 && selector<=48);
 const int raised=selector==38 || selector==47 || selector==48;
 const float expectedWidth=raised?4.0f:8.0f;
 const int timeRow=timeGlyph && fabsf(x1-x0-expectedWidth)<.01f && fabsf(y1-y0-8)<.01f && x0>=8 && x1<=100 &&
   (fabsf(y0-(raised?227:228))<.01f || fabsf(y0-(raised?467:468))<.01f);
 if(!wide && !timeRow){
  static const int glyph[6]={29,16,14,26,29,15};
  if(fabsf(x1-x0-8)<.01f && fabsf(y1-y0-8)<.01f && (fabsf(y0-212)<.01f || fabsf(y0-452)<.01f))
   for(unsigned j=0;j<6;++j)if(selector==glyph[j] && fabsf(x0-(26+9*(int)j))<.01f){heading=1;headingShift=1.5f-.22f*((x0+4)-52.5f);break;}
  // Captured original POS/LAP captions use the same 9-unit advance. Keep
  // their centers and baselines, tightening only the gaps between letters.
  if(!heading && fabsf(x1-x0-8)<.01f && fabsf(y1-y0-8)<.01f && (fabsf(y0-14)<.01f || fabsf(y0-255)<.01f)){
   static const int captions[2][3]={{27,26,30},{23,12,27}};
   for(unsigned word=0;word<2;++word)for(unsigned j=0;j<3;++j){float start=word?560.0f:401.0f;
    if(selector==captions[word][j] && fabsf(x0-(start+9*j))<.01f){heading=1;headingShift=-.22f*((x0+4)-(start+13));}
   }
  }
  if(!heading)return 0;
 }
 if(!(x1>x0 && y1>y0))return -1;unsigned corners=0;
 for(unsigned i=0;i<3;++i){float x=v[i]->x-VertexSnap,y=v[i]->y-VertexSnap;int right=fabsf(x-x1)<.01f,bottom=fabsf(y-y1)<.01f;if((!right && !(fabsf(x-x0)<.01f)) || (!bottom && !(fabsf(y-y0)<.01f)))return -1;corners|=1u<<(right+2*bottom);}
 if(corners!=7 && corners!=11 && corners!=13 && corners!=14)return -1;
 if(heading){for(unsigned i=0;i<3;++i){output[i]=*v[i];output[i].x+=headingShift;}return 1;}
 if(timeRow){
  // Original full-string advance measurement is retained. Move its common
  // anchor to the plaque center; align the replacement font's shared baseline.
  const float center=(x0+x1)*.5f,scale=selector==38?1.5f:1.0f;
  for(unsigned i=0;i<3;++i){output[i]=*v[i];output[i].x=VertexSnap+center-3.5f+(v[i]->x-VertexSnap-center)*scale;output[i].y+=raised?1.0f:0.0f;}return 1;
 }
 float scale=selector==34?1.25f:1.5f,center=(x0+x1)*.5f,left=center-(x1-x0)*scale*.5f,right=center+(x1-x0)*scale*.5f;
 if(left<0 || right>640)return -1;
 for(unsigned i=0;i<3;++i){output[i]=*v[i];output[i].x=VertexSnap+center+(v[i]->x-VertexSnap-center)*scale;}return 1;
}
