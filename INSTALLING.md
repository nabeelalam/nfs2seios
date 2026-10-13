# Personal installation and original assets

There is no current public release IPA. When a reviewed build is deliberately
uploaded, download its unsigned development package from this repository's
Actions page. Verify its published validation metadata and checksum.

An unsigned IPA needs personal signing before installation. Use your own ready
SideStore setup and its documented import/refresh procedure. Never upload Apple
credentials, pairing information, certificates or provisioning profiles here.
See the [SideStore documentation](https://docs.sidestore.io/) for setup details.

Launch the Game app and use its Files import to select separately supplied
original assets containing GAMEDATA and FEDATA. The main game data and original
Windows executable are not supplied by this repository; the executable is not
the iPad runtime. The importer validates and copies assets into owned app storage.

The Probe is a separate platform diagnostic and does not execute the original
game. A successful Probe installation cannot substitute for game acceptance.
Report installation or runtime errors with private values and personal file
paths removed. Never attach original game files or signing material to an issue.
