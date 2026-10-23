# Audio dialog whole-word artwork

Eleven fixed whole-word graphics reconstruct the eight English Audio body labels, DONE, CANCEL and AUDIO in the accepted lighter Haettenschweiler lettering. The first eight images and original source geometry retain their accepted byte identities. No Main menu width correction is applied to this batch.

All images retain original measured allocations, baseline 24, fourfold raster dimensions and transparent padding on every edge. DONE is right aligned within its original 35 pixel width, CANCEL is left aligned within 51 pixels, and AUDIO is centered within 41 pixels. Their destinations are respectively [333,369,368,401], [388,369,439,401] and [348,105,389,137]. The letters use the same 0.88 horizontal treatment and half logical pixel stroke inset as the accepted compact captions; none of the three new words requires an extra horizontal fit.

The source contains white RGB with alpha so original RGB565 shadow, disabled, selected, normal and title colors can be applied separately. It contains no font file, alphabet or original game pixels. These eleven images do not substitute a general font; dynamic values retain their original commands.

`generate_audio_dialog_words.py` authenticates the complete source file, exact finite inventory and all decoded image digests before writing either generated header. Runtime decoding independently checks bounded RLE, dimensions, full RGBA identity and all four padding edges. Decoder results have immutable shared ownership. Source pins and tests preserve the accepted first eight images independently of the new source-file digest.

All eleven words are active through independently verified original resources and the complete 48-command Audio text packet. Twenty-two ordered shadow/foreground spans preserve the intervening original dynamic text. DONE retains its actual right measure, CANCEL its unmeasured left policy and AUDIO its actual center measure. Current guest authority is reacquired independently of immutable artwork and packet retention.

Local validation passes 21 checks each in optimized and sanitizer modes, plus original-game navigation, activation of both buttons, reentry, and 143 captured word comparisons. Physical iPad appearance, runtime performance and energy use remain unverified. See evidence/hd-audio-dialog-controls-display.json for bounded coverage and remaining gaps.
