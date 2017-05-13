#ifndef LOG_OUTPUT_H_20170513
#define LOG_OUTPUT_H_20170513

void LogOutput_Initialize(const char *proc_name);
void LogOutput_SetMask(int mask);
void LogOutput_Cleanup();

#endif //LOG_OUTPUT_H_20170513
