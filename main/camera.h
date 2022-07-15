#pragma once

#include "esp_camera.h"
#include "esp_http_server.h"

esp_err_t camera_init();

esp_err_t jpg_httpd_handler(httpd_req_t *req);

