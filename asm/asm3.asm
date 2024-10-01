PUBLIC asm3
EXTERN puts:PROC
.code
asm3 PROC
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
				mov		ebx, eax
				test	ebx, 0xFF000000
				jz		scs11
				jns		scs10
				xor		ebx, ebx
				jmp		scs12
scs10:			mov		ebx, 0x00FF0000
scs11:			shr		ebx, 16
scs12:			mov		[cy], ebx
Asm3 ENDP
END