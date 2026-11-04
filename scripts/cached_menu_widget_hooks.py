"""Private staged source additions; no canonical generation or activation."""
def add_cached_menu_widget_hooks(text,name):
    if name!="Methods_03.cpp":return text
    insertions=(("423EA0","nfs_menu_cached_widget_source::ProducerScope nfs_widgets_producer(*this);"),
                ("423CA0","nfs_menu_cached_widget_source::consumer(*this,uint32_t(eax));"),
                ("423C40","nfs_menu_cached_widget_source::retire();"))
    for function,code in insertions:
        anchor="Fn(void) Game::_sub_"+function+"()\n{\n"
        if text.count(anchor)!=1:raise ValueError("Original measured widget entry changed: "+function)
        block="    /* NFS_CACHED_WIDGET_SOURCE_BEGIN:"+function+" */\n    "+code+"\n    /* NFS_CACHED_WIDGET_SOURCE_END:"+function+" */\n"
        text=text.replace(anchor,anchor+block)
    return '#include "CachedMenuWidgetSource.h"\n'+text
