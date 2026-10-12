.text
main:
    li x5, 0
    li x6, 3
loop:
    addi x5, x5, 1
    bne  x5, x6, loop   
    add x7, x5, x5
    add x8, x5, x5
    
    