PUBLIC Asm1
EXTERN puts:PROC
.code
Asm10 PROC
				// Read and store color in current and sampled area registers
				mov		ebx, [src]
				mov		eax, [ebx]
				add		[src], 4
				mov		ecx, eax
				and		ecx, 0x00FF
				mov		[cb], ecx
				mov		[ab], ecx
				shr		eax, 8
				mov		ecx, eax
				and		ecx, 0x00FF
				mov		[cg], ecx
				mov		[ag], ecx
				shr		eax, 8
				and		eax, 0x00FF
				mov		[cr], eax
				mov		[ar], eax
				mov		[count], 1
Asm10 endp
END