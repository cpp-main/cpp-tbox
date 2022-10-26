#ifndef LOG_OUTPUT_H
#define LOG_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

//! Initialize log output module
//! \param  proc_name   Current program name
//! \note   This function should be invoke at program begin
void LogOutput_Initialize(const char *proc_name);

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
