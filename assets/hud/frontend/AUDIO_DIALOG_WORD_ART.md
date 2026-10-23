# Audio dialog body artwork

Eight fixed whole-word graphics reconstruct the English Audio body labels in the accepted lighter Haettenschweiler lettering. They use the original measured horizontal allocations and right edge 368, baseline 24, fourfold raster dimensions, and transparent padding on every edge. The letter shapes use the same 0.88 horizontal treatment and half logical pixel stroke inset as the accepted compact captions. Only words exceeding their original allocation receive an additional horizontal fit; original geometry is unchanged.

The source contains white RGB with alpha so original RGB565 shadow, disabled, selected and normal colors can be applied separately. It contains no font file, alphabet or original game pixels. These eight images do not substitute a general font. Dynamic values, title and buttons remain separate.

`generate_audio_dialog_words.py` authenticates the complete source file, exact finite inventory and all decoded image digests before writing either generated header. Runtime decoding independently checks bounded RLE, dimensions, full RGBA identity and all four padding edges. Decoder results have immutable shared ownership.

The eight body labels are active through independently measured original resources and the complete original Audio text packet. Sixteen ordered shadow/foreground spans replace only their original glyph draws; dynamic values, title, buttons and bars retain their original draw order. Live source authority is checked before queueing, and immutable artwork and text owners remain retained through rendering.

Local validation passes 21 checks in each optimized and sanitizer mode, plus original-game navigation and 104 captured label comparisons. This is desktop visual and ownership validation; physical iPad appearance, performance and energy use remain unverified. See evidence/hd-audio-dialog-word-display.json for bounded coverage and known gaps.
