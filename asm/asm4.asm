PUBLIC asm4
EXTERN puts:PROC
.code
asm4 PROC
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
					jz		scs21
					jns		scs20
					xor		eax, eax
					jmp		scs22
scs20:				mov		eax, 0x00FF0000
scs21:				shr		eax, 16
scs22:				mov		[ci], eax
Asm4 ENDP
END