// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.

@8192
D=A
@SCREEN
D=D+A
@max_add
M=D             //holds the address of the word right after the end of the screen memory map
                //  (since it's outside the map, it shouldn't be modified)

(MAIN_LOOP)
    @KBD            
    D=M             
    @TURN_OFF
    D;JEQ           //if KBD is 0, jump to TURN_OFF to set val to 0
                    //otherwise, continue downwards to set val to -1

    @val
    M=-1            //variable val, used to turn the pixels on (val=-1) or off (val=0)

    (RESET_ADDRESS)     //setting current address to SCREEN
    @SCREEN
    D=A
    @curadd
    M=D                 

    (LOOP_CHANGE_COLOR)     //loop to change the screen color word by word
        @val
        D=M                 
        @curadd
        A=M
        M=D                 //changing color of the current address
        @curadd
        M=M+1
        D=M                 //incrementing current address
        @max_add
        D=M-D           
        @LOOP_CHANGE_COLOR
        D;JNE               //when current address reaches highest address (max_add), loop immediately terminates

    @MAIN_LOOP
    0;JMP               //unconditional jump back to MAIN_LOOP to keep checking keyboard input

    (TURN_OFF)
    @val
    M=0
    @RESET_ADDRESS
    0;JMP