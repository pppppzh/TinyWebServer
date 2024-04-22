#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       
#include <unistd.h>      
#include <sys/stat.h>    
#include <sys/mman.h>   

#include "buffer.h"
#include "log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(buffer &buff);
    void AddHeader_(buffer &buff);
    void AddContent_(buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;
    
    char* mmFile_; 
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE; 
    static const std::unordered_map<int, std::string> CODE_STATUS;        
    static const std::unordered_map<int, std::string> CODE_PATH;            
};


#endif //HTTP_RESPONSE_H

