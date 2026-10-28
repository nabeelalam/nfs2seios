// Private source-exact timing atlas replay controls. Unknown/clipped UVs retain original.
#pragma once
static int nfs_replay_controls_triangle(const float uv[6]){
 static const float box[3][4]={{168.5f,21.5f,187.f,42.f},{47.5f,216.5f,256.f,256.f},{9.5f,227.5f,15.f,231.f}};
 for(unsigned k=0;k<3;++k){unsigned corners=0;for(unsigned i=0;i<3;++i){float u=uv[2*i]*256/255,v=uv[2*i+1]*256/255;unsigned r=2,b=2;if(fabsf(u-box[k][0])<.01f)r=0;else if(fabsf(u-box[k][2])<.01f)r=1;if(fabsf(v-box[k][1])<.01f)b=0;else if(fabsf(v-box[k][3])<.01f)b=1;if(r>1||b>1){corners=0;break;}corners|=1u<<(r+2*b);}if(corners==7||corners==11||corners==13||corners==14)return 79+(int)k;}return 0;
}
