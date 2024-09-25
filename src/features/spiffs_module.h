
#ifndef SPIFFS_MODULE_H
#define SPIFFS_MODULE_H

#include "FS.h"
#include "../features/wcc_module.h"

#define FORMAT_SPIFFS_IF_FAILED true

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  String file_content = "";

  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return "";
  }

  Serial.println("- read from file:");
  while (file.available())
  {
    file_content += file.readString();
  }
  file.close();
  return file_content;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- message appended");
  }
  else
  {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("- file renamed");
  }
  else
  {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
}

void save_wcc_settings(fs::FS &fs, uint8_t *data, size_t len)
{
  Serial.printf("Save WCC settings\n");

  deleteFile(fs, "/wcc_settings.bin");

  File file = fs.open("/wcc_settings.bin", FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.write(data, len))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
  file.close();
}

uint8_t *read_wcc_settings(fs::FS &fs, size_t *wcc_data_len)
{

  File file = fs.open("/wcc_settings.bin");

  if (!file)
  {
    Serial.println("- failed to open file for reading");
    return NULL;
  }

  size_t avail_len = file.available();
  uint8_t *data = (uint8_t *)ps_calloc(avail_len, sizeof(uint8_t));

  *wcc_data_len = file.readBytes((char *)data, avail_len); // read all to buffer to buffer

  file.close();
  return data;
}

#endif