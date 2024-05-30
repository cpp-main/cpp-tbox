#ifndef TBOX_TRACE_PARSER_JSON_WRITER_H_20240530
#define TBOX_TRACE_PARSER_JSON_WRITER_H_20240530

#include <cstdint>
#include <string>
#include <fstream>

namespace tbox {
namespace trace {

class JsonWriter {
 public:
  bool open(const std::string &filename);

  bool writeHeader();
  bool writeRecorder(const char *name, const char *tid, uint64_t start_ts_us, uint64_t duration_us);
  bool writeFooter();

 private:
  std::ofstream ofs_;
};

}
}

#endif //TBOX_TRACE_PARSER_JSON_WRITER_H_20240530
