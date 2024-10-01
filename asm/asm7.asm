PUBLIC asm7
EXTERN puts:PROC
.code
asm7 PROC
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
					jz		scs41
					jns		scs40
					xor		eax, eax
					jmp		scs42
scs40:				mov		eax, 0x00FF0000
scs41:				shr		eax, 16
scs42:				mov		[ci], eax
Asm7 ENDP
END