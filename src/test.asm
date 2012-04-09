;test.asm

.org 0

starting:
  ld a, 3 + 2 -     4*2
  xor a ; xx
  ld hl, 4
label1:
label2:
  add a,b
  add a, 15 ;(3*5) no preprocessing yet
  ret;fuck this
  ld	b	,	5
  .db 0, 1, 3, 4, 5, 6, 7, 8

;David Phillip's RLE decompress routine
DispRLE:
 ld bc,1024			; we need to copy 
DispRLEL:
 ld a,(hl)			; get the next byte
 cp $91				; is it a run?
 jr z,DispRLERun	; then we need to decode the run
 ldi				; copy the byte, and update counters
DispRLEC:
 ld a,b				; check the low byte and
 or c				; the high byte for 0
 jr nz,DispRLEL		; if not, then we're not done either
 ret				; if it's zero, we're done
DispRLERun:
 inc hl				; move to the run value
 ld a,(hl)			; get the run value
 inc hl				; move to the run count
 push hl			; save source pointer
 ld h,(hl)			; get the run count
 ex de,hl			; swap source and destination pointers
DispRLERunL:
 ld (hl),a			; copy the byte
 inc hl				; increase destination pointer
 dec bc				; decrease byte count
 dec d				; decrease run count
 jr nz,DispRLERunL	; if we're not done, then loop
 ex de,hl			; swap pointers back
 pop hl				; recover source pointer
 inc hl				; advance the source pointer
 jr DispRLEC		; check to see if we should loop



