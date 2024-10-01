PUBLIC asm5
EXTERN puts:PROC
.code
asm5 PROC
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
					jz		scs31
					jns		scs30
					xor		eax, eax
					jmp		scs32
scs30:				mov		eax, 0x00FF0000
scs31:				shr		eax, 16
scs32:				mov		[cq], eax
Asm5 ENDP
END