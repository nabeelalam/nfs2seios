// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <SDL3/SDL.h>
#include "AssetSelection.h"
#include "AssetImport.h"
#include <stdexcept>
@interface GameFolderPicker : NSObject <UIDocumentPickerDelegate>
@property(nonatomic) BOOL done;
@property(nonatomic) BOOL importing;
@property(nonatomic,copy) NSString *root;
@property(nonatomic,copy) NSString *error;
@property(nonatomic,strong) NSURL *destination;
@end
@implementation GameFolderPicker
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)picker {self.done=YES;}
- (void)documentPicker:(UIDocumentPickerViewController *)picker didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSURL *source=urls.firstObject;if(!source){self.done=YES;return;}
    self.importing=YES;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0),^{
        BOOL scoped=[source startAccessingSecurityScopedResource];
        __block NSError *failure=nil;
        NSFileCoordinator *coordinator=[[NSFileCoordinator alloc] initWithFilePresenter:nil];
        [coordinator coordinateReadingItemAtURL:source options:NSFileCoordinatorReadingWithoutChanges error:&failure byAccessor:^(NSURL *root){
            try{nfs::import_assets(root.path.UTF8String,NSBundle.mainBundle.resourcePath.UTF8String,self.destination.path.UTF8String);}
            catch(const std::exception &error){failure=[NSError errorWithDomain:@"NFSImport" code:1 userInfo:@{NSLocalizedDescriptionKey:[NSString stringWithUTF8String:error.what()] ?: @"Game import failed."}];}
        }];
        if(scoped)[source stopAccessingSecurityScopedResource];
        dispatch_async(dispatch_get_main_queue(),^{self.error=failure.localizedDescription;self.root=failure?nil:self.destination.path;self.importing=NO;self.done=YES;});
    });
}
@end
namespace nfs {
std::string app_log_path(int,char **){
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    return [documents URLByAppendingPathComponent:@"native-port.log"].path.UTF8String;
}
std::string select_app_assets(int,char **){
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL *destination=[documents URLByAppendingPathComponent:@"game" isDirectory:YES];
    BOOL directory=NO;
    if([NSFileManager.defaultManager fileExistsAtPath:destination.path isDirectory:&directory]){
        if(!directory)throw std::runtime_error("Documents/game must be a folder");
        return destination.path.UTF8String;
    }
    SDL_Window *window=SDL_CreateWindow("Select original game folder",640,480,0);
    if(!window)throw std::runtime_error(SDL_GetError());
    @try {
        UIWindow *native=(__bridge UIWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER,nullptr);
        if(!native.rootViewController)throw std::runtime_error("Could not open Files from the game window");
        GameFolderPicker *selection=[[GameFolderPicker alloc] init];selection.destination=destination;
        UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeFolder] asCopy:NO];
        picker.delegate=selection;picker.allowsMultipleSelection=NO;
        [native.rootViewController presentViewController:picker animated:YES completion:nil];
        // SDL's iOS event pump runs UIKit and main-queue callbacks. Never block
        // this thread on the import worker; retain selection until it completes.
        bool quitting=false;
        while(!selection.done){
            SDL_Event event;while(SDL_PollEvent(&event))if(event.type==SDL_EVENT_QUIT){quitting=true;if(!selection.importing){[picker dismissViewControllerAnimated:NO completion:nil];selection.done=YES;}}
            SDL_Delay(10);
        }
        if(selection.error)throw std::runtime_error(selection.error.UTF8String);
        return quitting || !selection.root ? std::string():std::string(selection.root.UTF8String);
    } @finally {SDL_DestroyWindow(window);}
}
}
