PUBLIC asm12
EXTERN puts:PROC
.code
asm12 PROC
				// Divide sample registers by sample count (to get average)
				mov		eax, [ar]
				xor		edx, edx
				mov		ebx, [count]
				div		ebx
				mov		ecx, eax
				shl		ecx, 8
				mov		eax, [ag]
				xor		edx, edx
				div		ebx
				or		ecx, eax
				shl		ecx, 8
				mov		eax, [ab]
				xor		edx, edx
				div		ebx
				or		ecx, eax

				// Store sample as on-screen pixel.
				mov		ebx, [dst]
				mov		[ebx], ecx
				add		[dst], 4
Asm12 ENDP
END