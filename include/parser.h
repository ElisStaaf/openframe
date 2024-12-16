#ifndef _PARSER_H
#define _PARSER_H

#include <hash.h>
#include <http.h>

void parser_parse_request(const char *req, http_request *request);



#endif