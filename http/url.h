#ifndef TBOX_HTTP_URL_H_20220512
#define TBOX_HTTP_URL_H_20220512

#include <string>
#include <map>
#include <cstdint>

namespace tbox {
namespace http {

/**
 * URL格式：<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
 */
struct Url {
    std::string scheme;

    struct Host {
        std::string user;
        std::string password;
        std::string host;
        uint16_t    port = 0;
    };

    struct Path {
        using StringMap = std::map<std::string, std::string>;
        std::string path;
        StringMap   params;
        StringMap   query;
        std::string frag;
    };

    Host host;
    Path path;
};

std::string LocalToUrl(const std::string &local_str);
std::string UrlToLocal(const std::string &url_str);

std::string UrlToString(const Url &url);
std::string UrlHostToString(const Url::Host &host);
std::string UrlPathToString(const Url::Path &path);

bool StringToUrl(const std::string &str, Url &url);
bool StringToUrlHost(const std::string &str, Url::Host &host);
bool StringToUrlPath(const std::string &str, Url::Path &path);

}
}

#endif //TBOX_HTTP_URL_H_20220512
