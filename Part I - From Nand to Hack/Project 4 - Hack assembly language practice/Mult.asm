// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
// The algorithm is based on repetitive addition.

@R0
D=M
@x
M=D
@R1
D=M
@y
M=D         //Read x and y values (x=ram0, y=ram1)
@R2
M=0         //set ram2 to 0

@x
D=M-D
@SWAP
D;JLT       //if x<y, swap x and y. For efficiency purposes

(ADD_LOOP)
    @y
    D=M
    @END
    D;JEQ       //jump to END if y=0
                //otherwise continue down to add x to ram2 and decrement y

    @y          
    M=M-1       
    @x
    D=M
    @R2
    M=D+M
    @ADD_LOOP
    0;JMP

(SWAP)
@x
D=M
@R2
M=D         //using ram2 as temp here
@y
D=M
@x
M=D
@R2
D=M
M=0         //resetting ram2 to 0
@y
M=D
@ADD_LOOP
0;JMP       //moving to the addition loop

(END)
@END
0;JMP

//Warning: 16 bit numbers are only capable of expressing values from -32768 to +32767 (inclusive)
//If result of arithmetic exceeds these limits overflow occurs and incorrect values are seen

