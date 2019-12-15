#include "spimcore.h"

unsigned ip_helper(unsigned *arr, unsigned count);

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

	// R-type funct
	#define add   32
	#define sub   34
	#define and   36
	#define or    37
	#define slt   42
	#define sltu  43

	switch(ALUControl)
	{
		case 0: case add:
			*ALUresult = A + B;
			break;

		case 1: case sub:
			*ALUresult = A - B;
            break;

		case 2: case slt:

			*ALUresult = (signed) A < (signed) B;
            break;

		case 3: case sltu:

			*ALUresult = (unsigned) A < (unsigned) B;
            break;

		case 4: case and:
			*ALUresult = A & B;
            break;

		case 5: case or:
			*ALUresult = A | B;
            break;

		default:
			break;
	}

	if(*ALUresult == 0)
		*Zero = 1;
	else
		*Zero = 0;

	return;
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	*instruction = Mem[PC>>2];

	if(PC % 4 != 0)
		return 1;

	return 0;
}


/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	// Convert instructions from hex to binary
	unsigned val;
	unsigned binary[32];
	unsigned op_arr[6];
	unsigned r1_arr[5];
	unsigned r2_arr[5];
	unsigned r3_arr[5];
	unsigned funct_arr[6];
	unsigned offset_arr[16];
	unsigned jsec_arr[26];

	for(int i = 0; i < 32; i++)
	{
		val = instruction / 2;
		binary[i] = instruction % 2;
		instruction = val;
	}

	for(int i = 0; i < 6; i++)
		op_arr[i] = binary[31-i];

	for(int i = 0; i < 5; i++)
		r1_arr[i] = binary[25-i];

	for(int i = 0; i < 5; i++)
		r2_arr[i] = binary[20-i];

	for(int i = 0; i < 5; i++)
		r3_arr[i] = binary[15-i];

	for(int i = 0; i < 6; i++)
		funct_arr[i] = binary[5-i];

	for(int i = 0; i < 16; i++)
		offset_arr[i] = binary[15-i];

	for(int i = 0; i < 26; i++)
		jsec_arr[i] = binary[25-i];

	*op = ip_helper(op_arr, 6);
	*r1 = ip_helper(r1_arr, 5);
	*r2 = ip_helper(r2_arr, 5);
	*r3 = ip_helper(r3_arr, 5);
	*funct = ip_helper(funct_arr, 6);
	*offset = ip_helper(offset_arr, 16);
	*jsec = ip_helper(jsec_arr, 26);

	return;
}

unsigned ip_helper(unsigned *arr, unsigned count)
{
	unsigned decimal = 0;
	unsigned temp;
	unsigned power = count - 1;

	for(int i = 0; i < count; i++)
	{
		temp = 1;

		if(arr[i] == 1)
		{
			if(power - i != 1)
			{
				for(int j = 0; j < power - i; j++)
						temp *= 2;
			}
			else
				temp = 2;

			decimal += temp;
		}
	}

	return decimal;
}


/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls)
{
	#define r	  0
	#define j     2
	#define beq	  4
	#define addi  8
	#define slti  10
	#define sltiu 11
	#define lui	  15
	#define lw	  35
	#define sw	  43

	switch(op)
	{
		case r:	// r-type
			controls->RegDst = 1;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 7; // r-type
			controls->MemWrite = 0;
			controls->ALUSrc = 0;
			controls->RegWrite = 1;
			return 0;

		case j: // j-type
			controls->RegDst = 2;
			controls->Jump = 1;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // don't care
			controls->MemWrite = 0;
			controls->ALUSrc = 2;
			controls->RegWrite = 0;
			return 0;

		case beq: // i-type
			controls->RegDst = 2;
			controls->Jump = 0;
			controls->Branch = 1;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 1; // sub
			controls->MemWrite = 0;
			controls->ALUSrc = 0;
			controls->RegWrite = 0;
			return 0;

		case addi: // i-type
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 0; // add
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			return 0;

		case slti: // i-type
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 2; // slt
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			return 0;

		case sltiu: // i-type
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 0;
			controls->ALUOp = 3; // sltu
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			return 0;

		case lui: // i-type
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 2;
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // don't care
			controls->MemWrite = 0;
			controls->ALUSrc = 2;
			controls->RegWrite = 1;
			return 0;

		case lw: // i-type
			controls->RegDst = 0;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 1;
			controls->MemtoReg = 1;
			controls->ALUOp = 0; // add
			controls->MemWrite = 0;
			controls->ALUSrc = 1;
			controls->RegWrite = 1;
			return 0;

		case sw: // i-type
			controls->RegDst = 2;
			controls->Jump = 0;
			controls->Branch = 0;
			controls->MemRead = 0;
			controls->MemtoReg = 2;
			controls->ALUOp = 0; // add
			controls->MemWrite = 1;
			controls->ALUSrc = 1;
			controls->RegWrite = 0;
			return 0;

		default:
			return 1;
	}
}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	*data1 = Reg[r1];

	*data2 = Reg[r2];

	return;
}


/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	unsigned binary[32];
	unsigned tmp1 = offset, tmp2;

	for(int i = 31; i >= 0; i--)
	{
		tmp2 = tmp1 / 2;
		binary[i] = tmp1 % 2;
		tmp1 = tmp2;

		if(i <= 16)
			if(binary[21] == 1)
				binary[i] = 1;
			else
				binary[i] = 0;
	}

	*extended_value = ip_helper(binary, 32);

	return;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	// ALUOp Switch
	#define OPaddordc 0
	#define OPsub 1
	#define OPslt 2
	#define OPsltu 3
	#define OPand 4
	#define OPor 5
	#define OPshiftXvalue 6
	#define OPr 7

	// R-type funct Switch
	#define add   32
	#define sub   34
	#define and   36
	#define or    37
	#define slt   42
	#define sltu  43

	if(ALUSrc == 0)
	{
		if(funct == add || funct == sub || funct == and || funct == or || funct == slt || funct == sltu)
			ALU(data1, data2, funct, ALUresult, Zero);
		else
			ALU(data1, data2, ALUOp, ALUresult, Zero);

		return 0;
	}

	if(ALUSrc == 1)
	{
		ALU(data1, extended_value, ALUOp, ALUresult, Zero);
		return 0;
	}

	if(ALUSrc == 2)
	{
		*ALUresult = extended_value;

		if(*ALUresult == 0)
			*Zero = 1;
		else
			*Zero = 0;

		return 0;
	}

	return 1;
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	if(MemRead == 1)
	{
		if(ALUresult % 4 != 0)
			return 1;

		*memdata = Mem[ALUresult >> 2];
	}

	if(MemWrite == 1)
	{
		if(ALUresult % 4 != 0)
			return 1;

		Mem[ALUresult >> 2] = data2;
	}

	return 0;
}


/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	if(RegWrite == 1 && MemtoReg == 1)
	{
		if(RegDst == 1)
			Reg[r3] = memdata;
		else
			Reg[r2] = memdata;
	}

	if(RegWrite == 1 && MemtoReg == 0)
	{
		if(RegDst == 1)
			Reg[r3] = ALUresult;
		else
			Reg[r2] = ALUresult;
	}

	if(RegWrite == 1 && MemtoReg == 2)
		Reg[r2] = ALUresult << 16;

	return;
}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
	*PC = *PC+4;

	if(Jump)
	{
		*PC = jsec << 2;
		return;
	}

	if(Branch && Zero)
	{
		*PC = *PC + (extended_value << 2);
		return;
	}

	return;
}
