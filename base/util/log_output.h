#ifndef LOG_OUTPUT_H
#define LOG_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

//! Initialize log output module
//! \param  proc_name   Current program name
//! \note   This function should be invoke at program begin
void LogOutput_Initialize(const char *proc_name);

//! Defines log output mask bits
#define LOG_OUTPUT_MASK_STDOUT  1   //!< to stdout
#define LOG_OUTPUT_MASK_SYSLOG  2   //!< to syslog

//! Set log's output channel
//! \mask   LOG_OUTPUT_MASK_SYSLOG or LOG_OUTPUT_MASK_STDOUT
void LogOutput_SetMask(int mask);

void LogOutput_Cleanup();

//! You should implement this function, otherwise print all
//! \param  module_id   Module ID
//! \param  func_name   Function name
//! \param  file_name   File name
//! \param  level       Log Level
//
//! \return true    Print this log
//! \return false   Ingore this log
//
//bool LogOutput_FilterFunc(const char *module_id, const char *func_name,
//                          const char *file_name, int level);

#ifdef __cplusplus
}
#endif

#endif //LOG_OUTPUT_H
