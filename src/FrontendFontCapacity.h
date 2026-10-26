// SPDX-License-Identifier: MIT
#ifndef NFS_FRONTEND_FONT_CAPACITY_H
#define NFS_FRONTEND_FONT_CAPACITY_H
/* The immutable legacy2048-row prefix remains useful on older GL limits. */
static unsigned nfs_front_atlas_upload_height(unsigned limit,unsigned full_height){
 if(full_height<2048 || full_height>2304 || limit<2048 || limit<1024)return 0;
 return limit>=full_height?full_height:2048;
}
static int nfs_front_atlas_row_fits(float atlas_x,float atlas_y,float width,float height,
                                   unsigned uploaded_height,unsigned full_height){
 if(uploaded_height!=2048 && uploaded_height!=full_height)return 0;
 if(full_height<2048 || full_height>2304)return 0;
 /* Ordered comparisons reject NaN and infinity as well as invalid extents. */
 return atlas_x>=0 && width>0 && (atlas_x+width)*4<=1024
     && atlas_y>=0 && height>0 && atlas_y*8+height*4<=(float)uploaded_height;
}
static float nfs_front_atlas_tow(float legacy_tow,unsigned uploaded_height){
 return legacy_tow*(2048.0f/(float)uploaded_height);
}
#endif
