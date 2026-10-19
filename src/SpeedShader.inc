// SPDX-License-Identifier: MIT
// Relocated gold digits and uppercase letters require high precision; retain original
// gold when unavailable. The HD timing sampler remains available either way.
		"\n#if !defined(NFS_FORCE_BASIC_HD) && (!defined(GL_ES) || defined(GL_FRAGMENT_PRECISION_HIGH))\n"
		"#ifdef GL_ES\n"
		"highp vec2 hdCoord = vTexCoord.st / vTexCoord.q;\n"
		"#else\n"
		"vec2 hdCoord = vTexCoord.st / vTexCoord.q;\n"
		"#endif\n"
		"if (vTexCoord.z > 1.5 && vTexCoord.z < 11.5) {\nif (vTexCoord.z < 2.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.165113546080508,0.105936253324468);\nelse if (vTexCoord.z < 3.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(-0.171917831688596,0.105936253324468);\nelse if (vTexCoord.z < 4.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.159353482521186,0.105936253324468);\nelse if (vTexCoord.z < 5.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.156473450741525,0.105936253324468);\nelse if (vTexCoord.z < 6.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.157400357521186,0.105936253324468);\nelse if (vTexCoord.z < 7.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.154520325741525,0.105936253324468);\nelse if (vTexCoord.z < 8.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(-0.180826822916667,0.105936253324468);\nelse if (vTexCoord.z < 9.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(0.503829152960526,0.0111889128989362);\nelse if (vTexCoord.z < 10.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.389722921080508,0.104938912898936);\nelse if (vTexCoord.z < 11.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.386842889300847,0.104938912898936);\n}\n"
"if(vTexCoord.z > 48.5 && vTexCoord.z < 49.5) hdCoord=hdCoord*vec2(0.251968503937008)+vec2(-0.001472533218503937,0.6860274667814961);\n"
#include "SmallGlyphShader.inc"
"bool pauseOutside=false;\nif(vTexCoord.z>49.5 && vTexCoord.z<50.5){hdCoord=hdCoord*vec2(1.005546037185101,1.014601585314977)+vec2(0.3032194958535599,0.6166784408244681);pauseOutside=hdCoord.x<0.314453125 || hdCoord.x>0.6123046875 || hdCoord.y<0.626953125 || hdCoord.y>0.7041015625;}\nif(vTexCoord.z>50.5 && vTexCoord.z<51.5){hdCoord=hdCoord*vec2(1.005546037185101,1.014601585314977)+vec2(0.6040007458535599,0.5179832945478724);pauseOutside=hdCoord.x<0.6162109375 || hdCoord.x>0.8798828125 || hdCoord.y<0.626953125 || hdCoord.y>0.7041015625;}\nif(vTexCoord.z>51.5 && vTexCoord.z<52.5){hdCoord=hdCoord*vec2(1.005546037185101,1.014601585314977)+vec2(0.3022429333535599,0.4095433011968085);pauseOutside=hdCoord.x<0.314453125 || hdCoord.x>0.7412109375 || hdCoord.y<0.7080078125 || hdCoord.y>0.78515625;}\nif(vTexCoord.z>52.5 && vTexCoord.z<53.5){hdCoord=hdCoord*vec2(1.004907739127085,1.014601585314977)+vec2(0.3041973001596267,0.4017100232712766);pauseOutside=hdCoord.x<0.314453125 || hdCoord.x>0.7900390625 || hdCoord.y<0.794921875 || hdCoord.y>0.8720703125;}\nif(vTexCoord.z>53.5 && vTexCoord.z<54.5){hdCoord=hdCoord*vec2(1.005546037185101,1.014601585314977)+vec2(0.7338835583535599,0.5042906416223405);pauseOutside=hdCoord.x<0.7451171875 || hdCoord.x>0.880859375 || hdCoord.y<0.7080078125 || hdCoord.y>0.791015625;}\nif(vTexCoord.z>54.5 && vTexCoord.z<55.5){hdCoord=hdCoord*vec2(1.004907739127085,1.013048128342246)+vec2(0.3032207376596267,0.07348188920454557);pauseOutside=hdCoord.x<0.314453125 || hdCoord.x>0.798828125 || hdCoord.y<0.8759765625 || hdCoord.y>0.958984375;}\n"

"if(vTexCoord.z>55.5 && vTexCoord.z<56.5)hdCoord=hdCoord*vec2(1.060602307908809,1.07287779758368)+vec2(-0.00632476076555023,0.6005879103535353);\n"
		"vec4 hd = texture2D(uHdTextureSampler, hdCoord);if(pauseOutside)hd=vec4(0.0);\n"
		"if (vTexCoord.z > 0.5) sampled = hd;\n"
		"#else\n"
		"vec4 hd = texture2D(uHdTextureSampler, vTexCoord.st / vTexCoord.q);\n"
		"if (vTexCoord.z > 0.5 && vTexCoord.z < 1.5) sampled = hd;\n"
		"#endif\n"
		"}\n"
