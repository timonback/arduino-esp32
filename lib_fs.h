#ifndef FS_LIB
#define FS_LIB

#define FORMAT_LITTLEFS_IF_FAILED true

#include "LittleFS.h"
#define SPIFFS LittleFS

void listDirCallback(fs::FS &fs, const char *dirname, void (*callback)(File &))
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    callback(file);

    if (file.isDirectory())
    {
      listDirCallback(fs, file.path(), callback);
    }

    file = root.openNextFile();
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  listDirCallback(fs, dirname, [](File &file)
                  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.print(file.name());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf(
          "  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour,
          tmstruct->tm_min, tmstruct->tm_sec);
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.path());
      Serial.print("  SIZE: ");
      Serial.print(file.size());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      Serial.printf(
          "  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour,
          tmstruct->tm_min, tmstruct->tm_sec);
    } });
}

void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println("Dir created");
  }
  else
  {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println("Dir removed");
  }
  else
  {
    Serial.println("rmdir failed");
  }
}

uint16_t swapEndian(uint16_t value)
{
  return (value >> 8) | (value << 8);
}

void readRGB565File(fs::FS &fs, const char *path, uint16_t *data, uint32_t length, uint32_t offset)
{
  File file = fs.open(path);
  if (!file)
  {
    Serial.println(F("Failed to open file for reading"));
    return;
  }

  // Seek to the specified offset
  file.seek(offset * sizeof(uint16_t) / sizeof(char));

  // Read the RGB565 data directly into the uint16_t array
  size_t bytesRead = file.readBytes((char *)data, length * sizeof(uint16_t));
  if (bytesRead != length * sizeof(uint16_t))
  {
    Serial.println("Failed to read the expected amount of data");
  }
  file.close();

  for (int i = 0; i < length; i++)
  {
    data[i] = swapEndian(data[i]);
  }
}

int readFileToLong(fs::FS &fs, const char *path, long *data)
{
  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return -1;
  }
  size_t bytesRead = file.readBytes((char *)data, sizeof(long));
  file.close();

  if (bytesRead != sizeof(long))
  {
    Serial.println("Failed to read the expected amount of data");
    return -2;
  }
  else
  {
    Serial.printf("Read long value: %ld\n", *data);
  }
  return 0;
}

int readFileToConstChar(fs::FS &fs, const char *path, const char *data, size_t max_length)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return -1;
  }
  size_t bytesRead = file.readBytes((char *)data, max_length - 1);
  if (bytesRead == 0)
  {
    Serial.println("Failed to read from file or file is empty");
    file.close();
    return -2;
  }
  file.close();
  Serial.printf("Read const char value: %s\n", *data);
  return 0;
}

void readFileToString(fs::FS &fs, const char *path, String &data)
{
  File file = SPIFFS.open(path);
  if (file)
  {
    String offsetVal = file.readStringUntil('\n');
    data = offsetVal;
    file.close();
  }
}

void readFileToConsole(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("File renamed");
  }
  else
  {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }
}

size_t getFreeSpace(fs::LittleFSFS &fs)
{
  size_t freeSpace = fs.totalBytes() - fs.usedBytes();
  Serial.printf("Free space: %ld bytes\n", freeSpace);
  return freeSpace;
}



void fs_setup()
{
#ifdef FEATURE_FS
  if (!SPIFFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    Serial.println(F("LittleFS Mount Failed"));
    return;
  }

  Serial.println(F("----list 1----"));
  listDir(SPIFFS, "/", 1);
#endif
}

#endif
