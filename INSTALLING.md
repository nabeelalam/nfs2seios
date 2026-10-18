# Personal installation without a Mac

Build 17 is an independently inspected thin ARM64 iOS-device Release package.
Personal SideStore signing is still required. Original game files stay separate.

## Update the existing game

1. Download the [build 17 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-39093193712-1/NFSIISEGame-development-unsigned.zip) in Files and extract it.
2. Update the existing app through your ready SideStore setup, importing
   NFSIISEGame-development-unsigned.ipa with the same Apple Account. SideStore
   performs personal signing; the ZIP itself is unsigned.
3. Keep using the same separately stored original game folder. Build 12's
   automatic saved-folder startup is retained.
4. Diagnostics start hidden. Tap with four fingers to show or hide them. Tap
   the FPS label for rendering settings. The small keyboard button is independent.
5. Keep **Lock FPS to 30 off** while testing normal 60 FPS and Low Power Mode.
   Compare the same track, opponents, render size and graphics. The overlay
   shows Low Power Mode state. A few minutes at one battery percentage does
   not establish energy savings; controlled longer comparisons remain needed.
6. The optional 30 FPS switch is below diagnostics and is remembered. Check
   the counter and normal game-clock/car behavior when enabled, then turn it
   off to return to the normal rendering path.
7. For the reported freeze, test single-player Tournament, white McLaren F1,
   Proving Grounds and all opponents. If it freezes, leave the app open for
   at least 15 seconds before closing. Look for NFSIISE-stall.txt in the app's
   folder in Files and share that report. A report is best effort; long loading
   can also trigger it, and its presence alone does not establish a cause.

The app requests fullscreen and opts into Game Mode. To filter notifications,
configure [Gaming Focus](https://support.apple.com/en-us/105118) on the iPad.
The app cannot force all system notifications off.

## Input and rendering controls

One finger moves the cursor and taps to click; two fingers send Escape, three
send Enter, and four toggle diagnostics without sending game keys. The keyboard
button shows/hides typing input. Native controller remapping remains in the
original controls settings; a held resume press should not also shift down.

Pause before changing render size. Settings offer 640x480, 960x720, 1280x960
and native screen pixels in the original 4:3 view; the overlay shows actual
render size. CPU/app is the share of total CPU capacity, so one busy core can
limit performance despite a low percentage. GPU utilization is not estimated.
Physical build15 acceptance, battery savings and HD artwork are unverified.

## Separately supplied game files

The current public IPA contains only the executable, app metadata and license
notices. It excludes GAMEDATA, FEDATA, install.win and all text.* language files.
For a folder import, supply install.win and text.eng beside GAMEDATA/FEDATA
in the selected folder before opening it. Other text.* languages are optional.
Build3 had a confirmed import regression; replace it with the current build. Existing
app-owned game folders remain reusable without importing them again.

For the supplied-startup importer, the local helper
`python scripts/package_private_startup.py` prepares a private startup ZIP under
ignored .build storage after verifying the already fetched pinned engine source.
Keep this ZIP private. Extract its startup files into a separate copy of your own
game folder alongside GAMEDATA/FEDATA and retain the license notice. Transfer
that folder through your private Files storage. The helper does not alter your
original game folder, fetch game assets or upload files.
Do not delete that folder or transfer it into the public repository.

Build7 accepts a selected folder containing GAMEDATA, FEDATA,
install.win and text.eng, with optional other text.* languages. Startup files
must be compatible with the translated engine; an original CD root may not
contain them. This import path passed local host and Apple build checks;
the same importer launched build4 on the target iPad. Build7 device acceptance
is pending. The Windows executable is not used. The game's
native-port.log is written into its app Documents folder for failure diagnosis.

## Signing and refresh

SideStore is an installation option and is not a runtime dependency of the app.
Its [official FAQ](https://docs.sidestore.io/docs/faq) documents the normal
seven-day signing period, three simultaneous apps including SideStore, and ten
App IDs per week for a free Apple Account. Refresh before expiration using your
configured SideStore/VPN setup. Refresh of this particular game remains unverified.
Updating with the same identity preserves the existing app data according to
SideStore's instructions; do not uninstall first.

For initial setup, follow SideStore's [prerequisites](https://docs.sidestore.io/docs/installation/prerequisites)
and [installation guide](https://docs.sidestore.io/docs/installation/install).
A computer is needed for initial setup; a local Mac is not required for the
project's Windows/SideStore path. Enter credentials only into the signing tool.
Never send Apple Account credentials, pairing files, signing certificates or
private keys to chat, Git or cloud builds. No paid service or membership is
required by the tested installation route.

Compiling, packaging, signing, installing, launching and completing a race are
separate acceptance steps. A valid unsigned IPA does not prove installation,
physical performance or stability.


Build 12 adds a keyboard icon below the FPS display: tap it to
show or hide the keyboard, including Return for opening the original control
remapping prompt. Map both players separately in Split Screen setup. Controller
input remains native; player one's Back/View resets the car. One-finger movement
acts as a relative trackpad. A one-finger tap clicks, a two-finger tap sends
Escape, and a three-finger tap sends Enter. Build12 physical feedback accepted touch tracking and gestures. Current
controller/typing regression still needs device validation. Preserve imported assets and saved control profiles
when updating; remap only if the existing profile needs it.


Build15 detailed diagnostics show Game CPU ms/frame after60 submissions.
Use the FPS label to select detailed diagnostics if compact view is enabled.
After switching Low Power Mode, allow a fresh60-submission interval before
recording both FPS and Game CPU in comparable busy and clear sections. This
is game-thread CPU time, not GPU use, complete frame latency or energy use.
The worker-error cleanup change is not a confirmed fix for the Tournament freeze.


Build 16 retains the same app identity and existing imported startup behavior.
It adds sound-operation ownership tested locally with original-code contracts,
Tournament collision playback and optimized split-screen controls. Update the
existing app through SideStore with the same Apple Account. Whether this resolves the reported
iPad Tournament freeze remains unverified; please finish at least one race in that
scenario and report any freeze, error or stall report. For the performance target,
use the same full-graphics Native-resolution split race in normal/Low Power Mode,
Lock FPS to 30 off, and allow 60 submissions before recording Game CPU with FPS.
No battery or 60 FPS in Low Power Mode improvement is claimed by the sound change.


Build 17 retains the same app identity, imported folder access and controls.
It adds the checked-copy bundle validated under local original-data sanitizer
and Release runs. Update with the same Apple Account through SideStore.
Compare the same Native-resolution/full-graphics split-screen race in normal
and Low Power Mode, with Lock FPS to 30 off. Allow sixty submissions for the
Game CPU average. Physical improvement is unverified; report FPS and Game CPU
in comparable busy and clear sections and any freeze or incorrect behavior.
