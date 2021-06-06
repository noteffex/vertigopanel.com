#pragma once
#include <string>
#define CURL_STATICLIB
#include "lib_curl/curl.h"

#ifdef _DEBUG
#pragma comment(lib, "http/lib_curl/libcurl_a_debug.lib")
#else
#pragma comment(lib, "http/lib_curl/libcurl_a.lib")
#endif

namespace http {
    size_t write_file(void* ptr, size_t size, size_t nmemb, FILE* stream);
    size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
}

namespace http {
    void download_file(std::string url, const char* file);
    std::pair<std::string, int> get_data(std::string url, std::string communication = "");
    std::pair<std::string, int> get_data_post(std::string url, std::string post_data = "");
}