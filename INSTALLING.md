# Personal installation without a Mac

Native game builds have been personally signed and launched on the target iPad
through the user's existing SideStore setup. Earlier game versions rendered
original menus and races with audio and controller input, but racing performance
was below enjoyable play. Build3 is a verified unsigned development download;
its installation, frame rate and complete-race acceptance on the iPad remain
pending. The separate Probe is a platform diagnostic, not the game.

## Update the existing game

1. Download the [build3 ZIP](https://github.com/nabeelalam/nfs2seios/releases/download/development-38599376607-1/NFSIISEGame-development-unsigned.zip)
   in Files and extract it.
2. Keep the currently installed NFS II SE app and its imported Documents/game
   folder. Do not delete the app to install this update.
3. Enable the local VPN required by your existing SideStore setup. Import
   NFSIISEGame-development-unsigned.ipa into SideStore using the same Apple
   Account that signed the existing game. SideStore performs personal signing
   and installation; the ZIP itself is not a signed installation.
4. Launch NFS II SE, connect the controller, and record the displayed menu and
   racing FPS. Check that menu directions select items without confirming them.
   A complete race and background/reopen check remain device acceptance items.

Build3 contains the renderer optimization and presentation FPS counter. Later
source changes to startup import and callback exception recovery are not included
in this exact download. The release tag and manifest identify its source revision.

## Separately supplied game files

The public build3 IPA contains only the executable, app metadata and license
notices. It excludes GAMEDATA, FEDATA, install.win and all text.* language files.
Build3 is an update for an existing imported game folder; it cannot perform a
fresh import without the startup support that earlier installations supplied.

For a future build with the supplied-startup importer, the local helper
`python scripts/package_private_startup.py` prepares a private startup ZIP under
ignored .build storage after verifying the already fetched pinned engine source.
Keep this ZIP private. Extract its startup files into a separate copy of your own
game folder alongside GAMEDATA/FEDATA and retain the license notice. Transfer
that folder through your private Files storage when a compatible app build is
available. The helper does not alter your original game folder, fetch game assets,
upload files or add fresh import support to build3.
Do not delete that folder or transfer it into the public repository.

Newer source also accepts a selected folder containing GAMEDATA, FEDATA,
install.win and text.eng, with optional other text.* languages. Startup files
must be compatible with the translated engine; an original CD root may not
contain them. This newer import path has local host tests only and is not in
build3 or physically validated. The Windows executable is not used. The game's
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
