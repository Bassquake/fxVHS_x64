PUBLIC asm14
EXTERN puts:PROC
.code
asm14 PROC
					mov		ebx, [src]		// Read current pixel and copy to softened pixel
					mov		eax, [ebx]
					xor		ecx, ecx
					mov		cl, al
					mov		[cb], ecx
					mov		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					mov		[cg], ecx
					mov		[tg], ecx
					mov		cl, al
					mov		[cr], ecx
					mov		[tr], ecx

					mov		eax, [ebx - 4]	// Read pixel to the left
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + 4]	// And the one on the right
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		edx, [spit]
					shl		edx, 2
					neg		edx
					mov		eax, [ebx + edx]	// The one above
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					neg		edx
					mov		eax, [ebx + edx]	// And the one below
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		ebx, [d5t]		// Divide values by 5
					mov		eax, [tr]
					shl		eax, 2
					mov		eax, [ebx + eax]
					mov		[tr], eax
					mov		eax, [tg]
					shl		eax, 2
					mov		eax, [ebx + eax]
					mov		[tg], eax
					mov		eax, [tb]
					shl		eax, 2
					mov		eax, [ebx + eax]
					mov		[tb], eax

					mov		edx, [scat]		// Apply scale (Sharpen Effect)
					mov		ecx, [cr]
					mov		eax, ecx
					sub		eax, [tr]
					jns		shf10
					neg		eax
shf10:				shl		eax, 2
					mov		eax, [eax + edx]
					add		ecx, eax
					test	ch, ch		// Clamp value within 0 - 255
					jz		shf12
					js		shf11
					mov		ecx, 0xFF00
					jmp		shf13
shf11:				xor		ecx, ecx
shf12:				shl		ecx, 8
shf13:
					mov		eax, [cg]
					mov		ebx, eax
					sub		ebx, [tg]
					jns		shf20
					neg		ebx
shf20:				shl		ebx, 2
					mov		ebx, [ebx + edx]
					add		eax, ebx
					test	ah, ah		// Clamp value within 0 - 255
					jz		shf21
					js		shf22
					or		ecx, 0x00FF
					jmp		shf22
shf21:				or		ecx, eax
shf22:				shl		ecx, 8

					mov		eax, [cb]
					mov		ebx, eax
					sub		ebx, [tb]
					jns		shf30
					neg		ebx
shf30:				shl		ebx, 2
					mov		ebx, [ebx + edx]
					add		eax, ebx
					test	ah, ah		// Clamp value within 0 - 255
					jz		shf31
					js		shf32
					or		ecx, 0x00FF
					jmp		shf32
shf31:				or		ecx, eax
shf32:
					mov		ebx, [dst]	// Store new pixel and increment pointers
					mov		[ebx], ecx
					add		[dst], 4
					add		[src], 4
Asm14 ENDP
END