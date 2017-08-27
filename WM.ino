bool doWifiConnect() {
  String _ssid = WiFi.SSID();
  String _psk = WiFi.psk();

  String _pskMask = "";
  for (int i = 0; i < _psk.length(); i++) {
    _pskMask += "*";
  }
  Serial.println("ssid = " + _ssid + ", psk = " + _pskMask);


  const char* ipStr = SonoffNetConfig.ip; byte ipBytes[4]; parseBytes(ipStr, '.', ipBytes, 4, 10);
  const char* netmaskStr = SonoffNetConfig.netmask; byte netmaskBytes[4]; parseBytes(netmaskStr, '.', netmaskBytes, 4, 10);
  const char* gwStr = SonoffNetConfig.gw; byte gwBytes[4]; parseBytes(gwStr, '.', gwBytes, 4, 10);

  if (!startWifiManager && _ssid != "" && _psk != "" ) {
    Serial.println(F("Connecting WLAN the classic way..."));
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.hostname(GlobalConfig.Hostname);
    WiFi.begin(_ssid.c_str(), _psk.c_str());
    int waitCounter = 0;
    if (String(SonoffNetConfig.ip) != "0.0.0.0")
      WiFi.config(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

    while (WiFi.status() != WL_CONNECTED) {
      waitCounter++;
      Serial.print(".");
      digitalWrite(LEDPinSwitch, (!(digitalRead(LEDPinSwitch))));
      digitalWrite(LEDPinPow, (!(digitalRead(LEDPinPow))));
      if (waitCounter == 20) {
        return false;
      }
      delay(500);
    }
    Serial.println("Wifi Connected");
    return true;
  } else {
    WiFiManager wifiManager;
    digitalWrite(LEDPinSwitch, LOW);
    digitalWrite(LEDPinPow, HIGH);
    wifiManager.setDebugOutput(wifiManagerDebugOutput);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    WiFiManagerParameter custom_ccuip("ccu", "IP der CCU2", GlobalConfig.ccuIP, IPSIZE, 0, "pattern='((^|\\.)((25[0-5])|(2[0-4]\\d)|(1\\d\\d)|([1-9]?\\d))){4}$'");
    //WiFiManagerParameter custom_loxusername("loxusername", "Loxone Username", "", VARIABLESIZE);
    //WiFiManagerParameter custom_loxpassword("loxpassword", "Loxone Password", "", VARIABLESIZE,4);
    WiFiManagerParameter custom_loxudpport("loxudpport", "Loxone UDP Port", LoxoneConfig.UDPPort, 10, 0, "pattern='[0-9]{1,5}'");
    WiFiManagerParameter custom_sonoffname("sonoff", "Sonoff Ger&auml;tename", GlobalConfig.DeviceName, VARIABLESIZE, 0, "pattern='[A-Za-z0-9]+'");
    char*chrRestoreOldState = "0";
    if (GlobalConfig.restoreOldRelayState) chrRestoreOldState =  "1" ;
    WiFiManagerParameter custom_cbrestorestate("restorestate", "Schaltzustand wiederherstellen: ", chrRestoreOldState, 8, 1);

    WiFiManagerParameter custom_powervariablename("hmpowervariable_pow", "Variable f&uuml;r Leistung", HomeMaticConfig.PowerVariableName, VARIABLESIZE, 0, "pattern='[A-Za-z0-9]+'");
    String del = String(GlobalConfig.MeasureInterval);
    char delBuf[8];
    del.toCharArray(delBuf, 8);
    WiFiManagerParameter custom_powermeasureinterval("custom_powermeasureinterval_pow", "Messintervall", delBuf, 8, 0, "pattern='[0-9]{1,4}'");

    String options = "";
    switch (GlobalConfig.BackendType) {
      case BackendType_HomeMatic:
        options = F("<option selected value='0'>HomeMatic</option><option value='1'>Loxone</option>");
        break;
      case BackendType_Loxone:
        options = F("<option value='0'>HomeMatic</option><option selected value='1'>Loxone</option>");
        break;
      default:
        options = F("<option value='0'>HomeMatic</option><option value='1'>Loxone</option>");
        break;
    }
    WiFiManagerParameter custom_backendtype("backendtype", "Backend", "", 8, 2, options.c_str());

    String model = "";
    switch (GlobalConfig.SonoffModel) {
      case SonoffModel_Switch:
        model = F("<option selected value='0'>Switch/S20</option><option value='1'>POW</option>");
        break;
      case SonoffModel_Pow:
        model = F("<option value='0'>Switch/S20</option><option selected value='1'>POW</option>");
        break;
      default:
        model = F("<option value='0'>Switch/S20</option><option value='1'>POW</option>");
        break;
    }
    WiFiManagerParameter custom_sonoffmodel("sonoffmodel", "Sonoff Modell", "", 8, 2, model.c_str());

    WiFiManagerParameter custom_ip("custom_ip", "IP-Adresse", (String(SonoffNetConfig.ip) != "0.0.0.0") ? SonoffNetConfig.ip : "", IPSIZE, 0, "pattern='((^|\\.)((25[0-5])|(2[0-4]\\d)|(1\\d\\d)|([1-9]?\\d))){4}$'");
    WiFiManagerParameter custom_netmask("custom_netmask", "Netzmaske", (String(SonoffNetConfig.netmask) != "0.0.0.0") ? SonoffNetConfig.netmask : "", IPSIZE, 0, "pattern='((^|\\.)((25[0-5])|(2[0-4]\\d)|(1\\d\\d)|([1-9]?\\d))){4}$'");
    WiFiManagerParameter custom_gw("custom_gw", "Gateway",  (String(SonoffNetConfig.gw) != "0.0.0.0") ? SonoffNetConfig.gw : "", IPSIZE, 0, "pattern='((^|\\.)((25[0-5])|(2[0-4]\\d)|(1\\d\\d)|([1-9]?\\d))){4}$'");
    WiFiManagerParameter custom_text("<br/><br><div>Statische IP (wenn leer, dann DHCP):</div>");
    wifiManager.addParameter(&custom_sonoffmodel);
    wifiManager.addParameter(&custom_ccuip);
    //wifiManager.addParameter(&custom_loxusername);
    //wifiManager.addParameter(&custom_loxpassword);
    wifiManager.addParameter(&custom_loxudpport);
    wifiManager.addParameter(&custom_sonoffname);
    wifiManager.addParameter(&custom_powervariablename);
    wifiManager.addParameter(&custom_powermeasureinterval);
    wifiManager.addParameter(&custom_cbrestorestate);
    wifiManager.addParameter(&custom_backendtype);
    wifiManager.addParameter(&custom_text);
    wifiManager.addParameter(&custom_ip);
    wifiManager.addParameter(&custom_netmask);
    wifiManager.addParameter(&custom_gw);

    String Hostname = "Sonoff-" + WiFi.macAddress();

    wifiManager.setConfigPortalTimeout(ConfigPortalTimeout);

    if (startWifiManager == true) {
      if (_ssid == "" || _psk == "" ) {
        wifiManager.resetSettings();
      }
      else {
        if (!wifiManager.startConfigPortal(Hostname.c_str())) {
          Serial.println(F("WM: failed to connect and hit timeout"));
          delay(500);
          ESP.restart();
        }
      }
    }

    wifiManager.setSTAStaticIPConfig(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

    wifiManager.autoConnect(Hostname.c_str());

    Serial.println(F("Wifi Connected"));
    Serial.println("CUSTOM STATIC IP: " + String(SonoffNetConfig.ip) + " Netmask: " + String(SonoffNetConfig.netmask) + " GW: " + String(SonoffNetConfig.gw));
    if (wm_shouldSaveConfig) {
      if (String(custom_ip.getValue()).length() > 5) {
        Serial.println("Custom IP Address is set!");
        strcpy(SonoffNetConfig.ip, custom_ip.getValue());
        strcpy(SonoffNetConfig.netmask, custom_netmask.getValue());
        strcpy(SonoffNetConfig.gw, custom_gw.getValue());

      } else {
        strcpy(SonoffNetConfig.ip,      "0.0.0.0");
        strcpy(SonoffNetConfig.netmask, "0.0.0.0");
        strcpy(SonoffNetConfig.gw,      "0.0.0.0");
      }

      GlobalConfig.restoreOldRelayState = (atoi(custom_cbrestorestate.getValue()) == 1);
      GlobalConfig.BackendType = (atoi(custom_backendtype.getValue()));
      GlobalConfig.SonoffModel = (atoi(custom_sonoffmodel.getValue()));

      strcpy(GlobalConfig.ccuIP, custom_ccuip.getValue());
      strcpy(GlobalConfig.DeviceName, custom_sonoffname.getValue());
      //strcpy(LoxoneConfig.Username, custom_loxusername.getValue());
      //strcpy(LoxoneConfig.Password, custom_loxpassword.getValue());
      strcpy(LoxoneConfig.UDPPort, custom_loxudpport.getValue());

      strcpy(HomeMaticConfig.PowerVariableName, custom_powervariablename.getValue());
      GlobalConfig.MeasureInterval = atoi(custom_powermeasureinterval.getValue());

      saveSystemConfig();

      delay(100);
      ESP.restart();
    }
    return true;
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("AP-Modus ist aktiv!");
}

void saveConfigCallback () {
  Serial.println("Should save config");
  wm_shouldSaveConfig = true;
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);
    str = strchr(str, sep);
    if (str == NULL || *str == '\0') {
      break;
    }
    str++;
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  IPAddress gw = WiFi.gatewayIP();
  Serial.print("Gateway Address: ");
  Serial.println(gw);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

