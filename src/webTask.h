/*
WIFI_Name: ESP32-CAM
WIFI_Password: MyPassword
Web address: http://192.168.4.1
*/
// webTask.h - Web server module
#pragma once

#define WIFI_SSID "ESP32-CAM"
#define WIFI_PASSWORD "MyPassword"

void WebTask_ListFiles();
void WebTask_HandleDownload();
void WebTask_HandleView();
void WebTask_Init();
void WebTask_HandleDelete();
void WebTask(void *pvParameters);