/* SPDX-License-Identifier: MIT
 * GLES surface transfer helpers included in the prepared original backend.
 */
typedef struct {GLint viewport[4],scissor[4],texture,pack,unpack;} LfbGLState;
static void lfbSave(LfbGLState *state){
    glGetIntegerv(GL_VIEWPORT,state->viewport);glGetIntegerv(GL_SCISSOR_BOX,state->scissor);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&state->texture);
    glGetIntegerv(GL_PACK_ALIGNMENT,&state->pack);glGetIntegerv(GL_UNPACK_ALIGNMENT,&state->unpack);
}
static void lfbRestore(const LfbGLState *state){
    glUseProgram(g_shaderProgramDisp);glUniform1f(g_uGammaLocDisp,g_gammaValue);
    useGameProgram(true);glBindTexture(GL_TEXTURE_2D,state->texture);
    glViewport(state->viewport[0],state->viewport[1],state->viewport[2],state->viewport[3]);
    glScissor(state->scissor[0],state->scissor[1],state->scissor[2],state->scissor[3]);
    glPixelStorei(GL_PACK_ALIGNMENT,state->pack);glPixelStorei(GL_UNPACK_ALIGNMENT,state->unpack);
}
static void lfbTextureParameters(void){
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}
static void lfbDraw(GLuint target,GLuint texture,int width,int height,BOOL upperOrigin){
    const GLfloat upper[4][2]={{0,0},{0,1},{1,0},{1,1}};
    useGameProgram(false);glBindFramebuffer(GL_FRAMEBUFFER,target);
    glViewport(0,0,width,height);glScissor(0,0,width,height);
    glUniform1f(g_uGammaLocDisp,1.0f);
    glVertexAttribPointer(g_aTexCoordLocDisp,2,GL_FLOAT,GL_FALSE,0,upperOrigin?upper:g_textureCoordDisp);
    glBindTexture(GL_TEXTURE_2D,texture);glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}
static BOOL lfbRead(GrBuffer_t buffer,GrOriginLocation_t origin,void *pixels){
    if(!g_glCtx || !g_framebufferHeight)return false;
    drawTriangles();
    uint8_t *rgba=(uint8_t *)malloc(NFS_SURFACE_WIDTH*NFS_SURFACE_HEIGHT*4);
    if(!rgba)return false;
    LfbGLState state;lfbSave(&state);
    GLuint texture=0,target=0;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
    lfbTextureParameters();glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,NFS_SURFACE_WIDTH,NFS_SURFACE_HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glGenFramebuffers(1,&target);glBindFramebuffer(GL_FRAMEBUFFER,target);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    BOOL ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    if(ok){
        lfbDraw(target,buffer==GR_BUFFER_FRONTBUFFER?g_frontTexture:g_framebufferTexture,NFS_SURFACE_WIDTH,NFS_SURFACE_HEIGHT,false);
        glPixelStorei(GL_PACK_ALIGNMENT,4);glReadPixels(0,0,NFS_SURFACE_WIDTH,NFS_SURFACE_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,rgba);
        ok=glGetError()==GL_NO_ERROR;
    }
    if(ok)for(int y=0;y<NFS_SURFACE_HEIGHT;++y)for(int x=0;x<NFS_SURFACE_WIDTH;++x){
        const uint8_t *c=rgba+((size_t)y*NFS_SURFACE_WIDTH+x)*4;
        const uint16_t color=(uint16_t)(((c[0]>>3)<<11)|((c[1]>>2)<<5)|(c[2]>>3));
        const int row=origin==GR_ORIGIN_UPPER_LEFT?NFS_SURFACE_HEIGHT-1-y:y;
        memcpy((uint8_t *)pixels+(size_t)row*NFS_SURFACE_STRIDE+x*2,&color,2);
    }
    lfbRestore(&state);glDeleteFramebuffers(1,&target);glDeleteTextures(1,&texture);free(rgba);return ok;
}
static BOOL lfbWrite(GrBuffer_t buffer,GrOriginLocation_t origin,const void *pixels){
    if(!g_glCtx || !g_framebufferHeight)return false;
    drawTriangles();LfbGLState state;lfbSave(&state);
    GLuint texture=0;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);lfbTextureParameters();
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,NFS_SURFACE_WIDTH,NFS_SURFACE_HEIGHT,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,pixels);
    lfbDraw(buffer==GR_BUFFER_FRONTBUFFER?g_frontFramebuffer:g_framebuffer,texture,g_framebufferHeight*4/3,g_framebufferHeight,origin==GR_ORIGIN_UPPER_LEFT);
    BOOL ok=glGetError()==GL_NO_ERROR;
    if(ok && buffer==GR_BUFFER_FRONTBUFFER)ok=presentFrontSurface();
    lfbRestore(&state);glDeleteTextures(1,&texture);return ok;
}
