PUBLIC Asm1
EXTERN puts:PROC
.code
Asm1 PROC
					mov		ebx, [src]
					add		[src], 4
					mov		eax, [ebx]
					mov		ecx, eax
					and		ecx, 0x0000FF
					shl		ecx, 2
					mov		[tb], ecx
					mov		ecx, eax
					and		ecx, 0x00FF00
					shr		ecx, 6
					mov		[tg], ecx
					and		eax, 0xFF0000
					shr		eax, 14
					mov		[tr], eax

					mov		ebx, [yr]			// Calculate Luma(Y)
					add		ebx, [tr]
					mov		eax, [ebx]
					mov		ebx, [yg]
					add		ebx, [tg]
					add		eax, [ebx]
					mov		ebx, [yb]
					add		ebx, [tb]
					add		eax, [ebx]
					test	eax, 0xFF000000
					jz		cs11
					jns		cs10
					xor		eax, eax
					jmp		cs12
cs10:				mov		eax, 0x00FF0000
cs11:				shr		eax, 16
cs12:				mov		[cy], eax

					mov		ebx, [ir]			// Calculate Chroma(I)
					add		ebx, [tr]
					mov		eax, [ebx]
					mov		ebx, [ig]
					add		ebx, [tg]
					add		eax, [ebx]
					mov		ebx, [ib]
					add		ebx, [tb]
					add		eax, [ebx]
					add		eax, 8388608
					test	eax, 0xFF000000
					jz		cs21
					jns		cs20
					xor		eax, eax
					jmp		cs22
cs20:				mov		eax, 0x00FF0000
cs21:				shr		eax, 16
cs22:				mov		[ci], eax

					mov		ebx, [qr]			// Calculate Chroma(Q)
					add		ebx, [tr]
					mov		eax, [ebx]
					mov		ebx, [qg]
					add		ebx, [tg]
					add		eax, [ebx]
					mov		ebx, [qb]
					add		ebx, [tb]
					add		eax, [ebx]
					add		eax, 8388608

					test	eax, 0xFF000000
					jz		cs31
					jns		cs30
					xor		eax, eax
					jmp		cs32
cs30:				mov		eax, 0x00FF0000
cs31:				shr		eax, 16
cs32:				mov		[cq], eax
Asm1 ENDP
END
