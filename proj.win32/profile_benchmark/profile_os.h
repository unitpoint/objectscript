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

extern int gc_step_count;
extern double gc_step_start_time;
extern double gc_step_time;
extern double gc_sum_time;

#define OS_PROFILE_BEGIN_GC do{ gc_step_start_time = getTimeSec(); gc_step_time = 0; }while(false)
#define OS_PROFILE_END_GC do{ gc_step_count++; gc_step_time = getTimeSec() - gc_step_start_time; gc_sum_time += gc_step_time; }while(false)

#define OS_PROFILE_BEGIN_OPCODE(opcode) do{ gc_step_time = 0; start_opcode_time = getTimeSec(); }while(false)
#define OS_PROFILE_END_OPCODE(opcode) do{ \
	opcodes_usage[opcode].count++; \
	opcodes_usage[opcode].time += getTimeSec() - start_opcode_time - gc_step_time; \
}while(false)

#endif //__PROFILE_OS_H__
