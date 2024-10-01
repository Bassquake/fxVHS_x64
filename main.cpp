/*//////////////////////////////////////////////////////////////////////////

	flaXen's VHS VirtualDub Filter (c) Copyright 2000 Dan Flower (flaXen)

	Reduce or eliminate chromatic morie	paterns, abrupt chromatic
	deviations, and noise by use of a temporal accumulation buffer.
	(Works on any video source, but was built for VHS)

*///////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>

#include "resource.h"

#include "ScriptInterpreter.h"
#include "ScriptError.h"
#include "ScriptValue.h"
#include "filter.h"

// Set to call assembly code?
extern "C" void Asm1();
extern "C" void Asm2();
extern "C" void Asm3();
extern "C" void Asm4();
extern "C" void Asm5();
extern "C" void Asm6();
extern "C" void Asm7();
extern "C" void Asm8();
extern "C" void Asm9();
extern "C" void Asm10();
extern "C" void Asm11();
extern "C" void Asm12();
extern "C" void Asm13();
extern "C" void Asm14();

typedef struct MyFilterData {
	long	*TmpBuff, *ScaleTab;
	long	*Div3Tab, *Div5Tab, *Div9Tab;
	long	*yr, *yg, *yb;
	long	*ir, *ig, *ib;
	long	*qr, *qg, *qb;
	long	*ri, *rq;
	long	*gi, *gq;
	long	*bi, *bq;
	long	*stBuffY, *stBuffI, *stBuffQ;
	long	*stlBuffY, *stlBuffI, *stlBuffQ;
	long	*stpBuffY, *stpBuffI, *stpBuffQ;
	long	*stCount, *stError;
	int		stChromaThresh, stLumaThresh;
	int		stTempError, stTempBias, stEnable;
	int		nrThreshold, nrRadius;
	int		nrPreFilter, nrPostFilter;
	int		shEnable, shEffect, sh8Dir;
	int		csHorizontal, csHorizNeg;
	int		csVertical, csVertNeg;
	int		csShiftI, csShiftQ;
} MyFilterData;

///////////////////////////////////////////////////////////////////////////

int		fxVHSRunProc(const FilterActivation *fa, const FilterFunctions *ff);
void	fxVHS_CrShift(Pixel32 *, Pixel32 *, PixDim, PixDim, long, MyFilterData *);
void	fxVHS_DeNoise(Pixel32 *, Pixel32 *, PixDim, PixDim, long, MyFilterData *);
void	fxVHS_Sharpen(Pixel32 *, Pixel32 *, PixDim, PixDim, long, MyFilterData *);
int		fxVHSStartProc(FilterActivation *fa, const FilterFunctions *ff);
int		fxVHSEndProc(FilterActivation *fa, const FilterFunctions *ff); 
int		fxVHSInitProc(FilterActivation *fa, const FilterFunctions *ff);
int		fxVHSConfigProc(FilterActivation *fa, const FilterFunctions *ff, HWND hwnd);
void	fxVHSStringProc(const FilterActivation *fa, const FilterFunctions *ff, char *buf);
void	fxVHSScriptConfig(IScriptInterpreter *isi, void *lpVoid, CScriptValue *argv, int argc);
bool	fxVHSFssProc(FilterActivation *fa, const FilterFunctions *ff, char *buf, int buflen);

///////////////////////////////////////////////////////////////////////////

ScriptFunctionDef fxVHS_func_defs[]={
    { (ScriptFunctionPtr)fxVHSScriptConfig, "Config", "0iiiiiiiiiiiiiiii" },
    { NULL },
};

CScriptObject fxVHSobj={
    NULL, fxVHS_func_defs
};

struct FilterDefinition filterDef_fxVHS = {
	NULL, NULL, NULL,		// next, prev, module

	"VHS",				// name
	"flaXen's VHS Toys: Color Stabilizing, Chroma Shifting, Noise Reduction, and Sharpening",	// desc
	"flaXen",				// maker

	NULL,					// private_data
	sizeof(MyFilterData),	// inst_data_size
	fxVHSInitProc,			// initProc
	NULL,					// deinitProc
	fxVHSRunProc,			// runProc
	NULL,					// paramProc
	fxVHSConfigProc,		// configProc
	fxVHSStringProc,		// stringProc
	fxVHSStartProc,			// startProc
	fxVHSEndProc,			// endProc
	&fxVHSobj,				// script_obj
	fxVHSFssProc,			// fssProc
};

///////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) __cdecl VirtualdubFilterModuleInit2(FilterModule *fm, const FilterFunctions *ff, int& vdfd_ver, int& vdfd_compat);
extern "C" void __declspec(dllexport) __cdecl VirtualdubFilterModuleDeinit(FilterModule *fm, const FilterFunctions *ff);

static FilterDefinition *fd_fxVHS;

int __declspec(dllexport) __cdecl VirtualdubFilterModuleInit2(FilterModule *fm, const FilterFunctions *ff, int& vdfd_ver, int& vdfd_compat) {
	if (!(fd_fxVHS = ff->addFilter(fm, &filterDef_fxVHS, sizeof(FilterDefinition))))
		return 1;

	vdfd_ver    = VIRTUALDUB_FILTERDEF_VERSION;
	vdfd_compat = VIRTUALDUB_FILTERDEF_COMPATIBLE;
	return 0;
}

void __declspec(dllexport) __cdecl VirtualdubFilterModuleDeinit(FilterModule *fm, const FilterFunctions *ff) {
	ff->removeFilter(fd_fxVHS);
}

///////////////////////////////////////////////////////////////////////////

int fxVHSRunProc(const FilterActivation *fa, const FilterFunctions *ff) {
	MyFilterData	*mfd = (MyFilterData *)fa->filter_data;
	PixDim	w, h;
	Pixel32	*src, *dst, *isrc, *idst;
	long	*byi, *bii, *bqi, *bci, *bei;
	long	cr, cg, cb, cy, ci, cq;
	long	tr, tg, tb;
	long	*lbyi, *lbii, *lbqi;
	long	*pbyi, *pbii, *pbqi;
	long	*by = (long *)mfd->stBuffY;
	long	*bu = (long *)mfd->stBuffI;
	long	*bv = (long *)mfd->stBuffQ;
	long	*bc = (long *)mfd->stCount;
	long	*be = (long *)mfd->stError;
	long	*lby = (long *)mfd->stlBuffY;
	long	*lbu = (long *)mfd->stlBuffI;
	long	*lbv = (long *)mfd->stlBuffQ;
	long	*pby = (long *)mfd->stpBuffY;
	long	*pbu = (long *)mfd->stpBuffI;
	long	*pbv = (long *)mfd->stpBuffQ;
	long	*yr = (long *)mfd->yr;
	long	*yg = (long *)mfd->yg;
	long	*yb = (long *)mfd->yb;
	long	*ir = (long *)mfd->ir;
	long	*ig = (long *)mfd->ig;
	long	*ib = (long *)mfd->ib;
	long	*qr = (long *)mfd->qr;
	long	*qg = (long *)mfd->qg;
	long	*qb = (long *)mfd->qb;
	long	*ri = (long *)mfd->ri;
	long	*rq = (long *)mfd->rq;
	long	*gi = (long *)mfd->gi;
	long	*gq = (long *)mfd->gq;
	long	*bi = (long *)mfd->bi;
	long	*bq = (long *)mfd->bq;
	const long	lThresh = mfd->stLumaThresh;
	const long	cThresh = mfd->stChromaThresh;
	const long	swid = fa->src.w, shei = fa->src.h;		// So I'm lazy...
	const long	spit = fa->src.pitch >> 2;
	const long	ssize = fa->src.pitch * fa->src.h;

	//isrc = (Pixel32 *)fa->src.data;
	idst = (Pixel32 *)fa->dst.data;
	isrc = (Pixel32 *)mfd->TmpBuff;
	memcpy(isrc, fa->src.data, ssize);
	memcpy(idst, isrc, ssize);

	// Chroma Shifter
	if (mfd->csShiftI == BST_CHECKED || mfd->csShiftQ == BST_CHECKED) {
		fxVHS_CrShift(isrc, idst, swid - 1, shei - 1, spit, mfd);
		memcpy(isrc, idst, ssize);
	}

	// Pre-Filter De-Noise
	if (mfd->nrPreFilter == BST_CHECKED) {
		fxVHS_DeNoise(isrc, idst, swid, shei, spit, mfd);
		memcpy(isrc, idst, ssize);
	}

	// Chroma Stabilizer
	if (mfd->stEnable == BST_CHECKED) {
		// Vertical loop
		h = fa->dst.h;
		do {
			// Prepare all sorts of pointers...
			// b?i pointers reference the temporal accumulation buffer
			// bci references the counter portion of the accumulation buffer
			w = (h-1) * swid;
			byi = by + w; bii = bu + w;	bqi = bv + w;
			// bci and bei reference temporal counters
			bci = bc + w; bei = be + w;
			// lb?i pointers reference the LAST frame (no temporal functions)
			lbyi = lby + w; lbii = lbu + w; lbqi = lbv + w;
			// pb?i pointers reference the LAST accumulated frame
			pbyi = lby + w; pbii = lbu + w; pbqi = lbv + w;
			// src and dst reference the actual source and destination frames
			src = isrc + (h-1) * spit;
			dst = idst + (h-1) * spit;
			// Horizontal loop
			w = fa->dst.w;			
			do {
				// Read pixel & break it down
				//Function to call asm1.asm file
				//Asm1();
				
				// Find difference between current and previously accumulated
				// luma's for use in the range check and storing temporal error
				cr = abs(*pbyi - cy);
				// Find difference between current Luma and Luma from previous frame
				// and test against the Luma Threshold
				if (abs(*lbyi - cy) <= lThresh && cr <= lThresh) {
					cg = abs(*pbii - ci);
					cb = abs(*pbqi - cq);
					// If within range, check Chroma Thresholds in the same way as
					// the Luma Threshold check
					if (abs(*lbii - ci) <= cThresh && abs(*lbqi - cq) <= cThresh && cg <= cThresh && cb <= cThresh) {
						// If BOTH Chroma's fall within the Chroma Threshold, add
						// all signal components to the temporal accumulation buffer
						// and increment the accumulation counter
						*byi += cy; *bii += ci; *bqi += cq;
						*bci = *bci + 1;	// For some reason, *bci++ didn't work -- causes crash
						// Store error count
						*bei = *bei + mfd->Div3Tab[cr + cg + cb];
						if (*bei >= mfd->stTempError) {
							*byi = cy; *bii = ci; *bqi = cq;
							*bci = 1; *bei = 0;
						}
					} else {
						// If one (or both) of the Chroma's are outside of the
						// Threshold, replace the data in the temporal accumulation
						// buffer with new values and reset the accumulation counters
						*byi = cy; *bii = ci; *bqi = cq;
						*bci = 1; *bei = 0;
					}
				} else {
					// Same as above. Replace old components with new and reset counters
					*byi = cy; *bii = ci; *bqi = cq;
					*bci = 1; *bei = 0;
				}
	
				// Store current video components in the previous frame buffer
				*lbyi++ = cy; *lbii++ = ci; *lbqi++ = cq;

				// Calculate new components by dividing the temporal accumulation
				// buffer's data by the accumulation count (averages all collected
				// samples together)
				cy = *byi / *bci;
				ci = *bii / *bci;
				cq = *bqi / *bci;
				// Store results in previous frame buffer
				*pbyi++ = cy; *pbii++ = ci; *pbqi++ = cq;

				// Convert the video components (YIQ) back to RGB using a fixed-point
				// matrix lookup table and clamp results.
				//Function to call asm2.asm file
				//Asm2();

				// Increment temporal accumulation buffer pointers
				byi++; bii++; bqi++; bci++; bei++;
			} while(--w);
		} while(--h);
	}

	// Sharpen
	if (mfd->shEnable == BST_CHECKED) {
		fxVHS_Sharpen(idst, isrc, swid, shei, spit, mfd);
		memmove(idst, isrc, ssize);
	}

	// Post-Filter De-Noise
	if (mfd->nrPostFilter == BST_CHECKED) {
		fxVHS_DeNoise(idst, isrc, swid, shei, spit, mfd);
		memmove(idst, isrc, ssize);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////

void fxVHS_CrShift(Pixel32 *isrc, Pixel32 *idst, PixDim swid, PixDim shei, long spit, MyFilterData *mfd) {
	PixDim	x, y;
	Pixel32	*src, *dst, *ofs;
	long	tr, tg, tb, cy, ci, cq, ho, vo;
	long	*yr = (long *)mfd->yr, *yg = (long *)mfd->yg, *yb = (long *)mfd->yb;
	long	*ir = (long *)mfd->ir, *ig = (long *)mfd->ig, *ib = (long *)mfd->ib;
	long	*qr = (long *)mfd->qr, *qg = (long *)mfd->qg, *qb = (long *)mfd->qb;
	long	*ri = (long *)mfd->ri, *rq = (long *)mfd->rq;
	long	*gi = (long *)mfd->gi, *gq = (long *)mfd->gq;
	long	*bi = (long *)mfd->bi, *bq = (long *)mfd->bq;
	const long	hoff = mfd->csHorizontal * (mfd->csHorizNeg == BST_CHECKED ? -1 : 1);
	const long	voff = mfd->csVertical * (mfd->csVertNeg == BST_CHECKED ? 1 : -1);

	for (y = 0; y < shei; y++) {
		src = isrc + y * spit;
		dst = idst + y * spit;
		for (x = 0; x < swid; x++) {
			//Function to call asm3.asm file
			//Asm3();

			if (mfd->csShiftI == BST_UNCHECKED) {
				//Function to call asm4.asm file
				//Asm4();

			}
			if (mfd->csShiftQ == BST_UNCHECKED) {
				//Function to call asm5.asm file
				//Asm5();

			}

			ho = x - hoff; vo = y - voff;
			if (ho < 0) ho = 0; else if (ho > swid) ho = swid;
			if (vo < 0) vo = 0; else if (vo > shei) vo = shei;
			ofs = isrc + vo * spit + ho;
			//Function to call asm6.asm file
			//Asm6();

			if (mfd->csShiftI == BST_CHECKED) {
				//Function to call asm7.asm file
				//Asm7();
			}
			if (mfd->csShiftQ == BST_CHECKED) {
				//Function to call asm8.asm file
				//Asm8();
			}
			//Function to call asm9.asm file
			//Asm9();			
		}
	}
}

///////////////////////////////////////////////////////////////////////////

void fxVHS_DeNoise(Pixel32 *isrc, Pixel32 *idst, PixDim sw, PixDim sh, long spit, MyFilterData *mfd) {
	Pixel32	*src, *dst, *ofs;
	PixDim		w, h, x, y;
	long		cr, cg, cb, tr, tg, tb;
	long		ar, ag, ab, count;
	long		lf, rt, tp, bt;
	const int	Thresh = mfd->nrThreshold;
	const int	Rad = mfd->nrRadius;

	for (h = 0; h < sh; h++) {
		src = isrc + h * spit;
		dst = idst + h * spit;
		for (w = 0; w < sw; w++) {
			//Function to call asm10.asm file
			//Asm10();
					   
			tp = h - Rad; bt = h + Rad;
			if (tp < 0) tp = 0;				// Allow full-scene denoising w/o
			if (bt >= sh) bt = sh - 1;		// crossing boundaries
			for (y = tp; y < bt; y++) {
				ofs = isrc + y * spit + w - Rad;	// Upper-left of sampling area
				lf = h - Rad; rt = h + Rad;
				if (lf < 0) lf = 0;			// More sampling area cropping
				if (rt >= sw) rt = sw - 1;
				for (x = lf; x < rt; x++) {
					//Function to call asm11.asm file
					//Asm11();
				}
			}
			//Function to call asm12.asm file
			//Asm12();					   
		}
	}
}

///////////////////////////////////////////////////////////////////////////

void fxVHS_Sharpen(Pixel32 *isrc, Pixel32 *idst, PixDim sw, PixDim sh, long spit, MyFilterData *mfd) {
	Pixel32		*src, *dst;
	PixDim		w, h;
	long		cr, cg, cb, tr, tg, tb;
	long		*d5t, *d9t, *scat;

	d5t = (long *)mfd->Div5Tab;
	d9t = (long *)mfd->Div9Tab;
	scat = (long *)mfd->ScaleTab + 256;

	for(h = 1; h < sh - 1; h++) {
		src = isrc + h * spit + 1;
		dst = idst + h * spit + 1;
		if (mfd->sh8Dir == BST_CHECKED) {
			// Eight-Direction sharpening
			for(w = 1; w < sw - 1; w++) {
				//Function to call asm13.asm file
				//Asm13();
			}
		} else {
			// Four-Pixel sharpening
			// This is exactly the same as above, except only the current,
			// left, right, above, and below pixels are sampled
			for(w = 1; w < sw - 1; w++) {
				//Function to call asm14.asm file
				//Asm14();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

int fxVHSStartProc(FilterActivation *fa, const FilterFunctions *ff) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;
	double	scale;
	long	d;

	// Temporary Buffer
	if (!(mfd->TmpBuff = new long[fa->src.pitch * fa->src.h])) return 1;

	// Chroma Buffers for the chroma stabilizer
	d = fa->src.w * fa->src.h;
	if (!(mfd->stBuffY = new long[d])) return 1;
	if (!(mfd->stBuffI = new long[d])) return 1;
	if (!(mfd->stBuffQ = new long[d])) return 1;
	if (!(mfd->stCount = new long[d])) return 1;
	if (!(mfd->stError = new long[d])) return 1;
	d <<= 2;
	memset(mfd->stBuffY, 0, d);
	memset(mfd->stBuffI, 0, d);
	memset(mfd->stBuffQ, 0, d);
	memset(mfd->stCount, 0, d);
	memset(mfd->stError, 0, d);

	if (!(mfd->stlBuffY = new long[d])) return 1;
	if (!(mfd->stlBuffI = new long[d])) return 1;
	if (!(mfd->stlBuffQ = new long[d])) return 1;
	if (!(mfd->stpBuffY = new long[d])) return 1;
	if (!(mfd->stpBuffI = new long[d])) return 1;
	if (!(mfd->stpBuffQ = new long[d])) return 1;
	memset(mfd->stlBuffY, 0, d);
	memset(mfd->stlBuffI, 0, d);
	memset(mfd->stlBuffQ, 0, d);
	memset(mfd->stpBuffY, 0, d);
	memset(mfd->stpBuffI, 0, d);
	memset(mfd->stpBuffQ, 0, d);

	// RGB -> YUV (YIQ) conversion tables
	if (!(mfd->yr = new long[256])) return 1;
	if (!(mfd->yg = new long[256])) return 1;
	if (!(mfd->yb = new long[256])) return 1;

	if (!(mfd->ir = new long[256])) return 1;
	if (!(mfd->ig = new long[256])) return 1;
	if (!(mfd->ib = new long[256])) return 1;

	if (!(mfd->qr = new long[256])) return 1;
	if (!(mfd->qg = new long[256])) return 1;
	if (!(mfd->qb = new long[256])) return 1;

	// YUV (YIQ) -> RGB conversion tables
	if (!(mfd->ri = new long[256])) return 1;
	if (!(mfd->rq = new long[256])) return 1;
	if (!(mfd->gi = new long[256])) return 1;
	if (!(mfd->gq = new long[256])) return 1;
	if (!(mfd->bi = new long[256])) return 1;
	if (!(mfd->bq = new long[256])) return 1;

	// Calculate data for RGB -> YUV conversion tables
	for (d = 0; d < 256; d++) {
		// RGB -> YIQ transformation matrix
		mfd->yr[d] = (long)(+0.299 * d * 65536 + 0.5);
		mfd->yg[d] = (long)(+0.587 * d * 65536 + 0.5);
		mfd->yb[d] = (long)(+0.114 * d * 65536 + 0.5);
		mfd->ir[d] = (long)(+0.596 * d * 65536 + 0.5);
		mfd->ig[d] = (long)(-0.275 * d * 65536 + 0.5);
		mfd->ib[d] = (long)(-0.321 * d * 65536 + 0.5);
		mfd->qr[d] = (long)(+0.212 * d * 65536 + 0.5);
		mfd->qg[d] = (long)(-0.523 * d * 65536 + 0.5);
		mfd->qb[d] = (long)(+0.311 * d * 65536 + 0.5);

		// YIQ -> RGB transformation matrix
		mfd->ri[d] = (long)(+0.956 * (d-128) * 65536 + 0.5);
		mfd->rq[d] = (long)(+0.621 * (d-128) * 65536 + 0.5);
		mfd->gi[d] = (long)(-0.272 * (d-128) * 65536 + 0.5);
		mfd->gq[d] = (long)(-0.647 * (d-128) * 65536 + 0.5);
		mfd->bi[d] = (long)(-1.105 * (d-128) * 65536 + 0.5);
		mfd->bq[d] = (long)(+1.702 * (d-128) * 65536 + 0.5);
	}

	// Division by 3 table
	if (!(mfd->Div3Tab = new long[768])) return 1;
	for (d = 0; d < 768; d++) {
		mfd->Div3Tab[d] = d / 3 - mfd->stTempBias;
		if (mfd->Div3Tab[d] < 0) mfd->Div3Tab[d] = 0;
	}

	// Division by 5 table
	if (!(mfd->Div5Tab = new long[1280])) return 1;
	for (d = 0; d < 1280; d++) {
		mfd->Div5Tab[d] = d / 5 - mfd->stTempBias;
		if (mfd->Div5Tab[d] < 0) mfd->Div5Tab[d] = 0;
	}

	// Division by 9 table
	if (!(mfd->Div9Tab = new long[2304])) return 1;
	for (d = 0; d < 2304; d++) {
		mfd->Div9Tab[d] = d / 9 - mfd->stTempBias;
		if (mfd->Div9Tab[d] < 0) mfd->Div9Tab[d] = 0;
	}

	// Scale Table (for Sharpen Effect level)
	if (!(mfd->ScaleTab = new long[512])) return 1;
	scale = 1.0 / 50.0 * mfd->shEffect;
	for (d = -256; d < 256; d++)
		mfd->ScaleTab[d + 256] = (long)(d * scale + 0.5);

	return 0;
}

///////////////////////////////////////////////////////////////////////////

int fxVHSEndProc(FilterActivation *fa, const FilterFunctions *ff) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	// Delete temporary frame buffer
	delete[] mfd->TmpBuff; mfd->TmpBuff = NULL;

	// Delete RGB -> YUV conversion tables
	delete[] mfd->yr; mfd->yr = NULL;
	delete[] mfd->yg; mfd->yg = NULL;
	delete[] mfd->yb; mfd->yb = NULL;

	delete[] mfd->ir; mfd->ir = NULL;
	delete[] mfd->ig; mfd->ig = NULL;
	delete[] mfd->ib; mfd->ib = NULL;

	delete[] mfd->qr; mfd->qr = NULL;
	delete[] mfd->qg; mfd->qg = NULL;
	delete[] mfd->qb; mfd->qb = NULL;

	delete[] mfd->ri; mfd->ri = NULL;
	delete[] mfd->rq; mfd->rq = NULL;
	delete[] mfd->gi; mfd->gi = NULL;
	delete[] mfd->gq; mfd->gq = NULL;
	delete[] mfd->bi; mfd->bi = NULL;
	delete[] mfd->bq; mfd->bq = NULL;

	delete[] mfd->stBuffY; mfd->stBuffY = NULL;
	delete[] mfd->stBuffI; mfd->stBuffI = NULL;
	delete[] mfd->stBuffQ; mfd->stBuffQ = NULL;
	delete[] mfd->stlBuffY; mfd->stlBuffY = NULL;
	delete[] mfd->stlBuffI; mfd->stlBuffI = NULL;
	delete[] mfd->stlBuffQ; mfd->stlBuffQ = NULL;
	delete[] mfd->stpBuffY; mfd->stpBuffY = NULL;
	delete[] mfd->stpBuffI; mfd->stpBuffI = NULL;
	delete[] mfd->stpBuffQ; mfd->stpBuffQ = NULL;
	delete[] mfd->stCount; mfd->stCount = NULL;
	delete[] mfd->stError; mfd->stError = NULL;

	delete[] mfd->Div3Tab; mfd->Div3Tab = NULL;
	delete[] mfd->Div5Tab; mfd->Div5Tab = NULL;
	delete[] mfd->Div9Tab; mfd->Div9Tab = NULL;
	delete[] mfd->ScaleTab; mfd->ScaleTab = NULL;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

int fxVHSInitProc(FilterActivation *fa, const FilterFunctions *ff) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	mfd->stEnable = BST_CHECKED;
	mfd->stLumaThresh = 5;
	mfd->stChromaThresh = 30;
	mfd->stTempError = 15;
	mfd->stTempBias = 5;
	mfd->nrThreshold = 10;
	mfd->nrRadius = 1;
	mfd->nrPreFilter = BST_UNCHECKED;
	mfd->nrPostFilter = BST_UNCHECKED;
	mfd->shEnable = BST_UNCHECKED;
	mfd->shEffect = 15;
	mfd->sh8Dir = BST_UNCHECKED;
	mfd->csHorizontal = 1;
	mfd->csHorizNeg = BST_CHECKED;
	mfd->csVertical = 0;
	mfd->csVertNeg = BST_UNCHECKED;
	mfd->csShiftI = BST_UNCHECKED;
	mfd->csShiftQ = BST_UNCHECKED;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK fxVHSConfigDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	MyFilterData *mfd = (MyFilterData *)GetWindowLong(hdlg, DWLP_USER);

	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(hdlg, DWLP_USER, lParam);
		mfd = (MyFilterData *)lParam;
		CheckDlgButton(hdlg, IDC_ST_ENABLE, mfd->stEnable ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hdlg, IDC_ST_LUMA, mfd->stLumaThresh, FALSE);
		SetDlgItemInt(hdlg, IDC_ST_CHROMA, mfd->stChromaThresh, FALSE);
		SetDlgItemInt(hdlg, IDC_ST_ERROR, mfd->stTempError, FALSE);
		SetDlgItemInt(hdlg, IDC_ST_BIAS, mfd->stTempBias, FALSE);
		SetDlgItemInt(hdlg, IDC_NR_THRESH, mfd->nrThreshold, FALSE);
		SetDlgItemInt(hdlg, IDC_NR_RADIUS, mfd->nrRadius, FALSE);
		CheckDlgButton(hdlg, IDC_NR_PRE, mfd->nrPreFilter ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_NR_POST, mfd->nrPostFilter ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hdlg, IDC_SH_EFFECT, mfd->shEffect, FALSE);
		CheckDlgButton(hdlg, IDC_SH_ENABLE, mfd->shEnable ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_SH_8DIR, mfd->sh8Dir ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hdlg, IDC_CS_H, mfd->csHorizontal, FALSE);
		CheckDlgButton(hdlg, IDC_CS_HNEG, mfd->csHorizNeg ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hdlg, IDC_CS_V, mfd->csVertical, FALSE);
		CheckDlgButton(hdlg, IDC_CS_VNEG, mfd->csVertNeg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CS_SHI, mfd->csShiftI ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CS_SHQ, mfd->csShiftQ ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			mfd->stEnable = !!IsDlgButtonChecked(hdlg, IDC_ST_ENABLE);
			mfd->stLumaThresh = GetDlgItemInt(hdlg, IDC_ST_LUMA, &mfd->stLumaThresh, FALSE);
			mfd->stChromaThresh = GetDlgItemInt(hdlg, IDC_ST_CHROMA, &mfd->stChromaThresh, FALSE);
			mfd->stTempError = GetDlgItemInt(hdlg, IDC_ST_ERROR, &mfd->stTempError, FALSE);
			mfd->stTempBias = GetDlgItemInt(hdlg, IDC_ST_BIAS, &mfd->stTempBias, FALSE);
			mfd->nrThreshold = GetDlgItemInt(hdlg, IDC_NR_THRESH, &mfd->nrThreshold, FALSE);
			mfd->nrRadius = GetDlgItemInt(hdlg, IDC_NR_RADIUS, &mfd->nrRadius, FALSE);
			mfd->nrPreFilter = !!IsDlgButtonChecked(hdlg, IDC_NR_PRE);
			mfd->nrPostFilter = !!IsDlgButtonChecked(hdlg, IDC_NR_POST);
			mfd->shEffect = GetDlgItemInt(hdlg, IDC_SH_EFFECT, &mfd->shEffect, FALSE);
			mfd->shEnable = !!IsDlgButtonChecked(hdlg, IDC_SH_ENABLE);
			mfd->sh8Dir = !!IsDlgButtonChecked(hdlg, IDC_SH_8DIR);
			mfd->csHorizontal = GetDlgItemInt(hdlg, IDC_CS_H, &mfd->csHorizontal, FALSE);
			mfd->csHorizNeg = !!IsDlgButtonChecked(hdlg, IDC_CS_HNEG);
			mfd->csVertical = GetDlgItemInt(hdlg, IDC_CS_V, &mfd->csVertical, FALSE);
			mfd->csVertNeg = !!IsDlgButtonChecked(hdlg, IDC_CS_VNEG);
			mfd->csShiftI = !!IsDlgButtonChecked(hdlg, IDC_CS_SHI);
			mfd->csShiftQ = !!IsDlgButtonChecked(hdlg, IDC_CS_SHQ);
			EndDialog(hdlg, 0);

			if (mfd->stLumaThresh > 255) mfd->stLumaThresh = 255;
			if (mfd->stChromaThresh > 255) mfd->stChromaThresh = 255;
			if (mfd->stTempError == 0) mfd->stTempError = 1;
			if (mfd->stTempBias > 255) mfd->stTempBias = 255;
			if (mfd->nrThreshold > 255) mfd->nrThreshold = 255;
			if (mfd->nrRadius > 30) mfd->nrRadius = 30;
			if (mfd->nrRadius < 1) mfd->nrRadius = 1;
			if (mfd->shEffect > 100) mfd->shEffect = 100;

			return TRUE;
		case IDCANCEL:
			EndDialog(hdlg, 1);
			return FALSE;
		}
		break;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////

int fxVHSConfigProc(FilterActivation *fa, const FilterFunctions *ff, HWND hwnd) {
	return DialogBoxParam(fa->filter->module->hInstModule,
	       MAKEINTRESOURCE(IDD_FXVHS_CONFIG), hwnd,
		   fxVHSConfigDlgProc, (LPARAM)fa->filter_data);
}

///////////////////////////////////////////////////////////////////////////

void fxVHSStringProc(const FilterActivation *fa, const FilterFunctions *ff, char *str) {
	MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	sprintf(str, " st:%s,%d,%d,%d,%d nr:%d,%d,%s,%s sh:%s,%d,%d cs:%s%d,%s%d,%s%s",
		mfd->stEnable == BST_CHECKED ? "Y" : "N", mfd->stLumaThresh, mfd->stChromaThresh, 
		mfd->stTempError, mfd->stTempBias, mfd->nrThreshold, mfd->nrRadius, 
		mfd->nrPreFilter == BST_CHECKED ? "Y" : "N", mfd->nrPostFilter == BST_CHECKED ? "Y" : "N", 
		mfd->shEnable == BST_CHECKED ? "Y" : "N", mfd->shEffect, mfd->sh8Dir == BST_CHECKED ? 8 : 4,
		mfd->csHorizNeg == BST_CHECKED ? "-" : "", mfd->csHorizontal,
		mfd->csVertNeg == BST_CHECKED ? "-" : "", mfd->csVertical,
		mfd->csShiftI == BST_CHECKED ? "I" : "", mfd->csShiftQ == BST_CHECKED ? "Q" : "");
}

///////////////////////////////////////////////////////////////////////////

void fxVHSScriptConfig(IScriptInterpreter *isi, void *lpVoid, CScriptValue *argv, int argc) {
    FilterActivation *fa = (FilterActivation *)lpVoid;
    MyFilterData *mfd = (MyFilterData *)fa->filter_data;

	mfd->stEnable = argv[0].asInt();
	mfd->stLumaThresh = argv[1].asInt();
	mfd->stChromaThresh = argv[2].asInt();
	mfd->stTempError = argv[3].asInt();
	mfd->stTempBias = argv[4].asInt();
	mfd->nrThreshold = argv[5].asInt();
	mfd->nrRadius = argv[6].asInt();
	mfd->nrPreFilter = argv[7].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
	mfd->nrPostFilter = argv[8].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
	mfd->shEnable = argv[9].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
	mfd->shEffect = argv[10].asInt();
	mfd->sh8Dir = argv[11].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
	mfd->csHorizontal = argv[12].asInt();
	mfd->csHorizNeg = mfd->csHorizontal < 0 ? BST_CHECKED: BST_UNCHECKED;
	mfd->csHorizontal = abs(mfd->csHorizontal);
	mfd->csVertical = argv[13].asInt();
	mfd->csVertNeg = mfd->csVertical < 0 ? BST_CHECKED: BST_UNCHECKED;
	mfd->csVertical = abs(mfd->csVertical);
	mfd->csShiftI = argv[14].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
	mfd->csShiftQ = argv[15].asInt() == 1 ? BST_CHECKED : BST_UNCHECKED;
}

///////////////////////////////////////////////////////////////////////////

bool fxVHSFssProc(FilterActivation *fa, const FilterFunctions *ff, char *buf, int buflen) {
    MyFilterData *mfd = (MyFilterData *)fa->filter_data;

    _snprintf_s(buf, buflen, _TRUNCATE, "Config(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		mfd->stEnable == BST_CHECKED ? 1 : 0, mfd->stLumaThresh, mfd->stChromaThresh, 
		mfd->stTempError, mfd->stTempBias, mfd->nrThreshold, mfd->nrRadius, 
		mfd->nrPreFilter == BST_CHECKED ? 1 : 0, mfd->nrPostFilter == BST_CHECKED ? 1 : 0,
		mfd->shEnable == BST_CHECKED ? 1 : 0, mfd->shEffect, mfd->sh8Dir == BST_CHECKED ? 1 : 0,
		mfd->csHorizontal * (mfd->csHorizNeg == BST_CHECKED ? -1 : 1),
		mfd->csVertical * (mfd->csVertNeg == BST_CHECKED ? -1 : 1),
		mfd->csShiftI == BST_CHECKED ? 1 : 0, mfd->csShiftQ == BST_CHECKED ? 1 : 0);

    return true;
}