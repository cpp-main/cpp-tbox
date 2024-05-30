#include "json_writer.h"

namespace tbox {
namespace trace {

bool JsonWriter::open(const std::string &filename)
{
    ofs_.open(filename);
    return ofs_.is_open();
}

bool JsonWriter::writeHeader()
{
    if (!ofs_.is_open())
        return false;

    ofs_ << R"({ "otherData": {}, "traceEvents": [)" << std::endl;
    ofs_.flush();

    return true;
}

bool JsonWriter::writeRecorder(const char *name, const char *tid, uint64_t start_ts_us, uint64_t duration_us)
{
    if (!ofs_.is_open())
        return false;

    ofs_ << R"({ "cat": "function", "ph": "X", "pid": "", "tid":")" << tid << '"';
    ofs_.flush();

    return true;
}

bool JsonWriter::writeFooter()
{
    if (!ofs_.is_open())
        return false;

    ofs_ << R"(]})" << std::endl;
    ofs_.flush();

    return true;
}

}
}
