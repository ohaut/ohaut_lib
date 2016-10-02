#ifndef __CONFIG_MAP
#define __CONFIG_MAP

#include <functional>
#include <FS.h>

class ConfigEntry;
class ConfigEntry {
  public:
  char *key;
  char *value;
  ConfigEntry* next;

  ConfigEntry(const char *key, const char* value) {
    this->key = strdup(key);
    this->value = strdup(value);
    this->next = NULL;
  }
  ~ConfigEntry() {
    free(key);
    free(value);
  }
  void update(const char *value) {
    free(this->value);
    this->value = strdup(value);
  }
};

#define FOREACH_CALLBACK std::function<void(const char*, const char *, bool last)>

class ConfigMap {
  ConfigEntry *entry_list;
  public:
  ConfigMap() {
    entry_list = NULL;
  }
  ~ConfigMap() {
      ConfigEntry *next;
      while(entry_list) {
          next = entry_list->next;
          delete entry_list;
          entry_list = next;
      }
  }

  ConfigEntry* _find(const char *key) {
    ConfigEntry* p = entry_list;
    while (p) {
      if (strcmp(p->key, key)==0) return p;
      p = p->next;
    }
    return NULL;
  }

  void foreach(FOREACH_CALLBACK callback) {
    ConfigEntry *p = entry_list;
    while(p) {
      callback(p->key, p->value, p->next == NULL);
      p = p->next;
    }
  }

  void replaceVars(String& form) {
      foreach(
      [&form](const char* key, const char* value, bool last) {
          String var_name = "$";
          var_name += key;
          form.replace(var_name, (char*)value);
       });
  }

  String toJsonStr() {
     String json = "{";
     foreach([&json](const char* key, const char* value, bool last) {
        json += "\"";
        json += key;
        json += "\": \"";
        json += value;
        json += "\"";
        if (!last) json += ",\n ";
     });
     json += "}";
     return json;
  }

  void writeTSV(const char *filename) {
    File configFile = SPIFFS.open(filename, "w");
    foreach(
      [&configFile](const char* key, const char* value, bool last) {
        String str = key;
        str += '\t';
        str += value;
        str += '\n';
        configFile.write((const uint8_t*)str.c_str(), str.length());
    });
    configFile.close();
  }

  void readTSV(const char *filename) {
      File configFile = SPIFFS.open(filename, "r");

      String key="", value="";
      bool parsing_value=false;
      while (configFile.available()) {
        int val = configFile.read();
        if (val=='\n' && key.length()!=0) {
          // Line end, and we had a key
          set(key.c_str(), value.c_str());
          key = "";
          value = "";
          parsing_value = false;
        } else if (val=='\t') {
          // Tab switches from key parsing into value parsing
          parsing_value = true;
        } else {
          // Key or value parsing
          if (parsing_value) value += (char)val;
          else               key += (char)val;
        }
      }
      configFile.close();
  }

  void set(const char* key, const char* value) {
    ConfigEntry* p = _find(key);
    if (p) {
      p->update(value);
    } else {
      ConfigEntry *new_entry = new ConfigEntry(key, value);
      new_entry->next = entry_list;
      entry_list = new_entry;
    }
  }

  void set(const char* key, String value) {
    set(key, value.c_str());
  }

  char* operator[](const char* key) {
    ConfigEntry *p = _find(key);
    if (p) return p->value;
    return NULL;
  }

};
#endif
