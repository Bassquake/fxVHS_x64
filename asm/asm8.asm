PUBLIC asm8
EXTERN puts:PROC
.code
asm8 PROC
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
					jz		scs51
					jns		scs50
					xor		eax, eax
					jmp		scs52
scs50:				mov		eax, 0x00FF0000
scs51:				shr		eax, 16
scs52:				mov		[cq], eax
Asm8 ENDP
END