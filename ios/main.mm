// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import <GameController/GameController.h>
#import <MetalKit/MetalKit.h>
#import <AVFoundation/AVFoundation.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <cmath>
#include "AssetImport.h"
#include <exception>
extern "C" bool nfs_core_probe();

// Platform diagnostic only. Never call the low-address Linux engine on iPadOS.
@interface ProbeView : UIViewController <MTKViewDelegate, UIDocumentPickerDelegate>
@property(nonatomic, strong) MTKView *gpu;
@property(nonatomic, strong) id<MTLCommandQueue> queue;
@property(nonatomic, strong) UILabel *status;
@property(nonatomic, strong) UILabel *input;
@property(nonatomic, strong) NSTimer *timer;
@property(nonatomic, strong) AVAudioEngine *audio;
@property(nonatomic, strong) AVAudioPlayerNode *player;
@property(nonatomic, copy) NSString *assetStatus;
@property(nonatomic) BOOL aPressed;
@property(nonatomic) BOOL bPressed;
@property(nonatomic) BOOL paused;
@property(nonatomic) BOOL importing;
@property(nonatomic) BOOL importPressed;
@end

@implementation ProbeView
- (void)viewDidLoad {
    [super viewDidLoad];
    self.gpu = [[MTKView alloc] initWithFrame:self.view.bounds device:MTLCreateSystemDefaultDevice()];
    self.gpu.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.gpu.delegate = self;
    self.gpu.preferredFramesPerSecond = 30;
    self.queue = [self.gpu.device newCommandQueue];
    [self.view addSubview:self.gpu];
    UIStackView *stack = [[UIStackView alloc] init];
    stack.axis = UILayoutConstraintAxisVertical;
    stack.spacing = 18;
    stack.translatesAutoresizingMaskIntoConstraints = NO;
    self.status = [[UILabel alloc] init];
    self.status.textColor = UIColor.whiteColor;
    self.status.numberOfLines = 0;
    self.status.font = [UIFont systemFontOfSize:22 weight:UIFontWeightSemibold];
    self.input = [[UILabel alloc] init];
    self.input.numberOfLines = 0;
    self.input.textColor = UIColor.whiteColor;
    self.input.font = [UIFont monospacedSystemFontOfSize:16 weight:UIFontWeightRegular];
    UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
    [button setTitle:@"Import original game folder from Files" forState:UIControlStateNormal];
    [button addTarget:self action:@selector(importFolder) forControlEvents:UIControlEventTouchUpInside];
    [stack addArrangedSubview:self.status];
    [stack addArrangedSubview:self.input];
    [stack addArrangedSubview:button];
    [self.view addSubview:stack];
    [NSLayoutConstraint activateConstraints:@[
        [stack.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:32],
        [stack.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-32],
        [stack.centerYAnchor constraintEqualToAnchor:self.view.centerYAnchor]]];
    self.assetStatus = @"Game assets have not been imported. This build does not run the game.";
    self.audio = [[AVAudioEngine alloc] init];
    self.player = [[AVAudioPlayerNode alloc] init];
    [self.audio attachNode:self.player];
    AVAudioFormat *format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100 channels:1];
    [self.audio connect:self.player to:self.audio.mainMixerNode format:format];
    NSNotificationCenter *nc = NSNotificationCenter.defaultCenter;
    [nc addObserver:self selector:@selector(suspend:) name:UIApplicationWillResignActiveNotification object:nil];
    [nc addObserver:self selector:@selector(resume:) name:UIApplicationDidBecomeActiveNotification object:nil];
    [nc addObserver:self selector:@selector(interrupted:) name:AVAudioSessionInterruptionNotification object:nil];
    [self resume:nil];
}
- (void)dealloc { [NSNotificationCenter.defaultCenter removeObserver:self]; [self.timer invalidate]; }
- (BOOL)prefersStatusBarHidden { return YES; }
- (void)refreshStatus {
    self.status.text = [NSString stringWithFormat:@"NFS II SE — native platform probe\nMemory test: %@ · Metal device: %@\n%@",
        nfs_core_probe() ? @"PASS" : @"FAIL", self.gpu.device.name ?: @"unavailable", self.assetStatus];
}
- (void)suspend:(NSNotification *)note {
    [self.timer invalidate]; self.timer = nil;
    self.gpu.paused = YES;
    [self.player stop]; [self.audio pause];
    [AVAudioSession.sharedInstance setActive:NO error:nil];
    self.aPressed = self.bPressed = self.importPressed = NO;
}
- (void)resume:(NSNotification *)note {
    self.gpu.paused = NO;
    if (!self.timer) self.timer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(poll:) userInfo:nil repeats:YES];
    [self refreshStatus];
}
- (void)interrupted:(NSNotification *)note {
    NSNumber *type = note.userInfo[AVAudioSessionInterruptionTypeKey];
    if (type.unsignedIntegerValue == AVAudioSessionInterruptionTypeBegan) [self.audio pause];
    // A fresh A press starts audio after the interruption ends.
}
- (void)tone {
    NSError *error = nil;
    [AVAudioSession.sharedInstance setCategory:AVAudioSessionCategoryPlayback error:&error];
    if (!error) [AVAudioSession.sharedInstance setActive:YES error:&error];
    if (!error && !self.audio.running) [self.audio startAndReturnError:&error];
    if (error) { self.assetStatus = [@"Audio error: " stringByAppendingString:error.localizedDescription]; [self refreshStatus]; return; }
    AVAudioFormat *format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100 channels:1];
    AVAudioPCMBuffer *buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:8820];
    buffer.frameLength = 8820;
    for (unsigned i = 0; i < buffer.frameLength; ++i) {
        double envelope = std::fmin(1.0, std::fmin(i / 220.0, (8819-i) / 220.0));
        buffer.floatChannelData[0][i] = float(0.1 * envelope * std::sin(2 * 3.141592653589793 * 440 * i / 44100));
    }
    [self.player scheduleBuffer:buffer completionHandler:nil];
    [self.player play];
}
- (void)poll:(NSTimer *)timer {
    GCExtendedGamepad *g = nil;
    GCController *controller = nil;
    for (GCController *candidate in GCController.controllers) if (candidate.extendedGamepad) { controller = candidate; g = candidate.extendedGamepad; break; }
    if (!g) {
        self.input.text = @"Connect a Bluetooth controller.\nA: audio test · B: pause/resume · Y: Files import\nNo game engine is running in this diagnostic.";
        self.aPressed = self.bPressed = self.importPressed = NO;
        return;
    }
    if (g.buttonA.pressed && !self.aPressed) [self tone];
    if (g.buttonB.pressed && !self.bPressed) { self.paused = !self.paused; self.gpu.paused = self.paused; }
    if (g.buttonY.pressed && !self.importPressed) [self importFolder];
    self.aPressed = g.buttonA.pressed; self.bPressed = g.buttonB.pressed; self.importPressed = g.buttonY.pressed;
    self.input.text = [NSString stringWithFormat:@"%@\nSteer %+.2f · accelerate %.2f · brake %.2f\nD-pad %+.0f,%+.0f · gears LB:%d RB:%d · camera X:%d · menu:%d\nA: audio test · B: pause/resume · Y: import assets",
        controller.vendorName ?: @"Controller", g.leftThumbstick.xAxis.value, g.rightTrigger.value, g.leftTrigger.value,
        g.dpad.xAxis.value, g.dpad.yAxis.value, g.leftShoulder.pressed, g.rightShoulder.pressed,
        g.buttonX.pressed, g.buttonMenu.pressed];
}
- (void)drawInMTKView:(MTKView *)view {
    view.clearColor = MTLClearColorMake(0.025, 0.045, 0.08, 1);
    MTLRenderPassDescriptor *pass = view.currentRenderPassDescriptor;
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!pass || !drawable || !self.queue) return;
    id<MTLCommandBuffer> command = [self.queue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [command renderCommandEncoderWithDescriptor:pass];
    [encoder endEncoding]; [command presentDrawable:drawable]; [command commit];
}
- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {}
- (void)importFolder {
    if (self.importing || self.presentedViewController) return;
    UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes:@[UTTypeFolder] asCopy:NO];
    picker.delegate = self;
    picker.allowsMultipleSelection = NO;
    [self presentViewController:picker animated:YES completion:nil];
}
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSURL *source = urls.firstObject;
    if (!source) return;
    self.importing = YES;
    self.assetStatus = @"Importing into app storage…"; [self refreshStatus];
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        BOOL scoped = [source startAccessingSecurityScopedResource];
        __block NSError *failure = nil;
        NSFileManager *fm = NSFileManager.defaultManager;
        NSURL *documents = [fm URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
        NSURL *destination = [documents URLByAppendingPathComponent:@"game" isDirectory:YES];
        NSFileCoordinator *coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
        [coordinator coordinateReadingItemAtURL:source options:NSFileCoordinatorReadingWithoutChanges error:&failure byAccessor:^(NSURL *root) {
            try {
                nfs::import_assets(root.path.UTF8String, NSBundle.mainBundle.resourcePath.UTF8String, destination.path.UTF8String);
            } catch (const std::exception &error) {
                NSString *reason = [NSString stringWithUTF8String:error.what()] ?: @"Game import failed.";
                failure = [NSError errorWithDomain:@"NFSImport" code:1 userInfo:@{NSLocalizedDescriptionKey:reason}];
            }
        }];
        if (scoped) [source stopAccessingSecurityScopedResource];
        NSString *result = failure ? failure.localizedDescription : @"Game data and startup support files imported into Documents/game. This diagnostic does not run the game.";
        dispatch_async(dispatch_get_main_queue(), ^{ self.importing = NO; self.assetStatus = result; [self refreshStatus]; });
    });
}
@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation AppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)options {
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController = [[ProbeView alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
}
@end
int main(int argc, char **argv) {
    @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(AppDelegate.class)); }
}
