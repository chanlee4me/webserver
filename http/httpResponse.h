#include "httpRequest.h"
#include <string>
#include <map>

using namespace std;

class HttpResponse{
public:
    enum HttpStatusCode {
        OK = 200,
        BadRequest = 400,
        Forbidden = 403,
        NotFound = 404,
    };
    HttpResponse();
    ~HttpResponse();
    void setStatusCode(int statusCode);
    void setHeader(string key, string value);
    void setBody(string body);
    string getResponse();
 private:
  HttpRequest::Version version;
  HttpStatusCode statusCode;
  string statusMessage_;
  map<string, string> headers;
  string body_;
}