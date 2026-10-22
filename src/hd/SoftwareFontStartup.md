# Authored software-font startup

The startup path `activateConfiguredArtwork()` builds the compiled authored font
before the game session starts its worker. Its installation is independent of
optional artwork-pack success: missing, malformed or sealed font state does not
change background/native-mark installation, artwork return values or originals.
The worker acquires an immutable shared catalog; it never borrows decoder storage.

`scripts/author_software_fonts.py --check` regenerates and compares the two font
roles using only the pinned project font files and SIL OFL licenses. The isolated
Fontconfig setup rejects substitution and changes no system settings. Authoring
uses Pango 1.52.1/Cairo, disabled hinting, fractional positions and explicit pair
kerning; ligatures/context substitutions are disabled. Those authoring libraries
are not app/runtime dependencies. The checked-in data contains authored glyph
metrics, kerning and bounded R8 coverage, with no original-game pixels. The source
font URLs, license pins, recipe and generated content pins are recorded in
`assets/hud/fonts/software-fonts.json`; existing app packaging includes both OFLs.

The built-in loader validates exact metadata and coverage SHA-256 values before
`FontCatalog::create()` copies and validates the atlas. Numeric identity is not a
provenance proof. Metadata serialization is explicitly little endian without
host padding. Decoding checks every run and exact 1024 by 512 total before
changing output. Installation retains the catalog, and the first successful
acquisition freezes replacement. An unsuccessful empty acquisition permits later
startup installation. Producers must retain the owner and supply independent
original-call/target/lifetime proof; this startup path does not activate drawing.
