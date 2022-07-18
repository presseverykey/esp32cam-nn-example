#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "network.h"
#include "config.h"

#include "esp_log.h"
#define TAG "NETWORK"

static const char *ssid = WIFI_SSID;
static const char *pass = WIFI_PASS;
static const uint8_t channel = 1;

static httpd_handle_t server = NULL;

static esp_err_t getIndexHandler(httpd_req_t *req) {
    httpd_resp_sendstr(req, "<html><head><title>Index</title></head><body><h1>Hello!</h1></body></html>");
    return ESP_OK;
}


static const httpd_uri_t getIndex = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = getIndexHandler,
    .user_ctx  = NULL
};

void network_init(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config;
    wifi_config.ap.channel = channel;
    wifi_config.ap.max_connection = 5;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    memcpy(wifi_config.ap.ssid, ssid, strlen(ssid));
    wifi_config.ap.ssid_len = strlen(ssid);
    strcpy((char*)(wifi_config.ap.password), pass);
    if (strlen(pass) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ssid, pass, channel);

    //AP is up, start server

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &getIndex);
    } else {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void httpd_add_get_handler(const char *path, const requestHandler handler) {
  httpd_uri_t uri = {
    .uri       = path,
    .method    = HTTP_GET,
    .handler   = handler,
    .user_ctx  = NULL
  };
  httpd_register_uri_handler(server, &uri);
}

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t httpd_send_framebuffer_as_jpg(camera_fb_t *fb, httpd_req_t *req){
    esp_err_t res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline");
    }
    if (res == ESP_OK) {
        if(fb->format == PIXFORMAT_JPEG){
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
        }
    }
    return res;
}
