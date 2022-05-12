#ifndef TBOX_HTTP_URL_H_20220512
#define TBOX_HTTP_URL_H_20220512

#include <string>
#include <map>
#include <cstdint>

namespace tbox {
namespace http {

struct Url {
    using StringMap = std::map<std::string, std::string>;

    std::string scheme;
    std::string user;
    std::string password;
    std::string host;
    uint16_t    port;
    std::string path;
    StringMap   params;
    StringMap   query;
    std::string frag;
};

bool UrlToString(const Url &url, std::string &str);
bool StringToUrl(const std::string &str, Url &url);

}
}

#endif //TBOX_HTTP_URL_H_20220512
