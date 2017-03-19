#ifndef CHIP8_H_INCLUDED
#define CHIP8_H_INCLUDED

#include <SDL/SDL.h>

class chip8
{
public:
    //---------------VARIABLES-------------------------------------
    unsigned short opcode; //current opcode
    unsigned char memory[4096]; //program memory
    unsigned char V[16]; //general purpose registers
    unsigned short I; //Index register
    unsigned short pc; //program counter
    //0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    //0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    //0x200-0xFFF - Program ROM and work RAM
    unsigned char gfx[64*32]; //graphics
    unsigned char delay_timer;
    unsigned char beep_timer;
    unsigned short stack[16];
    unsigned short sp; //stack pointer
    unsigned char key[16]; //HEX based keypad
    Uint8 *keyState;
    SDL_Event keyEvent;
    bool drawFlag; // drawflag is bool??
    int needKey;

    //-------------------------------------------------------------
    chip8();
    void initialise();
    void loadGame(char *title);
    void emulateCycle();
    void setKeys();
    friend class test_val;

private:

};

//-----TEST FUNCTIONS-----------------//
void T_opcode(chip8 MyChip8);
void T_gfx(chip8 MyChip8);
void T_I(chip8 MyChip8);
void T_pc(chip8 MyChip8);
void T_V(chip8 MyChip8);
void T_V(chip8 MyChip8, int i);
void T_delay_timer(chip8 MyChip8);
void T_beep_timer(chip8 MyChip8);
void T_stack(chip8 MyChip8);
void T_sp(chip8 MyChip8);
void T_memory(chip8 MyChip8);


#endif // CHIP8_H_INCLUDED
