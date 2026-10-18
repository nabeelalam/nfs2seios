// SPDX-License-Identifier: MIT
// Relocated new speed glyphs require fragment high precision; retain original
// gold when unavailable. The HD timing sampler remains available either way.
		"\n#if !defined(NFS_FORCE_BASIC_HD) && (!defined(GL_ES) || defined(GL_FRAGMENT_PRECISION_HIGH))\n"
		"#ifdef GL_ES\n"
		"highp vec2 hdCoord = vTexCoord.st / vTexCoord.q;\n"
		"#else\n"
		"vec2 hdCoord = vTexCoord.st / vTexCoord.q;\n"
		"#endif\n"
		"if (vTexCoord.z > 1.5) {\nif (vTexCoord.z < 2.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.165113546080508,0.105936253324468);\nelse if (vTexCoord.z < 3.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(-0.171917831688596,0.105936253324468);\nelse if (vTexCoord.z < 4.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.159353482521186,0.105936253324468);\nelse if (vTexCoord.z < 5.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.156473450741525,0.105936253324468);\nelse if (vTexCoord.z < 6.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.157400357521186,0.105936253324468);\nelse if (vTexCoord.z < 7.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.154520325741525,0.105936253324468);\nelse if (vTexCoord.z < 8.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(-0.180826822916667,0.105936253324468);\nelse if (vTexCoord.z < 9.5) hdCoord = hdCoord * vec2(1.01272789817681,1.01460158531498) + vec2(0.503829152960526,0.0111889128989362);\nelse if (vTexCoord.z < 10.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.389722921080508,0.104938912898936);\nelse if (vTexCoord.z < 11.5) hdCoord = hdCoord * vec2(0.978398138916584,1.01460158531498) + vec2(-0.386842889300847,0.104938912898936);\n}\n"
		"vec4 hd = texture2D(uHdTextureSampler, hdCoord);\n"
		"if (vTexCoord.z > 0.5) sampled = hd;\n"
		"#else\n"
		"vec4 hd = texture2D(uHdTextureSampler, vTexCoord.st / vTexCoord.q);\n"
		"if (vTexCoord.z > 0.5 && vTexCoord.z < 1.5) sampled = hd;\n"
		"#endif\n"
		"}\n"
