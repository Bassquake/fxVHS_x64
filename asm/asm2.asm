PUBLIC asm2
EXTERN puts:PROC
.code
asm2 PROC
					mov		edx, [cy]			// Prepare components for indexing
					shl		edx, 16
					shl		[ci], 2
					shl		[cq], 2
					mov		eax, edx			// Extract Red from YIQ
					mov		ebx, [ri]
					add		ebx, [ci]
					add		eax, [ebx]
					mov		ebx, [rq]
					add		ebx, [cq]
					add		eax, [ebx]
					test	eax, 0xFF000000
					jz		cv11
					jns		cv10
					xor		eax, eax
					jmp		cv12
cv10:				mov		eax, 0x00FF0000
cv11:				shr		eax, 16
cv12:				mov		[cr], eax

					mov		eax, edx			// Extract Green from YIQ
					mov		ebx, [gi]
					add		ebx, [ci]
					add		eax, [ebx]
					mov		ebx, [gq]
					add		ebx, [cq]
					add		eax, [ebx]
					test	eax, 0xFF000000
					jz		cv21
					jns		cv20
					xor		eax, eax
					jmp		cv22
cv20:				mov		eax, 0x00FF0000
cv21:				shr		eax, 16
cv22:				mov		[cg], eax

					mov		eax, edx			// Extract Green from YIQ
					mov		ebx, [bi]
					add		ebx, [ci]
					add		eax, [ebx]
					mov		ebx, [bq]
					add		ebx, [cq]
					add		eax, [ebx]
					test	eax, 0xFF000000
					jz		cv31
					jns		cv30
					xor		eax, eax
					jmp		cv32
cv30:				mov		eax, 0x00FF0000
cv31:				shr		eax, 16
cv32:				mov		[cb], eax

					// Store new pixel in the destination frame buffer
					mov		ebx, [dst]
					add		[dst], 4
					mov		eax, [cr]
					shl		eax, 8
					or		eax, [cg]
					shl		eax, 8
					or		eax, [cb]
					mov		[ebx], eax
Asm2 ENDP
END