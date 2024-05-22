/*
# ESP - wireless.h

Place to devolve all the wireless communication functions to clean up .ino for further development.

Might turn into a class later idk.



*/

#pragma once



#include <esp_now.h>

void trimCharArray(char *inArray) {
  char nullChar[2] = "\0";              // This must be 2 char long
  int index = strcspn(inArray, "\n");  // Find the location of the newline char
  inArray[index] = *nullChar;
}

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP32: ESPNow Init Success");
  } else {
    Serial.println("ESP32: ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}


bool initScan = false;

// Scan for moduleRxs in AP mode
void ScanForRx() {
  int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, CHANNEL);  // Scan only on one channel
  // reset on each scan
  bool moduleRxFound = 0;
  memset(&moduleRx, 0, sizeof(moduleRx));
//  Serial.println("");
  if (scanResults == 0) {
#if WIFI_DEBUG == true    
    Serial.println("ESP32: No WiFi devices in AP Mode found");
#endif
  } else {
#if PRINT_SCAN_SUMMARY == true
    Serial.print("ESP32: Found ");
    Serial.print(scanResults);
    Serial.println(" devices ");
#endif
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (PRINTSCANRESULTS) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      delay(10);
      // Check if the current device starts with `moduleRx`
      if (SSID.indexOf("moduleRx") == 0) {
        // SSID of interest
      #if PRINT_SCAN_SUMMARY == true
        Serial.println("ESP32: Found a moduleRx.");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" [");
        Serial.print(BSSIDstr);
        Serial.print("]");
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      #endif
        // Get BSSID => Mac Address of the moduleRx
        int mac[6];
        if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
          for (int ii = 0; ii < 6; ++ii) {
            moduleRx.peer_addr[ii] = (uint8_t)mac[ii];
          }
        }

        moduleRx.channel = CHANNEL;  // pick a channel
        moduleRx.encrypt = 0;        // no encryption

        moduleRxFound = 1;
        // we are planning to have only one moduleRx in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }
  if (moduleRxFound) {
#if PRINT_MODULEFOUND == true
    Serial.println("ESP32: moduleRx Found, processing..");
    initScan = false;
#endif
  } else {
    if (!initScan){
#if PRINT_MODULEFOUND == true
    Serial.println("ESP32: moduleRx Not Found, trying again.");
#endif
    initScan = true;
    }
  }

  // clean up ram
  WiFi.scanDelete();
}


void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(moduleRx.peer_addr);
  Serial.print("ESP32: moduleRx Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}



// Check if the moduleRx is already paired with the moduleTx.
// If not, pair the moduleRx with moduleTx
bool manageRx() {
  if (moduleRx.channel == CHANNEL) {
    if (DELETEBEFOREPAIR) {
      deletePeer();
    }
#if PRINT_REMOTE_STATUS == true
    Serial.print("ESP32: moduleRx Status: ");
#endif
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(moduleRx.peer_addr);
    if (exists) {
      // moduleRx already paired.
#if PRINT_REMOTE_STATUS == true
      Serial.println("ESP32: Already Paired");
#endif
      return true;
    } else {
      // moduleRx not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&moduleRx);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("ESP32: Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what happened");
        return false;
      }
    }
  } else {
    // No moduleRx found to process
    Serial.println("ESP32: No moduleRx found to process");
    return false;
  }
}
