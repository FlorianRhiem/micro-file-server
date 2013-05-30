#ifndef HTTP_H
#define HTTP_H

/* Status 400: Bad_Request */
#define HTTP_CODE_400_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 233\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Bad Request - µfs</title>\n</head>\n<body>\n<h1>Bad Request</h1>\n<p>Sorry, but your browser seems to speek in riddles and µfs is unable to understand it.</p>\n</body>\n</html>\n";

/* Status 403: Forbidden */
#define HTTP_CODE_403_FORBIDDEN "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: 195\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Forbidden - µfs</title>\n</head>\n<body>\n<h1>Forbidden</h1>\n<p>Sorry, but µfs does not allow you to see this file.</p>\n</body>\n</html>\n";

/* Status 404: Not_Found */
#define HTTP_CODE_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 197\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>File Not Found - µfs</title>\n</head>\n<body>\n<h1>File Not Found</h1>\n<p>Sorry, but µfs is unable to find this file.</p>\n</body>\n</html>\n";

/* Status 405: Method_Not_Allowed */
#define HTTP_CODE_405_METHOD_NOT_ALLOWED "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: 206\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Method Not Allowed - µfs</title>\n</head>\n<body>\n<h1>Method Not Allowed</h1>\n<p>Sorry, but µfs only allows you to GET files.</p>\n</body>\n</html>\n";

/* Status 408: Request_Timeout */
#define HTTP_CODE_408_REQUEST_TIMEOUT "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/html\r\nContent-Length: 231\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Request Timeout - µfs</title>\n</head>\n<body>\n<h1>Request Timeout</h1>\n<p>Sorry, but your browser took too long to complete sending a request to µfs.</p>\n</body>\n</html>\n";

/* Status 431: Request_Header_Fields_Too_Large */
#define HTTP_CODE_431_REQUEST_HEADER_FIELDS_TOO_LARGE "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Type: text/html\r\nContent-Length: 277\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Request Header Fields Too Large - µfs</title>\n</head>\n<body>\n<h1>Request Header Fields Too Large</h1>\n<p>Sorry, but your browser sent a request to µfs that was simply too large for it to accept.</p>\n</body>\n</html>\n";

/* Status 503: Service_Unavailable */
#define HTTP_CODE_503_SERVICE_UNAVAILABLE "HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/html\r\nContent-Length: 204\r\nConnection: close\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>Service Unavailable - µfs</title>\n</head>\n<body>\n<h1>Service Unavailable</h1>\n<p>Sorry, but µfs is really busy right now.</p>\n</body>\n</html>\n";
#endif
