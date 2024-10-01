PUBLIC asm13
EXTERN puts:PROC
.code
asm13 PROC
					// Read current pixel and copy to softened samples
					mov		ebx, [src]
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

					// Retrieve all 8 surrounding pixels and accumulate them
					mov		eax, [ebx - 4]	// Left
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + 4]	// Right
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
					mov		eax, [ebx + edx]	// Above
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + edx - 4]	// Above - Left
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + edx + 4]	// Above - Right
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					neg		edx
					mov		eax, [ebx + edx]		// Below
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + edx - 4]	// Below - Left
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		eax, [ebx + edx + 4]	// Below - Right
					mov		cl, al
					add		[tb], ecx
					mov		cl, ah
					shr		eax, 16
					add		[tg], ecx
					mov		cl, al
					add		[tr], ecx

					mov		ebx, [d9t]		// Divide accumulated values by 9
					mov		eax, [tr]		// using precalculated table
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

					mov		edx, [scat]		// Apply scaling to pixel (Sharpen Effect)
					mov		ecx, [cr]
					mov		eax, ecx
					sub		eax, [tr]
					jns		she10
					neg		eax
she10:				shl		eax, 2
					mov		eax, [eax + edx]
					add		ecx, eax
					test	ch, ch			// Clamp value within 0 - 255
					jz		she12
					js		she11
					mov		ecx, 0xFF00
					jmp		she13
she11:				xor		ecx, ecx
she12:				shl		ecx, 8
she13:
					mov		eax, [cg]
					mov		ebx, eax
					sub		ebx, [tg]
					jns		she20
					neg		ebx
she20:				shl		ebx, 2
					mov		ebx, [ebx + edx]
					add		eax, ebx
					test	ah, ah			// Clamp value within 0 - 255
					jz		she21
					js		she22
					or		ecx, 0x00FF
					jmp		she22
she21:				or		ecx, eax
she22:				shl		ecx, 8

					mov		eax, [cb]
					mov		ebx, eax
					sub		ebx, [tb]
					jns		she30
					neg		ebx
she30:				shl		ebx, 2
					mov		ebx, [ebx + edx]
					add		eax, ebx
					test	ah, ah			// Clamp value within 0 - 255
					jz		she31
					js		she32
					or		ecx, 0x00FF
					jmp		she32
she31:				or		ecx, eax
she32:
					mov		ebx, [dst]		// Store calculated pixel and
					mov		[ebx], ecx		// increment pointers
					add		[dst], 4
					add		[src], 4
Asm13 ENDP
END