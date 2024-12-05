			sub_1		proc	near
; ���������� �������� ��������� DS, AX
020A:07B9  1E			push	ds
020A:07BA  50			push	ax
; ������������� DS ��������� 0040h (������� ������ ������� ������ BIOS)
020A:07BB  B8 0040			mov	ax,40h
020A:07BE  8E D8			mov	ds,ax
; �������� �������� ����� FLAGS � ������� AH
020A:07C0  9F			lahf				; Load ah from flags
; ��������: ������ �� ���� �� ���� �� ������ 10 ��� 13 
020A:07C1  F7 06 0314 2400	test	word ptr ds:[314h],2400h	; (0040:0314=3200h)
; ���� ������ ���� �� ����, �� ������� �� loc_22, ����� �������� cli �������� 
020A:07C7  75 0C			jnz	loc_22			; Jump if not zero 
020A:07C9  F0> 81 26 0314 FDFF	  lock and	word ptr ds:[314h],0FDFFh	; (0040:0314=3200h)
020A:07D0		loc_21:
; �������������� �������� ������ SF, ZF, AF, PF � CF �������� FLAGS �� AH
020A:07D0  9E			sahf				; Store ah into flags
; �������������� �������� ��������� AX, DS
020A:07D1  58			pop	ax
020A:07D2  1F			pop	ds
; ������� �� loc_23
020A:07D3  EB 03			jmp	short loc_23		; (07D8)
020A:07D5		loc_22:
; ����� IF 
020A:07D5  FA			cli				; Disable interrupts
020A:07D6  EB F8			jmp	short loc_21		; (07D0)
; ����� �� ������������
020A:07D8		loc_23:
020A:07D8  C3			retn
			sub_1		endp