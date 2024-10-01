PUBLIC asm6
EXTERN puts:PROC
.code
asm6 PROC
				mov		ebx, [ofs]
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
Asm6 ENDP
END