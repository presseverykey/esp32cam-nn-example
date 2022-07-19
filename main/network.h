#pragma once

#include <esp_http_server.h>
#include "esp_camera.h"

void network_init();

typedef esp_err_t (*requestHandler)(httpd_req_t *r);

void httpd_add_get_handler(const char *path, requestHandler handler);

esp_err_t httpd_send_framebuffer_as_jpg(camera_fb_t *fb, httpd_req_t *req);
