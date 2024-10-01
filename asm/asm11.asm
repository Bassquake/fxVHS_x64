PUBLIC asm11
EXTERN puts:PROC
.code
asm11 PROC
						mov		ebx, [ofs]	// Read sampled pixel
						mov		eax, [ebx]
						add		[ofs], 4
						mov		ecx, eax
						and		ecx, 0x00FF
						mov		[tb], ecx
						shr		eax, 8
						mov		ecx, eax
						and		ecx, 0x00FF
						mov		[tg], ecx
						shr		eax, 8
						and		eax, 0x00FF
						mov		[tr], eax

						// Find absolute distance from sample pixel and current pixel
						mov		edx, [Thresh]
						mov		eax, [tr]
						sub		eax, [cr]
						jns		dnc10
						neg		eax
dnc10:					cmp		eax, edx
						jg		dni10		// If out of threshold, jump out (failed IF)

						mov		eax, [tg]	// Do the same for each of R, G, and B
						sub		eax, [cg]
						jns		dnc20
						neg		eax
dnc20:					cmp		eax, edx
						jg		dni10

						mov		eax, [tb]
						sub		eax, [cb]
						jns		dnc30
						neg		eax
dnc30:					cmp		eax, edx
						jg		dni10

						mov		eax, [tr]	// If all values are within the threshold,
						add		[ar], eax	// add sample to sampling registers
						mov		eax, [tg]
						add		[ag], eax
						mov		eax, [tb]
						add		[ab], eax
						inc		[count]		// Increment sample count
dni10:				
Asm10 ENDP
END