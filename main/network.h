#pragma once

#include <esp_http_server.h>

void network_init();

typedef esp_err_t (*requestHandler)(httpd_req_t *r);

void httpd_addGetHandler(const char *path, requestHandler handler);