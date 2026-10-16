# Personal installation without a Mac

Native game builds have been personally signed and launched on the target iPad
through the existing SideStore setup. Build 10 is the user-accepted split-screen
60 FPS baseline. Build 12 is the new controller, keyboard and touch candidate:
its ARM64 Release package passed independent source, architecture, contents,
privacy and digest checks. Its physical input/FPS regression remains pending.

## Update the existing game

1. Download the [build 12 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-38980392788-1/NFSIISEGame-development-unsigned.zip)
   in Files and extract it.
2. Keep the currently installed NFS II SE app and your separately stored game
   folder in Files. Install this update over the existing app.
3. Enable the local VPN required by your existing SideStore setup. Import
   NFSIISEGame-development-unsigned.ipa into SideStore using the same Apple
   Account that signed the existing game. SideStore performs personal signing
   and installation; the ZIP itself is not a signed installation.
4. Select Split Screen in the original Setup menu. Use Native resolution and
   the same full graphics, track, cars and opponents as your most recent comparison.
   Check displayed FPS in busy and clear race sections, complete a race, and
   report audio or stability failures. Check both saved controller profiles;
   remap only if needed in the original controls settings while Split Screen is active.

Build 12 retains the accepted rendering and memory-check optimizations, ThinLTO,
diagnostics, selectable render sizes and importer. It adds native controller input
ownership, the keyboard icon and relative touch gestures. Check that a held
resume press does not also downshift, while a released fresh press does. Test
one-finger cursor movement/tap click, two-finger Escape (including race Pause),
and three-finger Enter. The keyboard icon shows/hides the keyboard and can supply
Return for original control remapping. Automatic text-field detection is deferred.
No battery saving or HD artwork improvement is claimed. Build 10 remains the
physically accepted comparison baseline. Background recovery, audio interruptions
and signing refresh still need physical acceptance.
Import errors are displayed before the picker window is closed. The release tag
and manifest identify its source revision.

## Performance controls

Tap the FPS overlay to select 640x480, 960x720, 1280x960 or native screen pixels.
Pause with the controller before changing the setting. Native retains the
original 4:3 view; side borders and original bitmap artwork remain. The overlay
shows actual render size, so this is distinct from merely stretching640x480.
The menu also switches between detailed diagnostics and compact FPS display.
Choices are remembered for the next launch.

CPU/app is this game's share of total CPU capacity; RAM is its resident memory.
A busy single core can limit performance even when CPU/app is low.
The millisecond value is the averaged interval between presentations. GPU usage
is unavailable with this renderer, and is not estimated from the FPS figure.

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
Escape, and a three-finger tap sends Enter. Physical iPad validation of these
new controls remains pending. Preserve imported assets and saved control profiles
when updating; remap only if the existing profile needs it.
