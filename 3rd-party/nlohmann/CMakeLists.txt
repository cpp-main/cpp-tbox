cmake_minimum_required(VERSION 3.15)

project(nlohmann)

set(TBOX_JSON_HEADERS
    json.hpp
    json_fwd.hpp)

install(FILES ${TBOX_JSON_HEADERS} DESTINATION include/nlohmann)
