#include <iostream>
#include <curl/curl.h>
#include <string>
#include "ollama.hpp"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(reinterpret_cast<char*>(contents), size * nmemb);

    return size * nmemb;
}

std::string parseResponse(const std::string& response) {
    size_t start = response.find("\"response\":\"");
    if (start == std::string::npos) return "";
    start += 12;
    size_t end = response.find("\"", start);
    if (end == std::string::npos) return "";
    return response.substr(start, end - start);
}

std::string ollama::generate(const std::string& prompt, const std::string& model) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if(curl) {
        std::string jsonPayload = R"({"model":")" + model + R"(","prompt":")" + prompt + R"(","stream":false})";

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

	return parseResponse(readBuffer);
}
