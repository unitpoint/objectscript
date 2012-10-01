#ifndef __PROFILE_OS_H__
#define __PROFILE_OS_H__

struct OpcodeProfile
{
	int opcode;
	int count;
	double time;

	OpcodeProfile()
	{
		opcode = 0;
		count = 0;
		time = 0;
	}
};

extern OpcodeProfile opcodes_usage[];
extern double start_opcode_time;
double getTimeSec();

#define OS_PROFILE_BEGIN_OPCODE(opcode) do{ start_opcode_time = getTimeSec(); }while(false)
#define OS_PROFILE_END_OPCODE(opcode) do{ \
	opcodes_usage[opcode].count++; \
	opcodes_usage[opcode].time += getTimeSec() - start_opcode_time; \
}while(false)

#endif //__PROFILE_OS_H__
