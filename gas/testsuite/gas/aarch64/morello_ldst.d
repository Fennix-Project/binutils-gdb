#as: -march=morello
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

0000000000000000 <.text>:
.*:	a2e47d62 	casa	c4, c2, \[x11\]
.*:	a2e4fd62 	casal	c4, c2, \[x11\]
.*:	a2a47d62 	cas	c4, c2, \[x11\]
.*:	a2a4fd62 	casl	c4, c2, \[x11\]
.*:	a2e47fe2 	casa	c4, c2, \[sp\]
.*:	a2e4ffe2 	casal	c4, c2, \[sp\]
.*:	a2a47fe2 	cas	c4, c2, \[sp\]
.*:	a2a4ffe2 	casl	c4, c2, \[sp\]
.*:	a2a48162 	swpa	c4, c2, \[x11\]
.*:	a2e48162 	swpal	c4, c2, \[x11\]
.*:	a2248162 	swp	c4, c2, \[x11\]
.*:	a2648162 	swpl	c4, c2, \[x11\]
.*:	a2a483e2 	swpa	c4, c2, \[sp\]
.*:	a2e483e2 	swpal	c4, c2, \[sp\]
.*:	a22483e2 	swp	c4, c2, \[sp\]
.*:	a26483e2 	swpl	c4, c2, \[sp\]
.*:	a23fc162 	ldapr	c2, \[x11\]
.*:	425ffd62 	ldar	c2, \[x11\]
.*:	225ffd62 	ldaxr	c2, \[x11\]
.*:	225f7d62 	ldxr	c2, \[x11\]
.*:	421ffd62 	stlr	c2, \[x11\]
.*:	a23fc3e2 	ldapr	c2, \[sp\]
.*:	425fffe2 	ldar	c2, \[sp\]
.*:	225fffe2 	ldaxr	c2, \[sp\]
.*:	225f7fe2 	ldxr	c2, \[sp\]
.*:	421fffe2 	stlr	c2, \[sp\]
.*:	c2c4b162 	ldct	x2, \[x11\]
.*:	c2c49162 	stct	x2, \[x11\]
.*:	c2c4b3e2 	ldct	x2, \[sp\]
.*:	c2c493e2 	stct	x2, \[sp\]
.*:	227f8d62 	ldaxp	c2, c3, \[x11\]
.*:	227f0d62 	ldxp	c2, c3, \[x11\]
.*:	227f97e2 	ldaxp	c2, c5, \[sp\]
.*:	227f17e2 	ldxp	c2, c5, \[sp\]
.*:	2202fd61 	stlxr	w2, c1, \[x11\]
.*:	22027d61 	stxr	w2, c1, \[x11\]
.*:	2202fffd 	stlxr	w2, c29, \[sp\]
.*:	22027ffd 	stxr	w2, c29, \[sp\]
.*:	22229564 	stlxp	w2, c4, c5, \[x11\]
.*:	22221564 	stxp	w2, c4, c5, \[x11\]
.*:	2222f7e4 	stlxp	w2, c4, c29, \[sp\]
.*:	222277e4 	stxp	w2, c4, c29, \[sp\]
.*:	c27ffd64 	ldr	c4, \[x11, #65520\]
.*:	c2400964 	ldr	c4, \[x11, #32\]
.*:	c2400164 	ldr	c4, \[x11\]
.*:	c23ffd64 	str	c4, \[x11, #65520\]
.*:	c2000964 	str	c4, \[x11, #32\]
.*:	c2000164 	str	c4, \[x11\]
.*:	c27fffe4 	ldr	c4, \[sp, #65520\]
.*:	c2400be4 	ldr	c4, \[sp, #32\]
.*:	c24003e4 	ldr	c4, \[sp\]
.*:	c23fffe4 	str	c4, \[sp, #65520\]
.*:	c2000be4 	str	c4, \[sp, #32\]
.*:	c20003e4 	str	c4, \[sp\]
.*:	a24ff164 	ldur	c4, \[x11, #255\]
.*:	a2501164 	ldur	c4, \[x11, #-255\]
.*:	a2400164 	ldur	c4, \[x11\]
.*:	a2410164 	ldur	c4, \[x11, #16\]
.*:	a20ff164 	stur	c4, \[x11, #255\]
.*:	a2101164 	stur	c4, \[x11, #-255\]
.*:	a2000164 	stur	c4, \[x11\]
.*:	a2010164 	stur	c4, \[x11, #16\]
.*:	a24ff3e4 	ldur	c4, \[sp, #255\]
.*:	a25013e4 	ldur	c4, \[sp, #-255\]
.*:	a24003e4 	ldur	c4, \[sp\]
.*:	a24103e4 	ldur	c4, \[sp, #16\]
.*:	a20ff3e4 	stur	c4, \[sp, #255\]
.*:	a21013e4 	stur	c4, \[sp, #-255\]
.*:	a20003e4 	stur	c4, \[sp\]
.*:	a20103e4 	stur	c4, \[sp, #16\]
.*:	a24ff964 	ldtr	c4, \[x11, #4080\]
.*:	a2500964 	ldtr	c4, \[x11, #-4096\]
.*:	a2400964 	ldtr	c4, \[x11\]
.*:	a2401964 	ldtr	c4, \[x11, #16\]
.*:	a20ff964 	sttr	c4, \[x11, #4080\]
.*:	a2100964 	sttr	c4, \[x11, #-4096\]
.*:	a2000964 	sttr	c4, \[x11\]
.*:	a2001964 	sttr	c4, \[x11, #16\]
.*:	a24ffbe4 	ldtr	c4, \[sp, #4080\]
.*:	a2500be4 	ldtr	c4, \[sp, #-4096\]
.*:	a2400be4 	ldtr	c4, \[sp\]
.*:	a2401be4 	ldtr	c4, \[sp, #16\]
.*:	a20ffbe4 	sttr	c4, \[sp, #4080\]
.*:	a2100be4 	sttr	c4, \[sp, #-4096\]
.*:	a2000be4 	sttr	c4, \[sp\]
.*:	a2001be4 	sttr	c4, \[sp, #16\]
.*:	42df9564 	ldp	c4, c5, \[x11, #1008\]
.*:	42e01564 	ldp	c4, c5, \[x11, #-1024\]
.*:	42c09564 	ldp	c4, c5, \[x11, #16\]
.*:	42ff1564 	ldp	c4, c5, \[x11, #-32\]
.*:	429f9564 	stp	c4, c5, \[x11, #1008\]
.*:	42a01564 	stp	c4, c5, \[x11, #-1024\]
.*:	42809564 	stp	c4, c5, \[x11, #16\]
.*:	42bf1564 	stp	c4, c5, \[x11, #-32\]
.*:	625f9564 	ldnp	c4, c5, \[x11, #1008\]
.*:	62601564 	ldnp	c4, c5, \[x11, #-1024\]
.*:	62409564 	ldnp	c4, c5, \[x11, #16\]
.*:	627f1564 	ldnp	c4, c5, \[x11, #-32\]
.*:	621f9564 	stnp	c4, c5, \[x11, #1008\]
.*:	62201564 	stnp	c4, c5, \[x11, #-1024\]
.*:	62009564 	stnp	c4, c5, \[x11, #16\]
.*:	623f1564 	stnp	c4, c5, \[x11, #-32\]
.*:	42df9be4 	ldp	c4, c6, \[sp, #1008\]
.*:	42e01be4 	ldp	c4, c6, \[sp, #-1024\]
.*:	42c09be4 	ldp	c4, c6, \[sp, #16\]
.*:	42ff1be4 	ldp	c4, c6, \[sp, #-32\]
.*:	429f9be4 	stp	c4, c6, \[sp, #1008\]
.*:	42a01be4 	stp	c4, c6, \[sp, #-1024\]
.*:	42809be4 	stp	c4, c6, \[sp, #16\]
.*:	42bf1be4 	stp	c4, c6, \[sp, #-32\]
.*:	625f9be4 	ldnp	c4, c6, \[sp, #1008\]
.*:	62601be4 	ldnp	c4, c6, \[sp, #-1024\]
.*:	62409be4 	ldnp	c4, c6, \[sp, #16\]
.*:	627f1be4 	ldnp	c4, c6, \[sp, #-32\]
.*:	621f9be4 	stnp	c4, c6, \[sp, #1008\]
.*:	62201be4 	stnp	c4, c6, \[sp, #-1024\]
.*:	62009be4 	stnp	c4, c6, \[sp, #16\]
.*:	623f1be4 	stnp	c4, c6, \[sp, #-32\]
.*:	a2402d64 	ldr	c4, \[x11, #32\]!
.*:	a2402564 	ldr	c4, \[x11\], #32
.*:	a2500d64 	ldr	c4, \[x11, #-4096\]!
.*:	a2500564 	ldr	c4, \[x11\], #-4096
.*:	a24ffd64 	ldr	c4, \[x11, #4080\]!
.*:	a24ff564 	ldr	c4, \[x11\], #4080
.*:	a2002d64 	str	c4, \[x11, #32\]!
.*:	a2002564 	str	c4, \[x11\], #32
.*:	a2100d64 	str	c4, \[x11, #-4096\]!
.*:	a2100564 	str	c4, \[x11\], #-4096
.*:	a20ffd64 	str	c4, \[x11, #4080\]!
.*:	a20ff564 	str	c4, \[x11\], #4080
.*:	a2402fe4 	ldr	c4, \[sp, #32\]!
.*:	a24027e4 	ldr	c4, \[sp\], #32
.*:	a2500fe4 	ldr	c4, \[sp, #-4096\]!
.*:	a25007e4 	ldr	c4, \[sp\], #-4096
.*:	a24fffe4 	ldr	c4, \[sp, #4080\]!
.*:	a24ff7e4 	ldr	c4, \[sp\], #4080
.*:	a2002fe4 	str	c4, \[sp, #32\]!
.*:	a20027e4 	str	c4, \[sp\], #32
.*:	a2100fe4 	str	c4, \[sp, #-4096\]!
.*:	a21007e4 	str	c4, \[sp\], #-4096
.*:	a20fffe4 	str	c4, \[sp, #4080\]!
.*:	a20ff7e4 	str	c4, \[sp\], #4080
.*:	62c11163 	ldp	c3, c4, \[x11, #32\]!
.*:	22c11163 	ldp	c3, c4, \[x11\], #32
.*:	62e01163 	ldp	c3, c4, \[x11, #-1024\]!
.*:	22e01163 	ldp	c3, c4, \[x11\], #-1024
.*:	62df9163 	ldp	c3, c4, \[x11, #1008\]!
.*:	22df9163 	ldp	c3, c4, \[x11\], #1008
.*:	62811163 	stp	c3, c4, \[x11, #32\]!
.*:	22811163 	stp	c3, c4, \[x11\], #32
.*:	62a01163 	stp	c3, c4, \[x11, #-1024\]!
.*:	22a01163 	stp	c3, c4, \[x11\], #-1024
.*:	629f9163 	stp	c3, c4, \[x11, #1008\]!
.*:	229f9163 	stp	c3, c4, \[x11\], #1008
.*:	62c117e4 	ldp	c4, c5, \[sp, #32\]!
.*:	22c117e4 	ldp	c4, c5, \[sp\], #32
.*:	62e017e4 	ldp	c4, c5, \[sp, #-1024\]!
.*:	22e017e4 	ldp	c4, c5, \[sp\], #-1024
.*:	62df97e4 	ldp	c4, c5, \[sp, #1008\]!
.*:	22df97e4 	ldp	c4, c5, \[sp\], #1008
.*:	628117e4 	stp	c4, c5, \[sp, #32\]!
.*:	228117e4 	stp	c4, c5, \[sp\], #32
.*:	62a017e4 	stp	c4, c5, \[sp, #-1024\]!
.*:	22a017e4 	stp	c4, c5, \[sp\], #-1024
.*:	629f97e4 	stp	c4, c5, \[sp, #1008\]!
.*:	229f97e4 	stp	c4, c5, \[sp\], #1008
.*:	a2636964 	ldr	c4, \[x11, x3\]
.*:	a2636964 	ldr	c4, \[x11, x3\]
.*:	a2635964 	ldr	c4, \[x11, w3, uxtw #4\]
.*:	a263d964 	ldr	c4, \[x11, w3, sxtw #4\]
.*:	a263c964 	ldr	c4, \[x11, w3, sxtw\]
.*:	a2236964 	str	c4, \[x11, x3\]
.*:	a2236964 	str	c4, \[x11, x3\]
.*:	a2235964 	str	c4, \[x11, w3, uxtw #4\]
.*:	a223d964 	str	c4, \[x11, w3, sxtw #4\]
.*:	a223c964 	str	c4, \[x11, w3, sxtw\]
.*:	a2636be4 	ldr	c4, \[sp, x3\]
.*:	a2636be4 	ldr	c4, \[sp, x3\]
.*:	a2635be4 	ldr	c4, \[sp, w3, uxtw #4\]
.*:	a263dbe4 	ldr	c4, \[sp, w3, sxtw #4\]
.*:	a263cbe4 	ldr	c4, \[sp, w3, sxtw\]
.*:	a2236be4 	str	c4, \[sp, x3\]
.*:	a2236be4 	str	c4, \[sp, x3\]
.*:	a2235be4 	str	c4, \[sp, w3, uxtw #4\]
.*:	a223dbe4 	str	c4, \[sp, w3, sxtw #4\]
.*:	a223cbe4 	str	c4, \[sp, w3, sxtw\]
.*:	c2d813e1 	blr	\[csp, #-1024\]
.*:	c2d7f3e1 	blr	\[csp, #1008\]
.*:	c2d033e1 	blr	\[csp, #16\]
.*:	c2d013e1 	blr	\[csp\]
.*:	c2d813e0 	br	\[csp, #-1024\]
.*:	c2d7f3e0 	br	\[csp, #1008\]
.*:	c2d033e0 	br	\[csp, #16\]
.*:	c2d013e0 	br	\[csp\]
.*:	c2d81041 	blr	\[c2, #-1024\]
.*:	c2d7f041 	blr	\[c2, #1008\]
.*:	c2d03041 	blr	\[c2, #16\]
.*:	c2d01041 	blr	\[c2\]
.*:	c2d81040 	br	\[c2, #-1024\]
.*:	c2d7f040 	br	\[c2, #1008\]
.*:	c2d03040 	br	\[c2, #16\]
.*:	c2d01040 	br	\[c2\]
.*:	c2c433e2 	ldpblr	c2, \[csp\]
.*:	c2c413e2 	ldpbr	c2, \[csp\]
.*:	c2c43042 	ldpblr	c2, \[c2\]
.*:	c2c41042 	ldpbr	c2, \[c2\]
.*:	f88ff160 	prfum	pldl1keep, \[x11, #255\]
.*:	f8901173 	prfum	pstl2strm, \[x11, #-255\]
.*:	f8800163 	prfum	pldl2strm, \[x11\]
.*:	f8810170 	prfum	pstl1keep, \[x11, #16\]
.*:	f88ff3e0 	prfum	pldl1keep, \[sp, #255\]
.*:	f89013f3 	prfum	pstl2strm, \[sp, #-255\]
.*:	f88003e3 	prfum	pldl2strm, \[sp\]
.*:	f88103f0 	prfum	pstl1keep, \[sp, #16\]
.*:	d508762b 	dc	ivac, x11
.*:	d508764b 	dc	isw, x11
.*:	d5087a4b 	dc	csw, x11
.*:	d5087e4b 	dc	cisw, x11
.*:	d50b742b 	dc	zva, x11
.*:	d50b7a2b 	dc	cvac, x11
.*:	d50b7b2b 	dc	cvau, x11
.*:	d50b7c2b 	dc	cvap, x11
.*:	d50b7e2b 	dc	civac, x11
.*:	d50b752b 	ic	ivau, x11
.*:	425f7ea2 	ldar	c2, \[c21\]
.*:	421f7ea2 	stlr	c2, \[c21\]
.*:	425f7fe2 	ldar	c2, \[csp\]
.*:	421f7fe2 	stlr	c2, \[csp\]
.*:	427f7ea2 	ldarb	w2, \[c21\]
.*:	427ffea2 	ldar	w2, \[c21\]
.*:	423f7ea2 	stlrb	w2, \[c21\]
.*:	423ffea2 	stlr	w2, \[c21\]
.*:	427f7fe2 	ldarb	w2, \[csp\]
.*:	427fffe2 	ldar	w2, \[csp\]
.*:	423f7fe2 	stlrb	w2, \[csp\]
.*:	423fffe2 	stlr	w2, \[csp\]
.*:	827ff2a4 	ldr	c4, \[c21, #8176\]
.*:	826022a4 	ldr	c4, \[c21, #32\]
.*:	826002a4 	ldr	c4, \[c21\]
.*:	825ff2a4 	str	c4, \[c21, #8176\]
.*:	824022a4 	str	c4, \[c21, #32\]
.*:	824002a4 	str	c4, \[c21\]
.*:	827ff3e4 	ldr	c4, \[csp, #8176\]
.*:	826023e4 	ldr	c4, \[csp, #32\]
.*:	826003e4 	ldr	c4, \[csp\]
.*:	825ff3e4 	str	c4, \[csp, #8176\]
.*:	824023e4 	str	c4, \[csp, #32\]
.*:	824003e4 	str	c4, \[csp\]
.*:	827ffea4 	ldr	x4, \[c21, #4088\]
.*:	82604ea4 	ldr	x4, \[c21, #32\]
.*:	82600ea4 	ldr	x4, \[c21\]
.*:	825ffea4 	str	x4, \[c21, #4088\]
.*:	82404ea4 	str	x4, \[c21, #32\]
.*:	82400ea4 	str	x4, \[c21\]
.*:	827fffe4 	ldr	x4, \[csp, #4088\]
.*:	82604fe4 	ldr	x4, \[csp, #32\]
.*:	82600fe4 	ldr	x4, \[csp\]
.*:	825fffe4 	str	x4, \[csp, #4088\]
.*:	82404fe4 	str	x4, \[csp, #32\]
.*:	82400fe4 	str	x4, \[csp\]
.*:	827ffaa4 	ldr	w4, \[c21, #2044\]
.*:	82608aa4 	ldr	w4, \[c21, #32\]
.*:	82600aa4 	ldr	w4, \[c21\]
.*:	825ffaa4 	str	w4, \[c21, #2044\]
.*:	82408aa4 	str	w4, \[c21, #32\]
.*:	82400aa4 	str	w4, \[c21\]
.*:	827ffbe4 	ldr	w4, \[csp, #2044\]
.*:	82608be4 	ldr	w4, \[csp, #32\]
.*:	82600be4 	ldr	w4, \[csp\]
.*:	825ffbe4 	str	w4, \[csp, #2044\]
.*:	82408be4 	str	w4, \[csp, #32\]
.*:	82400be4 	str	w4, \[csp\]
.*:	e2cffea4 	ldur	c4, \[c21, #255\]
.*:	e2c20ea4 	ldur	c4, \[c21, #32\]
.*:	e2d00ea4 	ldur	c4, \[c21, #-256\]
.*:	e2cff6a4 	ldur	x4, \[c21, #255\]
.*:	e2c206a4 	ldur	x4, \[c21, #32\]
.*:	e2d006a4 	ldur	x4, \[c21, #-256\]
.*:	e28ff6a4 	ldur	w4, \[c21, #255\]
.*:	e28206a4 	ldur	w4, \[c21, #32\]
.*:	e29006a4 	ldur	w4, \[c21, #-256\]
.*:	e22ff6a4 	ldur	b4, \[c21, #255\]
.*:	e22206a4 	ldur	b4, \[c21, #32\]
.*:	e23006a4 	ldur	b4, \[c21, #-256\]
.*:	e26ff6a4 	ldur	h4, \[c21, #255\]
.*:	e26206a4 	ldur	h4, \[c21, #32\]
.*:	e27006a4 	ldur	h4, \[c21, #-256\]
.*:	e2aff6a4 	ldur	s4, \[c21, #255\]
.*:	e2a206a4 	ldur	s4, \[c21, #32\]
.*:	e2b006a4 	ldur	s4, \[c21, #-256\]
.*:	e2eff6a4 	ldur	d4, \[c21, #255\]
.*:	e2e206a4 	ldur	d4, \[c21, #32\]
.*:	e2f006a4 	ldur	d4, \[c21, #-256\]
.*:	e22ffea4 	ldur	q4, \[c21, #255\]
.*:	e2220ea4 	ldur	q4, \[c21, #32\]
.*:	e2300ea4 	ldur	q4, \[c21, #-256\]
.*:	e28ffea4 	stur	c4, \[c21, #255\]
.*:	e2820ea4 	stur	c4, \[c21, #32\]
.*:	e2900ea4 	stur	c4, \[c21, #-256\]
.*:	e2cff2a4 	stur	x4, \[c21, #255\]
.*:	e2c202a4 	stur	x4, \[c21, #32\]
.*:	e2d002a4 	stur	x4, \[c21, #-256\]
.*:	e28ff2a4 	stur	w4, \[c21, #255\]
.*:	e28202a4 	stur	w4, \[c21, #32\]
.*:	e29002a4 	stur	w4, \[c21, #-256\]
.*:	e22ff2a4 	stur	b4, \[c21, #255\]
.*:	e22202a4 	stur	b4, \[c21, #32\]
.*:	e23002a4 	stur	b4, \[c21, #-256\]
.*:	e26ff2a4 	stur	h4, \[c21, #255\]
.*:	e26202a4 	stur	h4, \[c21, #32\]
.*:	e27002a4 	stur	h4, \[c21, #-256\]
.*:	e2aff2a4 	stur	s4, \[c21, #255\]
.*:	e2a202a4 	stur	s4, \[c21, #32\]
.*:	e2b002a4 	stur	s4, \[c21, #-256\]
.*:	e2eff2a4 	stur	d4, \[c21, #255\]
.*:	e2e202a4 	stur	d4, \[c21, #32\]
.*:	e2f002a4 	stur	d4, \[c21, #-256\]
.*:	e22ffaa4 	stur	q4, \[c21, #255\]
.*:	e2220aa4 	stur	q4, \[c21, #32\]
.*:	e2300aa4 	stur	q4, \[c21, #-256\]
.*:	e2cfffe4 	ldur	c4, \[csp, #255\]
.*:	e2c20fe4 	ldur	c4, \[csp, #32\]
.*:	e2d00fe4 	ldur	c4, \[csp, #-256\]
.*:	e2cff7e4 	ldur	x4, \[csp, #255\]
.*:	e2c207e4 	ldur	x4, \[csp, #32\]
.*:	e2d007e4 	ldur	x4, \[csp, #-256\]
.*:	e28ff7e4 	ldur	w4, \[csp, #255\]
.*:	e28207e4 	ldur	w4, \[csp, #32\]
.*:	e29007e4 	ldur	w4, \[csp, #-256\]
.*:	e22ff7e4 	ldur	b4, \[csp, #255\]
.*:	e22207e4 	ldur	b4, \[csp, #32\]
.*:	e23007e4 	ldur	b4, \[csp, #-256\]
.*:	e26ff7e4 	ldur	h4, \[csp, #255\]
.*:	e26207e4 	ldur	h4, \[csp, #32\]
.*:	e27007e4 	ldur	h4, \[csp, #-256\]
.*:	e2aff7e4 	ldur	s4, \[csp, #255\]
.*:	e2a207e4 	ldur	s4, \[csp, #32\]
.*:	e2b007e4 	ldur	s4, \[csp, #-256\]
.*:	e2eff7e4 	ldur	d4, \[csp, #255\]
.*:	e2e207e4 	ldur	d4, \[csp, #32\]
.*:	e2f007e4 	ldur	d4, \[csp, #-256\]
.*:	e22fffe4 	ldur	q4, \[csp, #255\]
.*:	e2220fe4 	ldur	q4, \[csp, #32\]
.*:	e2300fe4 	ldur	q4, \[csp, #-256\]
.*:	e28fffe4 	stur	c4, \[csp, #255\]
.*:	e2820fe4 	stur	c4, \[csp, #32\]
.*:	e2900fe4 	stur	c4, \[csp, #-256\]
.*:	e2cff3e4 	stur	x4, \[csp, #255\]
.*:	e2c203e4 	stur	x4, \[csp, #32\]
.*:	e2d003e4 	stur	x4, \[csp, #-256\]
.*:	e28ff3e4 	stur	w4, \[csp, #255\]
.*:	e28203e4 	stur	w4, \[csp, #32\]
.*:	e29003e4 	stur	w4, \[csp, #-256\]
.*:	e22ff3e4 	stur	b4, \[csp, #255\]
.*:	e22203e4 	stur	b4, \[csp, #32\]
.*:	e23003e4 	stur	b4, \[csp, #-256\]
.*:	e26ff3e4 	stur	h4, \[csp, #255\]
.*:	e26203e4 	stur	h4, \[csp, #32\]
.*:	e27003e4 	stur	h4, \[csp, #-256\]
.*:	e2aff3e4 	stur	s4, \[csp, #255\]
.*:	e2a203e4 	stur	s4, \[csp, #32\]
.*:	e2b003e4 	stur	s4, \[csp, #-256\]
.*:	e2eff3e4 	stur	d4, \[csp, #255\]
.*:	e2e203e4 	stur	d4, \[csp, #32\]
.*:	e2f003e4 	stur	d4, \[csp, #-256\]
.*:	e22ffbe4 	stur	q4, \[csp, #255\]
.*:	e2220be4 	stur	q4, \[csp, #32\]
.*:	e2300be4 	stur	q4, \[csp, #-256\]
.*:	827ff6a4 	ldrb	w4, \[c21, #511\]
.*:	826206a4 	ldrb	w4, \[c21, #32\]
.*:	826006a4 	ldrb	w4, \[c21\]
.*:	825ff6a4 	strb	w4, \[c21, #511\]
.*:	824206a4 	strb	w4, \[c21, #32\]
.*:	824006a4 	strb	w4, \[c21\]
.*:	827ff7e4 	ldrb	w4, \[csp, #511\]
.*:	826207e4 	ldrb	w4, \[csp, #32\]
.*:	826007e4 	ldrb	w4, \[csp\]
.*:	825ff7e4 	strb	w4, \[csp, #511\]
.*:	824207e4 	strb	w4, \[csp, #32\]
.*:	824007e4 	strb	w4, \[csp\]
.*:	a25f0164 	ldur	c4, \[x11, #-16\]
.*:	a2500164 	ldur	c4, \[x11, #-256\]
.*:	a21f0164 	stur	c4, \[x11, #-16\]
.*:	a2100164 	stur	c4, \[x11, #-256\]
.*:	a25f03e4 	ldur	c4, \[sp, #-16\]
.*:	a25003e4 	ldur	c4, \[sp, #-256\]
.*:	a21f03e4 	stur	c4, \[sp, #-16\]
.*:	a21003e4 	stur	c4, \[sp, #-256\]
.*:	e2df0ea4 	ldur	c4, \[c21, #-16\]
.*:	e2d00ea4 	ldur	c4, \[c21, #-256\]
.*:	e2df06a4 	ldur	x4, \[c21, #-16\]
.*:	e2d006a4 	ldur	x4, \[c21, #-256\]
.*:	e29f06a4 	ldur	w4, \[c21, #-16\]
.*:	e29006a4 	ldur	w4, \[c21, #-256\]
.*:	e29f0ea4 	stur	c4, \[c21, #-16\]
.*:	e2900ea4 	stur	c4, \[c21, #-256\]
.*:	e2df02a4 	stur	x4, \[c21, #-16\]
.*:	e2d002a4 	stur	x4, \[c21, #-256\]
.*:	e29f02a4 	stur	w4, \[c21, #-16\]
.*:	e29002a4 	stur	w4, \[c21, #-256\]
.*:	e2df0fe4 	ldur	c4, \[csp, #-16\]
.*:	e2d00fe4 	ldur	c4, \[csp, #-256\]
.*:	e2df07e4 	ldur	x4, \[csp, #-16\]
.*:	e2d007e4 	ldur	x4, \[csp, #-256\]
.*:	e29f07e4 	ldur	w4, \[csp, #-16\]
.*:	e29007e4 	ldur	w4, \[csp, #-256\]
.*:	e29f0fe4 	stur	c4, \[csp, #-16\]
.*:	e2900fe4 	stur	c4, \[csp, #-256\]
.*:	e2df03e4 	stur	x4, \[csp, #-16\]
.*:	e2d003e4 	stur	x4, \[csp, #-256\]
.*:	e29f03e4 	stur	w4, \[csp, #-16\]
.*:	e29003e4 	stur	w4, \[csp, #-256\]
.*:	e21ff6a4 	ldurb	w4, \[c21, #-1\]
.*:	e21f06a4 	ldurb	w4, \[c21, #-16\]
.*:	e21006a4 	ldurb	w4, \[c21, #-256\]
.*:	e21ff2a4 	sturb	w4, \[c21, #-1\]
.*:	e21f02a4 	sturb	w4, \[c21, #-16\]
.*:	e21002a4 	sturb	w4, \[c21, #-256\]
.*:	e21ff7e4 	ldurb	w4, \[csp, #-1\]
.*:	e21f07e4 	ldurb	w4, \[csp, #-16\]
.*:	e21007e4 	ldurb	w4, \[csp, #-256\]
.*:	e21ff3e4 	sturb	w4, \[csp, #-1\]
.*:	e21f03e4 	sturb	w4, \[csp, #-16\]
.*:	e21003e4 	sturb	w4, \[csp, #-256\]
