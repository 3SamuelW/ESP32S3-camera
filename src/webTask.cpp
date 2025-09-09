// webTask.cpp - Web server implementation
// This module implements a WiFi Access Point (AP) and HTTP server for file management on the SD card.
// Features: list, view, download, and delete image files via a web interface.
/*
WIFI_Name: ESP32-CAM
WIFI_Password: MyPassword
Web address: http://192.168.4.1
*/

#include <WiFi.h> // Include WiFi library for ESP32
#include <WebServer.h> // Include WebServer library for HTTP server
#include <SPI.h> // Include SPI library for SD card communication
#include <SD.h> // Include SD card library
#include "webTask.h" // Include header for this module

// WiFi AP credentials (SSID and password for the ESP32 AP)
const char *apSsid = WIFI_SSID; // SSID for the AP
const char *apPassword = WIFI_PASSWORD; // Password for the AP
// HTTP server instance on port 80 (default HTTP port)
WebServer webServer(80);

// List all image files on the SD card and serve an HTML page for file management
// This function generates an HTML page listing all .jpg and .png files on the SD card root directory.
// Each file has options to view, download, or delete it via the web interface.
void WebTask_ListFiles() {
    File root = SD.open("/"); // Open the root directory of the SD card
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
      <meta charset='utf-8'>
      <title>TF Web Server - 3SamuelW</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body {
          font-family: 'Segoe UI', 'Arial', sans-serif;
          background: linear-gradient(135deg, #e0eafc 0%, #cfdef3 100%);
          margin: 0;
          padding: 0;
        }
        .container {
          max-width: 700px;
          margin: 40px auto 20px auto;
          background: #fff;
          border-radius: 18px;
          box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.18);
          padding: 32px 24px 24px 24px;
        }
        h2 {
          text-align: center;
          color: #2d3a4b;
          margin-bottom: 10px;
        }
        .author {
          text-align: center;
          color: #888;
          font-size: 15px;
          margin-bottom: 24px;
        }
        ul {
          list-style: none;
          padding: 0;
        }
        li {
          background: #f7faff;
          margin: 18px 0;
          padding: 18px 16px;
          border-radius: 12px;
          box-shadow: 0 2px 8px rgba(0,0,0,0.06);
          display: flex;
          flex-wrap: wrap;
          align-items: center;
          justify-content: space-between;
        }
        .filename {
          font-weight: 500;
          color: #2d3a4b;
          flex: 1 1 180px;
          word-break: break-all;
        }
        .actions {
          display: flex;
          gap: 12px;
        }
        .actions a {
          display: inline-block;
          padding: 6px 14px;
          border-radius: 6px;
          background: #007bff;
          color: #fff;
          font-size: 15px;
          text-decoration: none;
          transition: background 0.2s;
          box-shadow: 0 1px 3px rgba(0,0,0,0.07);
        }
        .actions a:hover {
          background: #0056b3;
        }
        .actions .delete {
          background: #e74c3c;
        }
        .actions .delete:hover {
          background: #b93222;
        }
        @media (max-width: 600px) {
          .container { padding: 12px 4px; }
          li { flex-direction: column; align-items: flex-start; }
          .actions { width: 100%; gap: 8px; margin-top: 8px; }
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>📂 Photo Sets Online</h2>
        <div class="author">By 3SamuelW</div>
        <ul>
  )rawliteral";
    while (true) {
        File entry = root.openNextFile(); // Get next file in root directory
        if (!entry) break; // Exit loop if no more files
        String name = entry.name(); // Get file name
        // Only show .jpg and .png files, skip directories
        if (!entry.isDirectory() && (name.endsWith(".jpg") || name.endsWith(".png"))) {
            html += "<li>"; // Start list item
            html += "<span class='filename'>📄 " + name + "</span>"; // Show file name
            html += "<div class='actions'>"; // Start actions div
            html += "<a href='/view?file=" + name + "' target='_blank'>View</a>"; // View link
            html += "<a href='/download?file=" + name + "'>Download</a>"; // Download link
            html += "<a href='/delete?file=" + name + "' class='delete' onclick=\"return confirm('Delete " + name + "?')\">Delete</a>"; // Delete link with confirmation
            html += "</div></li>"; // End list item
        }
        entry.close(); // Close file entry
    }
    html += R"rawliteral(
        </ul>
      </div>
    </body>
    </html>
  )rawliteral";
    webServer.send(200, "text/html", html); // Send HTML page to client
    Serial.println("[WebTask] File list served to client."); // Debug output
}

// Handle file download requests. Sends the requested file as an attachment.
// This function checks for the 'file' parameter, opens the file, and streams it to the client.
void WebTask_HandleDownload() {
    if (!webServer.hasArg("file")) { // Check if 'file' parameter is present
        webServer.send(400, "text/plain", "Missing file parameter"); // Send error if missing
        Serial.println("[WebTask] Download failed: missing file parameter.");
        return;
    }
    String filename = webServer.arg("file"); // Get file name from request
    File file = SD.open("/" + filename); // Open file from SD card
    if (!file) { // If file not found
        webServer.send(404, "text/plain", "File not found"); // Send 404 error
        Serial.printf("[WebTask] Download failed: file not found (%s).\n", filename.c_str());
        return;
    }
    String contentType = "application/octet-stream"; // Set content type for download
    webServer.sendHeader("Content-Type", contentType); // Set HTTP headers
    webServer.sendHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    webServer.sendHeader("Connection", "close");
    webServer.streamFile(file, contentType); // Stream file to client
    file.close(); // Close file
    Serial.printf("[WebTask] File downloaded: %s\n", filename.c_str()); // Debug output
}

// Handle image view requests. Streams the image file to the browser.
// This function checks for the 'file' parameter, opens the file, and streams it as an image.
void WebTask_HandleView() {
    if (!webServer.hasArg("file")) { // Check if 'file' parameter is present
        webServer.send(400, "text/plain", "Missing file parameter"); // Send error if missing
        Serial.println("[WebTask] View failed: missing file parameter.");
        return;
    }
    String filename = webServer.arg("file"); // Get file name from request
    File file = SD.open("/" + filename); // Open file from SD card
    if (!file) { // If file not found
        webServer.send(404, "text/plain", "Image not found"); // Send 404 error
        Serial.printf("[WebTask] View failed: image not found (%s).\n", filename.c_str());
        return;
    }
    String contentType = filename.endsWith(".jpg") ? "image/jpeg" : "image/png"; // Determine content type
    webServer.streamFile(file, contentType); // Stream image to client
    file.close(); // Close file
    Serial.printf("[WebTask] Image viewed: %s\n", filename.c_str()); // Debug output
}

// Handle file delete requests. Removes the file from SD card and redirects to home.
// This function checks for the 'file' parameter, deletes the file, and redirects to the main page.
void WebTask_HandleDelete() {
    if (!webServer.hasArg("file")) { // Check if 'file' parameter is present
        webServer.send(400, "text/plain", "Missing file parameter"); // Send error if missing
        Serial.println("[WebTask] Delete failed: missing file parameter.");
        return;
    }
    String filename = webServer.arg("file"); // Get file name from request
    if (SD.exists("/" + filename)) { // Check if file exists
        SD.remove("/" + filename); // Delete file from SD card
        webServer.sendHeader("Location", "/"); // Redirect to home page
        webServer.send(302, "text/plain", "Redirecting to home..."); // Send redirect response
        Serial.printf("[WebTask] File deleted: %s\n", filename.c_str()); // Debug output
    } else {
        webServer.send(404, "text/plain", "File not found"); // Send 404 error
        Serial.printf("[WebTask] Delete failed: file not found (%s).\n", filename.c_str()); // Debug output
    }
}

// Initialize WiFi AP and start the HTTP server with all routes
// This function sets up the ESP32 as a WiFi AP, starts the HTTP server, and registers all URL handlers.
void WebTask_Init() {
    WiFi.softAP(apSsid, apPassword); // Start WiFi AP
    IPAddress ip = WiFi.softAPIP(); // Get AP IP address
    Serial.print("[WebTask] AP started. IP address: ");
    Serial.println(ip); // Print IP address
    webServer.on("/", HTTP_GET, WebTask_ListFiles); // Register handler for file list
    webServer.on("/view", HTTP_GET, WebTask_HandleView); // Register handler for image view
    webServer.on("/download", HTTP_GET, WebTask_HandleDownload); // Register handler for download
    webServer.on("/delete", HTTP_GET, WebTask_HandleDelete); // Register handler for delete
    webServer.onNotFound([]() { // Handler for unknown URLs
        webServer.send(404, "text/plain", "404: Not Found");
        Serial.println("[WebTask] 404 Not Found.");
    });
    webServer.begin(); // Start HTTP server
    Serial.println("[WebTask] Web server started."); // Debug output
}

// Main web server task loop. Handles incoming HTTP requests.
// This function should be run as a FreeRTOS task. It continuously processes HTTP requests.
void WebTask(void *pvParameters) {
    while (1) {
        webServer.handleClient(); // Handle incoming HTTP client requests
        vTaskDelay(30 / portTICK_PERIOD_MS); // Small delay to yield CPU
    }
}