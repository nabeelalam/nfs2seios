// Original black/white polygon pairs are adjacent in all six supported RLFs.
// Replace their coverage in the existing atlas; guest vertices stay immutable.
static int nfs_hd_silver_slash(const GrVertex *a,const GrVertex *b,const GrVertex *c,GrVertex output[3]){
 if(!nfs_hd_builtin_enabled || !nfs_hd_visible || !nfs_hd_aux_texture || !nfs_hd_selected || !nfs_hd_cell_layout_available)return 0;
 int white=a->r==247 && a->g==251 && a->b==247,black=a->r==16 && a->g==16 && a->b==16;if(!white && !black)return 0;
 const GrVertex *v[3]={a,b,c};for(unsigned i=0;i<3;++i)if(v[i]->oow!=1 || v[i]->a!=255 || v[i]->r!=a->r || v[i]->g!=a->g || v[i]->b!=a->b)return 0;
 for(unsigned half=0;half<2;++half)for(unsigned place=0;place<2;++place){float left=place?602:455,top=5+241*half;unsigned indices[3],corners=0;int valid=1;
  const float dx[4]={0,5,8,3},dy[4]={27,0,0,27},wx[4]={1,6,7,2},wy[4]={26,1,1,26};
  for(unsigned i=0;i<3;++i){indices[i]=4;for(unsigned j=0;j<4;++j)if(fabsf(v[i]->x-VertexSnap-left-(white?wx[j]:dx[j]))<.01f && fabsf(v[i]->y-VertexSnap-top-(white?wy[j]:dy[j]))<.01f){indices[i]=j;break;}if(indices[i]>3){valid=0;break;}corners|=1u<<indices[i];}
  if(!valid || (corners!=7 && corners!=11 && corners!=13 && corners!=14))continue;
  if(white)return 2;
  for(unsigned i=0;i<3;++i){unsigned j=indices[i];int right=j>=2,bottom=j==0 || j==3;output[i]=*v[i];output[i].x=VertexSnap+left-.5f+right*9;output[i].y=VertexSnap+top-.5f+bottom*28;output[i].tmuvtx[0].sow=(212.5f+right*35)*.25f;output[i].tmuvtx[0].tow=(188.5f+bottom*111)*.25f;}
  return 1;
 }return 0;
}
