;test.asm

 .org 0

starting:
  ld a, 3
  xor a ; xx
  ld hl, 4
label1:
label2:
  add a,b
  add a, 15 ;(3*5) no preprocessing yet
  ret;fuck this
  ld	b	,	5
  .db 0, 1, 3, 4, 5, 6, 7, 8

