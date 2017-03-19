#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL/SDL.h>
#include "chip8.h"

#define OC_ECHO false

using namespace std;

long getFileSize(FILE *file)
{
    long lCurPos, lEndPos;
    lCurPos = ftell(file);
    fseek(file, 0, 2);
    lEndPos = ftell(file);
    fseek(file, lCurPos, 0);
    return lEndPos;
}

chip8::chip8()
{

}

void chip8::initialise()  //OK (watch for mem)
{
    pc = 0x200;  // Program counter starts at 0x200
    opcode = 0;  // Reset current opcode
    I = 0;       // Reset index register
    sp = 0;      // Reset stack pointer

    for(int i = 0; i < (64*32); i++) // Clear display
        gfx[i] = 0;
    for(int i = 0; i < 16; i++) // Clear stack
        stack[i] = 0;
    for(int i = 0; i < 0x10; i++) // Clear registers V0-VF
        V[i] = 0;
    for(int i = 0; i < 4096; i++) // Clear memory
        memory[i] = 0;

     unsigned char chip8_fontset[80] =    //Load fontset
        {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
    for(int i = 0; i < 80; ++i)
        memory[i+128] = chip8_fontset[i];
    //T_memory(*this);
    delay_timer = 0;//Reset timers
    beep_timer = 0;


}

void chip8::loadGame(char *title)
{

    unsigned char *fileBuf;          // Pointer to our buffered data
    FILE *file = NULL;      // File pointer

    // Open the file in binary mode using the "rb" format string
    // This also checks if the file exists and/or can be opened for reading correctly
    if ((file = fopen(title, "rb")) == NULL)
    {
        fprintf(stderr,"Could not open specified file\n");
        exit (1);
    }
    else
        fprintf(stderr,"File opened successfully\n");

    // Get the size of the file in bytes
    int fileSize = getFileSize(file);
        fprintf(stderr,"file is %d bytes long", fileSize);

    // Allocate space in the buffer for the whole file
    fileBuf = new unsigned char[fileSize];

    // Read the file in to the buffer
    fread(fileBuf, fileSize, 1, file);

    // Now that we have the entire file buffered, we can take a look at some binary infomation
    // Lets take a look in hexadecimal
    //for (int i = 0; i < fileSize; i++)
    //    printf("%X ", fileBuf[i]);
    for(int i = 0; i < fileSize; ++i)
        memory[i + 512] = fileBuf[i];
    //T_memory(*this);

    delete[]fileBuf;
        fclose(file);
    //import with fopen in binary mode, input into buffer

}

void chip8::emulateCycle()
{
    opcode = memory[pc] << 8 | memory[pc + 1]; //Fetch opcode
    //T_opcode(*this);
    //Decode opcode
    switch(opcode & 0xF000)
    {
        //Some opcodes//
    case 0x0000:
        switch (opcode & 0x000F)
        {
        case 0x0000: //0x00E0: Clears the screen
            //cout << "0x00E0: Clears the screen" << endl;
            for (int i = 0; i < (64*32); i++)
            {
                gfx[i] = 0;
            }
            drawFlag = true;
            pc += 2;
            break;

        case 0x000E: //0x00EE: returns from subroutine
            //cout << "0x00EE: returns from subroutine" << endl;
            --sp;
            pc = stack[sp];
            pc += 2;
            //T_stack(*this);
            //T_sp(*this);
            //T_pc(*this);
            break;

        default:
            fprintf(stderr,"Unknown opcode: 0x%X\n", opcode);
        }
        break;

    case 0x1000: //0x1NNN: Jumps to address NNN.
        //cout << "0x1NNN: Jumps to address NNN" << endl;
        pc = opcode & 0x0FFF;
        break;

    case 0x2000: //0x2NNN: Calls subroutine at NNN
        //if(OC_ECHO) cout << "0x2NNN: Calls subroutine at NNN" << endl;
        stack[sp] = pc;
        ++sp;
        pc = opcode & 0x0FFF;
        break;

    case 0x3000: //0x3XNN: Skips the next instruction if VX equals NN
        //cout << "0x3XNN: Skips the next instruction if VX equals NN" << endl;
        if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x0FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x4000: //0x4XNN: Skips the next instruction if VX doesn't equal NN
        //cout << "0x4XNN: Skips the next instruction if VX doesn't equal NN" << endl;
        if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x0FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x5000: //0x5XY0: Skips the next instruction if VX equals VY
        //cout << "0x5XY0: Skips the next instruction if VX equals VY" << endl;
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0x6000: //0x6XNN: Sets VX to NN
        //if(OC_ECHO) cout << "0x6XNN: Sets VX to NN" << endl;
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x0FF);
        pc += 2;
        break;

    case 0x7000: //0x7XNN: Adds NN to VX
        //cout << "0x7XNN: Adds NN to VX" << endl;
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        pc += 2;
        break;

    case 0x8000:
        switch(opcode & 0x000F)
        {
        case 0x0000: //0x8XY0: Sets VX to the value of VY
            //cout << "0x8XY0: Sets VX to the value of VY" << endl;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: //0x8XY1: Sets VX to VX or VY
            //cout << "0x8XY1: Sets VX to VX or VY" << endl;
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002: //0x8XY2: Sets VX to VX and VY
            //cout << "0x8XY2: Sets VX to VX and VY" << endl;
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0003: //0x8XY3: Sets VX to VX xor VY
            //cout << "0x8XY3: Sets VX to VX xor VY" << endl;
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0004: //0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
            //cout << "0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't" << endl;
            if (V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc +=2;
            break;

        case 0x0005: //0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            //cout << "0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't" << endl;
            if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0006: //0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
            //cout << "0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift" << endl;
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
            pc += 2;
            break;

        case 0x0007: //0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            //cout << "0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't" << endl;
            if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                V[0xF] = 0;
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E: //0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
            //cout << "0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift" << endl;
            V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
            pc += 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
        }
        break;

    case 0x9000: //0x9XY0: Skips the next instruction if VX doesn't equal VY
        //cout << "0x9XY0: Skips the next instruction if VX doesn't equal VY" << endl;
        if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0xA000: //OK//0xANNN: Sets I to the address NNN
        //if(OC_ECHO) cout << "0xANNN: Sets I to the address NNN" << endl;
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: //0xBNNN: Jumps to the address NNN plus V0
        //cout << "0xBNNN: Jumps to the address NNN plus V0" << endl;
        pc = (opcode & 0x0FFF) + V[0];
        break;

    case 0xC000: //0xCXNN: Sets VX to a random number and NN
        //cout << "0xCXNN: Sets VX to a random number and NN" << endl;
        srand(time(NULL));
        V[(opcode & 0x0F00) >> 8] = rand();
        V[(opcode & 0x0F00) >> 8] &= (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: //0xDXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded (with the most significant bit of each byte displayed on the left) starting from memory location I; I value doesn't change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen
        {
        //if (OC_ECHO) cout << "0xDXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels" << endl;
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = (opcode & 0x000F);
        unsigned short pixel;

        V[0xF] = 0;//reset flag VF
        for(int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for(int xline = 0; xline < 8; xline++)
            {
                if((pixel & (0x80 >> xline)) != 0)//if need to write pixel
                {
                    if(gfx[x+xline + (y+yline)*64] == 1)//then if pixel already in gfx
                        V[0xF] = 1;   //then flag collision
                    gfx[x+xline + (y+yline)*64] ^= 1;
                        //write pixel to gfx
                }
            }
        }
        drawFlag = true;
        pc += 2;
        }
        break;

    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E: //0xEX9E: Skips the next instruction if the key stored in VX is pressed
            //cout << "0xEX9E: Skips the next instruction if the key stored in VX is pressed" << endl;
            if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;

        case 0x00A1: //0xEXA1: Skips the next instruction if the key stored in VX isn't pressed
            //cout << "0xEXA1: Skips the next instruction if the key stored in VX isn't pressed" << endl;
            if(key[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
        }
        break;

    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: //0xFX07: Sets VX to the value of the delay timer
            //cout << "0xFX07: Sets VX to the value of the delay timer" << endl;
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;

        case 0x000A: //0xFX0A: A key press is awaited, and then stored in VX
            needKey = 1;
            int resultat;
            while (needKey)
            {
                SDL_WaitEvent(&keyEvent);
                switch(keyEvent.type)
                {
                    case SDL_KEYDOWN:
                        switch(keyEvent.key.keysym.sym)
                        {
                        case SDLK_1:
                            resultat = 0x1;
                            needKey = 0;
                        break;
                        case SDLK_2:
                            resultat = 0x2;
                            needKey = 0;
                        break;
                        case SDLK_3:
                            resultat = 0x3;
                            needKey = 0;
                        break;
                        case SDLK_4:
                            resultat = 0xC;
                            needKey = 0;
                        break;
                        case SDLK_q:
                            resultat = 0x4;
                            needKey = 0;
                        break;
                        case SDLK_w:
                            resultat = 0x5;
                            needKey = 0;
                        break;
                        case SDLK_e:
                            resultat = 0x6;
                            needKey = 0;
                        break;
                        case SDLK_r:
                            resultat = 0xD;
                            needKey = 0;
                        break;
                        case SDLK_a:
                            resultat = 0x7;
                            needKey = 0;
                        break;
                        case SDLK_s:
                            resultat = 0x8;
                            needKey = 0;
                        break;
                        case SDLK_d:
                            resultat = 0x9;
                            needKey = 0;
                        break;
                        case SDLK_f:
                            resultat = 0xE;
                            needKey = 0;
                        break;
                        case SDLK_z:
                            resultat = 0xA;
                            needKey = 0;
                        break;
                        case SDLK_x:
                            resultat = 0x0;
                            needKey = 0;
                        break;
                        case SDLK_c:
                            resultat = 0xB;
                            needKey = 0;
                        break;
                        case SDLK_v:
                            resultat = 0xF;
                            needKey = 0;
                        break;
                        }
                }
                V[(opcode & 0x0F00) >> 8] = resultat;
            }
            pc += 2;
            break;


        case 0x0015: //0xFX15: Sets the delay timer to VX
            //cout << "0xFX15: Sets the delay timer to VX" << endl;
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0018: //0xFX18: Sets the sound timer to VX
            //cout << "0xFX18: Sets the sound timer to VX" << endl;
            beep_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x001E: //0xFX1E: Adds VX to I(*)
            //cout << "0xFX1E: Adds VX to I(*)" << endl;
            if(I > (0xFFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1;
            else
                V[0xF] = 0;
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0029: //0xFX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
            //cout << "0xFX29: Sets I to the location of the sprite for the character in VX" << endl;
            I = V[(opcode & 0x0F00) >> 8]*5+0x80;
            //T_V(*this);
            //T_I(*this);
            pc += 2;
            break;

        case 0x0033: //0xFX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
            //if(OC_ECHO) cout << "0xFX33: Stores the Binary-coded decimal representation of VX" << endl;
            memory[I] = V[(opcode & 0x0F00) >> 8] /100;
            memory[I+1] = (V[(opcode & 0x0F00) >> 8]/10)%10;
            memory[I+2] = V[(opcode & 0x0F00) >> 8] % 10; //différent du tuto (juste ou pas??)
            pc += 2;
            break;

        case 0x0055: //0xFX55: Stores V0 to VX in memory starting at address I ( On the original interpreter, when the operation is done, I=I+X+1)
            //cout << "0xFX55: Stores V0 to VX in memory starting at address I" << endl;
            for(int i = 0; i < (((opcode & 0x0F00) >> 8) + 1); i++)
            {
                memory[I + i] = V[i];
            }
            pc += 2;
            break;

        case 0x0065: //0xFX65: Fills V0 to VX with values from memory starting at address I( On the original interpreter, when the operation is done, I=I+X+1)
            //if (OC_ECHO) cout << "0xFX65: Fills V0 to VX with values from memory starting at address I" << endl;
            for(int i = 0; i < (((opcode & 0x0F00) >> 8) + 1); i++)
            {
                V[i] = memory[I + i];
            }
            pc += 2;
            break;

        default:
            fprintf(stderr,"Unknown opcode: 0x%X\n", opcode);
        }
        break;

    default:
        fprintf(stderr,"Unknown opcode: 0x%X\n", opcode);

    }

    //Update timers
    if(delay_timer > 0)
        --delay_timer;

    if(beep_timer > 0)
    {
        //printf("bouap: %X", beep_timer);
        if(beep_timer == 1)
            //printf("BEEP!\n");
        --beep_timer;
    }

}

void chip8::setKeys()
{
    keyState = SDL_GetKeyState(NULL);
    keyState[SDLK_1] ? key[0x1] = 1 : key[0x1] = 0;
    keyState[SDLK_2] ? key[0x2] = 1 : key[0x2] = 0;
    keyState[SDLK_3] ? key[0x3] = 1 : key[0x3] = 0;
    keyState[SDLK_4] ? key[0xC] = 1 : key[0xC] = 0;
    keyState[SDLK_q] ? key[0x4] = 1 : key[0x4] = 0;
    keyState[SDLK_w] ? key[0x5] = 1 : key[0x5] = 0;
    keyState[SDLK_e] ? key[0x6] = 1 : key[0x6] = 0;
    keyState[SDLK_r] ? key[0xD] = 1 : key[0xD] = 0;
    keyState[SDLK_a] ? key[0x7] = 1 : key[0x7] = 0;
    keyState[SDLK_s] ? key[0x8] = 1 : key[0x8] = 0;
    keyState[SDLK_d] ? key[0x9] = 1 : key[0x9] = 0;
    keyState[SDLK_f] ? key[0xE] = 1 : key[0xE] = 0;
    keyState[SDLK_z] ? key[0xA] = 1 : key[0xA] = 0;
    keyState[SDLK_x] ? key[0x0] = 1 : key[0x0] = 0;
    keyState[SDLK_c] ? key[0xB] = 1 : key[0xB] = 0;
    keyState[SDLK_v] ? key[0xF] = 1 : key[0xF] = 0;

}

//-----TEST FUNCTIONS-----------------//
void T_opcode(chip8 MyChip8)
{
    printf("Current opcode: 0x%X", MyChip8.opcode);
    cin.get();
}

void T_gfx(chip8 MyChip8)
{
    for(int y = 0; y < 32; y++)
    {
        for(int x = 0; x < 64; x++)
        {
            //printf("%d",MyChip8.gfx[x + y*64]);
            if((bool)MyChip8.gfx[x + y*64])
                cout << (char)219;
            else
                cout << (char)176;
        }
        cout << endl;
    }
    cin.get();
}

void T_I(chip8 MyChip8)
{
    printf("Index address: 0x%X", MyChip8.I);
    cin.get();
}

void T_pc(chip8 MyChip8)
{
    printf("Program counter: 0x%X", MyChip8.pc);
    cin.get();
}

void T_V(chip8 MyChip8)
{
    for(int i = 0; i < 0x10; i++)
    {
        printf("V%X = ", i);
        printf("0x%X ; ", MyChip8.V[i]);
    }
    printf("\n");
    cin.get();
}

void T_V(chip8 MyChip8, int i)
{
    printf("V%X = ", i);
    printf("Ox%X\n", MyChip8.V[i]);
    cin.get();
}

void T_delay_timer(chip8 MyChip8)
{
    printf("Delay timer: %d", MyChip8.delay_timer);
    cin.get();
}

void T_beep_timer(chip8 MyChip8)
{
    printf("Beep timer: %d", MyChip8.beep_timer);
    cin.get();
}

void T_stack(chip8 MyChip8)
{
    for(int i = 0; i < 0x10; i++)
    {
        printf("0x%X ; ", MyChip8.stack[i]);
    }
    printf("\n");
    cin.get();
}

void T_sp(chip8 MyChip8)
{
    printf("sp = %d", MyChip8.sp);
    cin.get();
}

void T_memory(chip8 MyChip8)
{
    printf("Interpreter:\n");
    for(int i = 0; i < 0x200; i++)
        printf("0x%X ", MyChip8.memory[i]);
    printf("\nProgram memory:\n");
    for(int i = 0x200; i < 0xFFF; i++)
        printf("0x%X ", MyChip8.memory[i]);
    printf("\n");

}

/*
    unsigned char memory[4096]; //program memory
    //0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    //0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    //0x200-0xFFF - Program ROM and work RAM
    unsigned char key[16]; //HEX based keypad
    */
