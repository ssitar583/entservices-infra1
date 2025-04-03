#pragma once
#include <string>
#include <curl/curl.h>

class HttpClient {
    public:
        enum Status {
            Success,
            HttpError,
            DiskError
        };

        HttpClient();
        ~HttpClient();

        Status downloadFile(const std::string & url, const std::string & fileName, uint32_t rateLimit = 0);

        void pause() { curl_easy_pause(curl, CURLPAUSE_RECV | CURLPAUSE_SEND); }
        void resume() { curl_easy_pause(curl, CURLPAUSE_CONT); }
        void cancel() { bCancel = true; }
        void setRateLimit(uint32_t rateLimit) {
            if (rateLimit > 0) {
                (void) curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)rateLimit);
            }
        }
        int getProgress() { return progress; }

        // SSL
        void setToken() {}
        void setCert() {}
        void setCertPath() {}

        //
        long getStatusCode() {
            return httpCode;
        }

    private:
        static size_t progressCb(void *ptr, double dltotal, double dlnow, double ultotal, double ulnow);
        static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

    private:
        CURL *curl;
        long httpCode = 0;
        bool bCancel = false;
        int progress = 0;
};

class trace_exit {
    public:
    trace_exit(std::string fn, uint32_t ln)
    : fn(fn) {
        std::cout << "--> Entering " << fn << ":" << ln << std::endl;
    }
    ~trace_exit() {
        std::cout << "<-- Exiting " << fn << std::endl;
    }
    private:
    std::string fn;
};

#define TR() // trace_exit tr(__FUNCTION__, __LINE__);

