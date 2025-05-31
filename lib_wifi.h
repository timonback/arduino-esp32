#ifndef WIFI_H
#define WIFI_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Esp.h>

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "vm/instruction.h"
#include "vm/vm.h"
extern VM vm;

#include "secrets.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

static const char *htmlContent PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 control</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin euismod, purus a euismod
    rhoncus, urna ipsum cursus massa, eu dictum tellus justo ac justo. Quisque ullamcorper
    arcu nec tortor ullamcorper, vel fermentum justo fermentum. Vivamus sed velit ut elit
    accumsan congue ut ut enim. Ut eu justo eu lacus varius gravida ut a tellus. Nulla facilisi.
    Integer auctor consectetur ultricies. Fusce feugiat, mi sit amet bibendum viverra, orci leo
    dapibus elit, id varius sem dui id lacus.</p>
</body>
</html>
)";
static const size_t htmlContentLength = strlen_P(htmlContent);

static const char *templateContent PROGMEM = R"(
<!DOCTYPE html>
<html>
<body>
    <h1>Hello, %USER%</h1>
    <h3>Uptime: %UPTIME% seconds</h3>
    <p>Current time: %TIME%</p>
    <p>Space left: %SPACE% bytes</p>
    %FILES%
    <h2>Command</h2>
    <form action="/command">
        <textarea name="command" placeholder="Enter commands, one per line">
        display_brightness:50
        display_cursor:0,10

        display_fill_screen:green

        display_text_color:red
        display_text_size:2
        display_println:Hello, World!

        display_text_color:blue
        display_text_size:1
        display_println:This is a test.
        delay:5000
        </textarea>
        <input type="submit" value="Update">
    </form>
    <h2>Update Data</h2>
    <form action="/update">
        <input type="text" name="file" placeholder="Enter filename">
        <textarea name="data" placeholder="Enter data"></textarea>
        <input type="submit" value="Update">
    </form>
    <h2>Upload File</h2>
    <form action="/upload?file=invalid" method="POST" enctype="multipart/form-data">
        <input type="text" name="file" placeholder="Enter filename" onChange="this.form.action='/upload?file='+this.value">
        <input type="file" name="data">
        <input type="submit" value="Upload">
    </form>
</body>
</html>
)";
static const size_t templateContentLength = strlen_P(templateContent);

static AsyncWebServer server(80);

void string_split(const String &str, char delimiter, std::function<void(const String &)> callback)
{
    int start = 0;
    int end = str.indexOf(delimiter);
    while (end != -1)
    {
        callback(str.substring(start, end));
        start = end + 1;
        end = str.indexOf(delimiter, start);
    }
    callback(str.substring(start));
}

void server_begin()
{
#ifdef FEATURE_FS
    {
        File f = SPIFFS.open("/template.html", "w");
        assert(f);
        f.print(templateContent);
        f.close();
    }

    server.serveStatic("/res/", SPIFFS, "/")
        .setDefaultFile("index.html");
#endif

    server.on("/lorem.html", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    // need to cast to uint8_t*
    // if you do not, the const char* will be copied in a temporary String buffer
    request->send(200, "text/html", (uint8_t *)htmlContent, htmlContentLength); });

    server.rewrite("/", "/index.html");
    server.on(
        "/index.html", HTTP_GET,
        [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/template.html", "text/html", false, [](const String &var) -> String
                        {
      if (var == "USER") {
        return String("Timon");
      } else if (var == "UPTIME") {
        return String( millis() / 1000 );
      } else if (var == "TIME") {
        return timeClient.getFormattedTime();
      } else if (var == "SPACE") {
        return String(getFreeSpace(SPIFFS));
      } else if(var == "FILES") {
        static String filesList;
        filesList = "<ul>";
        // Helper function to accumulate file list
        void (*fileListCallback)(File &) = [](File &file) {
            if (!file.isDirectory()) {
                filesList += "<li>";

                filesList += "<a href=\"/res";
                filesList += file.path();
                filesList += "\">";

                filesList += file.path();

                filesList += " (";
                filesList += file.size();
                filesList += " bytes)";
                filesList += "</a>";

                filesList += " <a href=\"/delete?file=";
                filesList += file.path();
                filesList += "\">(delete)</a>";

                filesList += "</li>";
            }
        };
        listDirCallback(SPIFFS, "/", fileListCallback);
        filesList += "</ul>";
        return filesList;
      }


      return emptyString; }); });

    server.on(
        "/command", HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            Serial.printf("Received GET request for URL: %s\n", request->url());

            String command;
            if (request->hasParam("command"))
            {
                command = request->getParam("command")->value();
            }
            if (command.isEmpty())
            {
                String response = "{";
                response += "\"status\":\"Error\",";
                response += "\"message\":\"Command parameter missing\"";
                response += "}";
                request->send(400, "application/json", response);
            }
            else
            {
                string_split(command, '\n',
                             [](const String &line)
                             {
                                 if (!line.isEmpty())
                                 {
                                     Instruction *instruction = instructionFromString(line);
                                     if (instruction)
                                     {
                                         vm.queue(std::unique_ptr<Instruction>(instruction));
                                     }
                                     else
                                     {
                                         Serial.printf("Invalid instruction: %s\n", line.c_str());
                                     }
                                 }
                             });
                request->send(200, "application/json", "{\"status\":\"OK\"}");
            }
        });

    // curl -v -H "Content-Type: application/x-www-form-urlencoded" -d "file=offset" -d "data=10" http://192.168.1.38/update
    server.on(
        "/update", HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            Serial.printf("Received POST request for URL: %s\n", request->url());

            String file;
            if (request->hasParam("file"))
            {
                file = request->getParam("file")->value();
            }
            String data;
            if (request->hasParam("data"))
            {
                data = request->getParam("data")->value();
            }
            if (file.isEmpty() || data.isEmpty())
            {
                String response = "{";
                response += "\"status\":\"Error\",";
                response += "\"message\":\"File or data parameter missing\",";
                response += "\"file\":\"" + file + "\",";
                response += "\"data\":\"" + data + "\"";
                response += "}";
                request->send(400, "application/json", response);
            }
            else
            {
                writeFile(SPIFFS, file.c_str(), data.c_str());
                request->send(200, "application/json", "{\"status\":\"OK\"}");
            }
            // onNotFound will always be called after this, and will not override the response object if `/game_log` is requested
        });

    // curl -v -F "data=@starter.ino" http://192.168.1.38/upload?file=starter.ino
    server.on(
        "/upload", HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            String file;
            if (request->hasParam("file"))
            {
                file = request->getParam("file")->value();
            }

            if (request->getResponse())
            {
                // 400 File not available for writing
                return;
            }

            if (!SPIFFS.exists(file))
            {
                return request->send(400, "text/plain", "Nothing uploaded: "+file);
            }

            request->send(SPIFFS, file, "text/plain");
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            String file;
            if (request->hasParam("file"))
            {
                file = request->getParam("file")->value();
            }

            Serial.printf("Upload[local: %s, esp: %s]: start=%u, len=%u, final=%d\n", filename.c_str(), file.c_str(), index, len, final);

            if (!index)
            {
                request->_tempFile = SPIFFS.open(file, "w");

                if (!request->_tempFile)
                {
                    request->send(400, "text/plain", "File not available for writing: " + file);
                }
            }
            if (len)
            {
                request->_tempFile.write(data, len);
            }
            if (final)
            {
                request->_tempFile.close();
            }
        });

    server.on("/delete", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  String file;
                  if (request->hasParam("file"))
                  {
                      file = request->getParam("file")->value();
                  }
                  if (SPIFFS.exists(file))
                  {
                      if (SPIFFS.remove(file))
                      {
                          request->send(200, "text/plain", "File deleted successfully");
                      }
                      else
                      {
                          request->send(500, "text/plain", "Failed to delete file: " + file);
                      }
                  }
                  else
                  {
                      request->send(404, "text/plain", "File not found: " + file);
                  }
              });

    server.on(
        "/reboot", HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            Serial.println("Rebooting...");
            request->send(200, "text/plain", "Rebooting...");
            delay(1000);
            ESP.restart();
        });

    // server.onNotFound([](AsyncWebServerRequest *request)
    //   {
    // if (request->url().startsWith("/res") || request->url() == "/update" || request->url() == "/template.html") {
    //   return;  // response object already created by onRequestBody
    // }
    // request->send(404, "text/plain", "Not found"); });

    server.begin();
}

void wifi_setup()
{
#ifdef FEATURE_WIFI
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);

    int offset = 2;
    {
        String offsetVal;
        readFileToString(SPIFFS, "/offset", offsetVal);
        if (-20 < offsetVal.toInt() && offsetVal.toInt() < 20)
        {
            offset = offsetVal.toInt();
        }
        else
        {
            Serial.printf(F("Invalid offset value: %s\n"), offsetVal.c_str());
        }
    }
    timeClient.setTimeOffset(3600 * offset);

    timeClient.setUpdateInterval(60 * 60 * 1000);
    timeClient.begin();

    server_begin();
#endif
}

#endif