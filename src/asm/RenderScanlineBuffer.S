
# void RenderScanlineBuffer(unsigned char* scanlineSrc, unsigned int* scanlineDest, uint16* workingPalette)
// r4 = unsigned int* scanlineSrc; (1 bit shifted color palette index)
// r5 = unsigned int* scanlineDest;

// symbol = uint16* ppu_workingPalette 

// r0 - r3 = scratch

    .global _RenderScanlineBuffer_ASM
    .text; .align 4
_RenderScanlineBuffer_ASM:
	// r2 = ppu_workingPalette
	mov.l .WorkingPalette, r2
	mov.l @r2, r2

	// loop
	bra _DoRender
	mov #15,r3

_DoRenderAgain:
	add #32,r5
_DoRender:

	// r0 = ppu_workingPalette[scanlineSrc[x]]
	mov.b @r4+,r0		// r0 = *scanlineSrc++
	mov.w @(r0,r2),r0	// r0 = ppu_workingPalette[*scanlineSrc]
	mov.w r0,@r5		// scanlineDest = r0
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(2,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(4,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0 
	mov.w r0,@(6,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(8,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(10,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(12,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(14,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(16,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(18,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(20,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(22,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(24,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(26,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(28,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0

	dt r3
	bf/s _DoRenderAgain
	mov.w r0,@(30,r5)

	rts
	nop

# void RenderScanlineBufferWide1(unsigned char* scanlineSrc, unsigned int* scanlineDest)
// r4 = unsigned int* scanlineSrc; (1 bit shifted color palette index)
// r5 = unsigned int* scanlineDest;
// symbol = uint16* ppu_workingPalette 

// r0 - r3 = scratch

    .global _RenderScanlineBufferWide1_ASM
    .text; .align 4
_RenderScanlineBufferWide1_ASM:
	// r2 = ppu_workingPalette
	mov.l .WorkingPalette, r2
	mov.l @r2, r2

	// loop
	bra _DoRenderWide1
	mov #30,r3

_DoRenderAgainWide1:
	add #24,r5
_DoRenderWide1:

	// r0 = ppu_workingPalette[scanlineSrc[x]]
	mov.b @r4+,r0		// r0 = *scanlineSrc++
	mov.w @(r0,r2),r0	// r0 = ppu_workingPalette[*scanlineSrc]
	mov.w r0,@r5		// scanlineDest = r0
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(2,r5)
	mov.w r0,@(4,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0 
	mov.w r0,@(6,r5)
	mov.w r0,@(8,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(10,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(12,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(14,r5)
	mov.w r0,@(16,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(18,r5)
	mov.w r0,@(20,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	
	dt r3
	bf/s _DoRenderAgainWide1
	mov.w r0,@(22,r5)

	rts
	nop
	
    .global _RenderScanlineBufferWide2_ASM
    .text; .align 4
_RenderScanlineBufferWide2_ASM:
	// r2 = ppu_workingPalette
	mov.l .WorkingPalette, r2
	mov.l @r2, r2

	// loop
	bra _DoRenderWide2
	mov #30,r3

_DoRenderAgainWide2:
	add #24,r5
_DoRenderWide2:
	// r0 = ppu_workingPalette[scanlineSrc[x]]
	mov.b @r4+,r0		// r0 = *scanlineSrc++
	mov.w @(r0,r2),r0	// r0 = ppu_workingPalette[*scanlineSrc]
	mov.w r0,@r5		// scanlineDest = r0
	mov.w r0,@(2,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(4,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0 
	mov.w r0,@(6,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(8,r5)
	mov.w r0,@(10,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(12,r5)
	mov.w r0,@(14,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(16,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(18,r5)
	
	mov.b @r4+,r0
	mov.w @(r0,r2),r0
	mov.w r0,@(20,r5)
	
	dt r3
	bf/s _DoRenderAgainWide2
	mov.w r0,@(22,r5)

	rts
	nop

    .text; .align 4
.WorkingPalette:
	.long	_ppu_workingPalette

.END