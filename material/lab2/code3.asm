.data
val: .word 42
.text
main:
    la x10, val
    lw x5, 0(x10)       
    add x6, x5, x7      
    addi x8, x9, 1      
    addi x11, x12, 2    
    
    