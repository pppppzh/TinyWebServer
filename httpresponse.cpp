#include "httpresponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE{
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "}};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS{
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH{
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse()
    : code_(-1), isKeepAlive_(false), mmFile_(nullptr) {}

HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void HttpResponse::Init(const std::string &srcDir, std::string &path, bool isKeepAlive, int code)
{
    assert(srcDir != "");
    if (mmFile_)
        UnmapFile();
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr;
    mmFileStat_ = {0};
}

void HttpResponse::MakeResponse(buffer &buff)
{
    if (stat((srcDir_ + path_).c_str(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode))
    {
        code_ = 404;
    }
    else if (!mmFileStat_.st_mode & S_IROTH)
    {
        code_ = 403;
    }
    else if (code_ == -1)
        code_ = 200;
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char *HttpResponse::File()
{
    return mmFile_;
}

size_t HttpResponse::FileLen() const
{
    return mmFileStat_.st_size;
}

void HttpResponse::ErrorHtml_()
{
    if (CODE_PATH.count(code_))
    {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).c_str(), &mmFileStat_);
    }
}

void HttpResponse::AddStateLine_(buffer &buff)
{
    std::string sta;
    if (CODE_STATUS.count(code_))
    {
        sta = CODE_STATUS.find(code_)->second;
    }
    else
    {
        code_ = 400;
        sta = CODE_STATUS.find(code_)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + sta + "\r\n");
}

void HttpResponse::AddHeader_(buffer &buff)
{
    buff.Append("Connection:");
    if (isKeepAlive_)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6,timeout=120\r\n");
    }
    else
    {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResponse::AddContent_(buffer &buff)
{
    int srcFd = open((srcDir_ + path_).c_str(), O_RDONLY);
    if (srcFd < 0)
    {
        ErrorContent(buff, "File not found");
        return;
    }
    LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    auto mmRet = mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (mmRet == (void *)-1)
    {
        ErrorContent(buff, "File not found");
        return;
    }
    mmFile_ = (char *)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

std::string HttpResponse::GetFileType_()
{
    size_t idx = path_.find_last_of('.');
    if (idx == std::string::npos)
        return "text/plain";
    std::string suffix = path_.substr(idx);
    if (SUFFIX_TYPE.count(suffix))
        return SUFFIX_TYPE.find(suffix)->second;
    return "text/plain";
}

void HttpResponse::ErrorContent(buffer &buff, std::string message)
{
    std::string body, status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(code_))
        status = CODE_STATUS.find(code_)->second;
    else
        status = "Bad Request";
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}