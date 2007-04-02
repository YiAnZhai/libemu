/* @header@ */



#include <stdlib.h>


#include "emu/emu.h"
#include "emu/emu_memory.h"
#include "emu/emu_cpu.h"
#include "emu/emu_cpu_data.h"
#include "emu/emu_getpc.h"
#include "emu/emu_cpu_instruction.h"


#define        MIN(a,b) (((a)<(b))?(a):(b))
#define        MAX(a,b) (((a)>(b))?(a):(b))

uint8_t emu_getpc_check(struct emu *e, uint8_t *data, uint32_t size, uint32_t offset)
{
	struct emu_cpu *c = emu_cpu_get(e);
	struct emu_memory *m = emu_memory_get(e);


//	uint32_t offset;
//	for (offset=0; offset<size;offset++)
//	{

	int reg;
	for (reg=0; reg<8; reg++)
	{
		emu_cpu_reg32_set(c, reg, 0);
	}

	emu_cpu_reg32_set(c, esp, 0x12000);


	switch (data[offset])
	{
	/* call */
	case 0xe8:
		emu_memory_write_block(m, 0x1000, data+offset, MIN(size-offset, 6));
		emu_cpu_eip_set(c, 0x1000);


		if ( emu_cpu_parse(c) != 0)
			break;



		printf("call within %i bytes \n", c->instr.cpu.disp);
		if (abs(c->instr.cpu.disp) > 512)
		{
			break;
		}
		else
		{

			printf("size is within\n");
		}

		if (c->instr.cpu.disp < 0)
		{
			if ( offset + c->instr.cpu.disp < 0  )
			{
				break;
			}
			else
			{
				printf("eip is within\n");
			}

		}

		if (c->instr.cpu.disp > 0)
		{
			if ( offset + c->instr.cpu.disp > size  )
			{
				break;
			}
			else
			{
				printf("eip is _still_ within size: %i i: %i disp: %i (%i)\n", size, offset, c->instr.cpu.disp, size - offset + c->instr.cpu.disp);
			}
		}

		printf("writing from offset %i to offset %i\n", offset + MIN(c->instr.cpu.disp, 0), 0x1000+MIN(c->instr.cpu.disp, 0));
		emu_memory_write_block(m, 0x1000 + MIN(c->instr.cpu.disp, 0), data + offset + MIN(c->instr.cpu.disp, 0), size - offset - MIN(c->instr.cpu.disp, 0));
		emu_cpu_eip_set(c, 0x1000);

		uint32_t espcopy = emu_cpu_reg32_get(c, esp);
		int j;
		for (j=0;j<64;j++)
		{
			int ret = emu_cpu_parse(emu_cpu_get(e));

			if (ret != -1)
			{
				ret = emu_cpu_step(emu_cpu_get(e));
			}

			if ( ret == -1 )
			{
				printf("cpu error %s\n", emu_strerror(e));
				break;
			}

			if (emu_cpu_reg32_get(c, esp) == espcopy) // eip pushed by call is popped
				return 1;
		}

		return 1;
		break;

		/* fnstenv */
	case 0xd9:
		emu_memory_write_block(m, 0x1000, data+offset, MIN(size-offset, 64));
		emu_cpu_eip_set(c, 0x1000);

		if ( emu_cpu_parse(c) != 0 )
			break;

		if( (c->instr.fpu.fpu_data[1] & 0x38) != 0x30 )
			break;

		printf("found valid fnstenv\n");

		espcopy = emu_cpu_reg32_get(c, esp);
		for (j=0;j<64;j++)
		{
			int ret = emu_cpu_parse(emu_cpu_get(e));

			if (ret != -1)
			{
				ret = emu_cpu_step(emu_cpu_get(e));
			}

			if ( ret == -1 )
			{
				printf("cpu error %s\n", emu_strerror(e));
				break;
			}

			if (emu_cpu_reg32_get(c, esp) == espcopy + 4) // eip written by fnstenv
				return 1;
		}


		break;

	}
	return 0;
}
