    .global Sum
    .text
    .align 4

Sum:
    mov     x2, xzr
    add     x1, x0, x1, lsl 3
    b       2f

1:  ldr     x3, [x0], 8
    add     x2, x2, x3
2:  cmp     x0, x1
    blt     1b

    mov     x0, x2
    ret

    .end
