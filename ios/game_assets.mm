// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <SDL3/SDL.h>
#include "AssetSelection.h"
#include "AssetImport.h"
#include "RunLog.h"
#include <cerrno>
#include <cstdio>
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
    NSURL *current=[documents URLByAppendingPathComponent:@"native-port.log"];
    NSURL *previous=[documents URLByAppendingPathComponent:@"native-port-previous.log"];
    const char *currentPath=current.fileSystemRepresentation;
    const char *previousPath=previous.fileSystemRepresentation;
    if(!currentPath || !previousPath)return {};
    // Keep the interrupted session available after one relaunch. A failed
    // rotation must never cause open_app_log to truncate the current file.
    if(std::rename(currentPath,previousPath)!=0 && errno!=ENOENT){
        std::fprintf(stderr,"Could not preserve the previous native app log\n");
        return {};
    }
    return currentPath;
}
std::string select_app_assets(int,char **){
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL *destination=[documents URLByAppendingPathComponent:@"game" isDirectory:YES];
    NSString *existingError=nil;
    if([NSFileManager.defaultManager fileExistsAtPath:destination.path]){
        try{
            validate_imported_assets(destination.path.UTF8String);
            return destination.path.UTF8String;
        }catch(const std::exception &error){
            NSString *reason=[NSString stringWithUTF8String:error.what()] ?: @"Could not read the imported game.";
            existingError=[reason stringByAppendingString:@" In Files, rename the game folder inside this app to keep it as a backup, then reopen the app and select your original game folder."];
        }
    }
    SDL_Window *window=SDL_CreateWindow("Select original game folder",640,480,0);
    if(!window)throw std::runtime_error(SDL_GetError());
    @try {
        UIWindow *native=(__bridge UIWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER,nullptr);
        if(!native.rootViewController)throw std::runtime_error("Could not open Files from the game window");
        GameFolderPicker *selection=[[GameFolderPicker alloc] init];selection.destination=destination;
        UIDocumentPickerViewController *picker=[[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeFolder] asCopy:NO];
        picker.delegate=selection;picker.allowsMultipleSelection=NO;
        if(existingError){
            selection.error=existingError;
            selection.done=YES;
        }else{
            [native.rootViewController presentViewController:picker animated:YES completion:nil];
        }
        // SDL's iOS event pump runs UIKit and main-queue callbacks. Never block
        // this thread on the import worker; retain selection until it completes.
        bool quitting=false;
        while(!selection.done){
            SDL_Event event;while(SDL_PollEvent(&event))if(event.type==SDL_EVENT_QUIT){quitting=true;if(!selection.importing){[picker dismissViewControllerAnimated:NO completion:nil];selection.done=YES;}}
            SDL_Delay(10);
        }
        if(selection.error){
            nfs::app_log("Game import failed",selection.error.UTF8String);
            // Keep the picker window alive until the user sees the error.
            // Reporting after destroying the last UIKit window can hide it.
            __block BOOL acknowledged=NO;
            UIAlertController *alert=[UIAlertController alertControllerWithTitle:(existingError ? @"Could not open imported game" : @"Could not import game")
                message:selection.error preferredStyle:UIAlertControllerStyleAlert];
            [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                handler:^(UIAlertAction *){acknowledged=YES;}]];
            void (^showError)(void)=^{[native.rootViewController presentViewController:alert animated:YES completion:nil];};
            if(native.rootViewController.presentedViewController)
                [native.rootViewController dismissViewControllerAnimated:NO completion:showError];
            else showError();
            while(!acknowledged){SDL_PumpEvents();SDL_Delay(10);}
            return std::string();
        }
        return quitting || !selection.root ? std::string():std::string(selection.root.UTF8String);
    } @finally {SDL_DestroyWindow(window);}
}
}

#ifdef NFS_CLASSIC_BUNDLED_ART
namespace nfs {std::string app_artwork_template_path(){NSString*path=[NSBundle.mainBundle pathForResource:@"classic-ui" ofType:@"ownedz"];return path?std::string(path.UTF8String):std::string();}}
#endif
