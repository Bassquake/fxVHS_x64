PUBLIC asm9
EXTERN puts:PROC
.code
asm9 PROC
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
				jz		scv11
				jns		scv10
				xor		eax, eax
				jmp		scv12
scv10:			mov		eax, 0x00FF0000
scv11:			shr		eax, 16
scv12:			mov		[tr], eax

				mov		eax, edx			// Extract Green from YIQ
				mov		ebx, [gi]
				add		ebx, [ci]
				add		eax, [ebx]
				mov		ebx, [gq]
				add		ebx, [cq]
				add		eax, [ebx]
				test	eax, 0xFF000000
				jz		scv21
				jns		scv20
				xor		eax, eax
				jmp		scv22
scv20:			mov		eax, 0x00FF0000
scv21:			shr		eax, 16
scv22:			mov		[tg], eax

				mov		eax, edx			// Extract Green from YIQ
				mov		ebx, [bi]
				add		ebx, [ci]
				add		eax, [ebx]
				mov		ebx, [bq]
				add		ebx, [cq]
				add		eax, [ebx]
				test	eax, 0xFF000000
				jz		scv31
				jns		scv30
				xor		eax, eax
				jmp		scv32
scv30:			mov		eax, 0x00FF0000
scv31:			shr		eax, 16
scv32:			mov		[tb], eax

				mov		ebx, [dst]
				add		[dst], 4
				mov		eax, [tr]
				shl		eax, 8
				or		eax, [tg]
				shl		eax, 8
				or		eax, [tb]
				mov		[ebx], eax
Asm9 ENDP
END