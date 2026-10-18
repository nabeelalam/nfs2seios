"""Render reviewed vector paths through locally installed librsvg/Cairo; no font download."""
import ctypes as c
import ctypes.util
class Rectangle(c.Structure):_fields_=[(n,c.c_double) for n in ['x','y','width','height']]
def rasterize(svg,width,height):
    if not isinstance(svg,bytes) or len(svg)>1024*1024 or not 0<width<=2048 or not 0<height<=2048:raise ValueError('Bounded SVG dimensions required')
    cairo=c.CDLL(ctypes.util.find_library('cairo'));rsvg=c.CDLL(ctypes.util.find_library('rsvg-2'));gobject=c.CDLL(ctypes.util.find_library('gobject-2.0'))
    def bind(lib,name,args,result):
        f=getattr(lib,name);f.argtypes=args;f.restype=result;return f
    surface_new=bind(cairo,'cairo_image_surface_create',[c.c_int,c.c_int,c.c_int],c.c_void_p)
    surface_free=bind(cairo,'cairo_surface_destroy',[c.c_void_p],None)
    context_new=bind(cairo,'cairo_create',[c.c_void_p],c.c_void_p)
    context_free=bind(cairo,'cairo_destroy',[c.c_void_p],None)
    status=bind(cairo,'cairo_status',[c.c_void_p],c.c_int)
    flush=bind(cairo,'cairo_surface_flush',[c.c_void_p],None)
    pixels=bind(cairo,'cairo_image_surface_get_data',[c.c_void_p],c.POINTER(c.c_ubyte))
    stride=bind(cairo,'cairo_image_surface_get_stride',[c.c_void_p],c.c_int)
    load=bind(rsvg,'rsvg_handle_new_from_data',[c.c_void_p,c.c_size_t,c.c_void_p],c.c_void_p)
    draw=bind(rsvg,'rsvg_handle_render_document',[c.c_void_p,c.c_void_p,c.POINTER(Rectangle),c.c_void_p],c.c_int)
    unref=bind(gobject,'g_object_unref',[c.c_void_p],None)
    surface=surface_new(0,width,height);context=None;handle=None
    try:
        if not surface:raise ValueError('Surface unavailable')
        context=context_new(surface)
        if not context or status(context):raise ValueError('Cairo context unavailable')
        data=c.create_string_buffer(svg);handle=load(data,len(svg),None)
        if not handle or not draw(handle,context,c.byref(Rectangle(0,0,width,height)),None) or status(context):raise ValueError('SVG render failed')
        flush(surface);row_stride=stride(surface);raw=c.string_at(pixels(surface),row_stride*height)
        rgba=bytearray(width*height*4)
        for y in range(height):
            for x in range(width):
                b,g,r,a=raw[y*row_stride+x*4:y*row_stride+x*4+4]
                # Native Cairo ARGB32 is premultiplied BGRA on this LE host.
                rgba[(y*width+x)*4:(y*width+x+1)*4]=bytes((min(255,(r*255+a//2)//a) if a else 0,min(255,(g*255+a//2)//a) if a else 0,min(255,(b*255+a//2)//a) if a else 0,a))
        return bytes(rgba)
    finally:
        if handle:unref(handle)
        if context:context_free(context)
        if surface:surface_free(surface)
if __name__=='__main__':
    red=rasterize(b'<svg xmlns="http://www.w3.org/2000/svg" width="2" height="1"><path d="M0 0H1V1H0Z" fill="#ff0000" fill-opacity="0.5"/></svg>',2,1)
    assert red[:4] in (bytes([255,0,0,127]),bytes([255,0,0,128])) and red[4:]==b'\0'*4
    assert rasterize(b'<svg xmlns="http://www.w3.org/2000/svg" width="1" height="1"><path d="M0 0H1V1H0Z" fill="#123456"/></svg>',1,1)==bytes([18,52,86,255])
    try:rasterize(b'not SVG',1,1)
    except ValueError:pass
    else:raise AssertionError('Malformed SVG accepted')
    print('PASS native vector rendering, straight-alpha conversion and malformed input rejection')
