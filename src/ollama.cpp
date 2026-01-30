#include <iostream>
#include <curl/curl.h>
#include <string>
#include "ollama.hpp"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(reinterpret_cast<char*>(contents), size * nmemb);

    return size * nmemb;
}

std::string parseResponse(const std::string& response) {
    const std::string key = "\"response\":\"";
    size_t start = response.find(key);
    if (start == std::string::npos) return "";

    start += key.length();
    std::string result;
    bool escape = false;

    for (size_t i = start; i < response.size(); ++i) {
        char c = response[i];

        if (escape) {
            result += c;
            escape = false;
        } else if (c == '\\') {
            escape = true;
            result += c;
        } else if (c == '"') {
            break;
        } else {
            result += c;
        }
    }

    return result;
}

std::string escapeJson(const std::string &s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string unescapeJsonString(const std::string& s) {
    std::string result;
    result.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char next = s[i + 1];
            switch (next) {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                default: result += next; break;
            }
            ++i;
        } else {
            result += s[i];
        }
    }

    return result;
}

std::string ollama::generate(const std::string& prompt, const std::string& model) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if(curl) {
		std::string escapedPrompt = escapeJson(prompt);
		std::string jsonPayload = R"({"model":")" + model + R"(","prompt":")" + escapedPrompt + R"(","stream":false})";

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

	std::string llm_response = parseResponse(readBuffer);
	return unescapeJsonString(llm_response);
}
