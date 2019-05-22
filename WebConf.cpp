#include <map>
#include <algorithm>
#include "WebConf.h"
#include "common.h"

#define WEBCONF_STATUS_ENABLED (this->_statusPin >= 0)

//When INIT_BUTTON is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define INIT_BUTTON D3

//Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

//We might want to place the config in the EEPROM in an offset.
#define WEBCONF_CONFIG_START 5

//User name on login.
#define WEBCONF_ADMIN_USER_NAME "admin"

//HTML page fragments
const char WEBCONF_HTTP_HEAD[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char WEBCONF_HTTP_STYLE[] PROGMEM = "<style>.de{background-color:#ffaaaa;} .em{font-size:0.8em;color:#bb0000;padding-bottom:0px;} .c{text-align: center;} div,input,select{padding:5px;font-size:1em;} input{width:95%;} select{width:100%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}</style>";
const char WEBCONF_HTTP_SCRIPT[] PROGMEM = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char WEBCONF_HTTP_HEAD_END[] PROGMEM = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char WEBCONF_HTTP_FORM_START[] PROGMEM = "<form action='' method='post'><fieldset><input type='hidden' name='iotSave' value='true'>";
const char WEBCONF_HTTP_FORM_PARAM[] PROGMEM = "<div class='{s}'><label for='{i}'>{b}</label><input type='{t}' id='{i}' name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}/><div class='em'>{e}</div></div>";
const char WEBCONF_HTTP_FORM_PARAM_SERIAL_BAUD[] PROGMEM = "<div class='{s}'><label for='{i}'>{b}</label><select type='{t}' id='{i}' name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}>  <option value ='115200'>115200</option>  <option value ='110'>110</option>  <option value ='300'>300</option>  <option value ='600'>600</option>  <option value ='1200'>1200</option>  <option value ='2400'>2400</option>  <option value ='4800'>4800</option>  <option value ='9600'>9600</option>  <option value ='19200'>19200</option>  <option value ='28800'>28800</option>  <option value ='38400'>38400</option>  <option value ='43000'>43000</option>  <option value ='56000'>56000</option>  <option value ='57600'>57600</option>  <option value ='128000'>128000</option>  <option value ='256000'>256000</option>  <option value ='512000'>512000</option>  <option value ='921600'>921600</option></select><div class='em'>{e}</div></div>";
const char WEBCONF_HTTP_FORM_PARAM_SERIAL_CONFIG[] PROGMEM = "<div class='{s}'><label for='{i}'>{b}</label><select type='{t}' id='{i}' name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}> <option value ='SERIAL_8N1'>SERIAL_8N1</option> <option value ='SERIAL_5N1'>SERIAL_5N1</option> <option value ='SERIAL_6N1'>SERIAL_6N1</option> <option value ='SERIAL_7N1'>SERIAL_7N1</option> <option value ='SERIAL_5N2'>SERIAL_5N2</option> <option value ='SERIAL_6N2'>SERIAL_6N2</option> <option value ='SERIAL_7N2'>SERIAL_7N2</option> <option value ='SERIAL_8N2'>SERIAL_8N2</option> <option value ='SERIAL_5E1'>SERIAL_5E1</option> <option value ='SERIAL_6E1'>SERIAL_6E1</option> <option value ='SERIAL_7E1'>SERIAL_7E1</option> <option value ='SERIAL_8E1'>SERIAL_8E1</option> <option value ='SERIAL_5E2'>SERIAL_5E2</option> <option value ='SERIAL_6E2'>SERIAL_6E2</option> <option value ='SERIAL_7E2'>SERIAL_7E2</option> <option value ='SERIAL_8E2'>SERIAL_8E2</option> <option value ='SERIAL_5O1'>SERIAL_5O1</option> <option value ='SERIAL_6O1'>SERIAL_6O1</option> <option value ='SERIAL_7O1'>SERIAL_7O1</option> <option value ='SERIAL_8O1'>SERIAL_8O1</option> <option value ='SERIAL_5O2'>SERIAL_5O2</option> <option value ='SERIAL_6O2'>SERIAL_6O2</option> <option value ='SERIAL_7O2'>SERIAL_7O2</option> <option value ='SERIAL_8O2'>SERIAL_8O2</option> </select><div class='em'>{e}</div></div>";

const char WEBCONF_HTTP_FORM_END[] PROGMEM = "</fieldset><button type='submit'>Apply</button></form>";
const char WEBCONF_HTTP_SAVED[] PROGMEM = "<div>Condiguration saved<br />Return to <a href='/'>home page</a>.</div>";
const char WEBCONF_HTTP_END[] PROGMEM = "</div></body></html>";
const char WEBCONF_HTTP_UPDATE[] PROGMEM = "<div style='padding-top:25px;'><a href='{u}'>Firmware update</a></div>";
const char WEBCONF_HTTP_CONFIG_VER[] PROGMEM = "<div style='font-size: .6em;'>Firmware config version '{v}'</div>";

std::map<String, enum SerialConfig> SerialConfigMap{
    {"SERIAL_5N1",SERIAL_5N1},
    {"SERIAL_6N1",SERIAL_6N1},
    {"SERIAL_7N1",SERIAL_7N1},
    {"SERIAL_8N1",SERIAL_8N1},
    {"SERIAL_5N2",SERIAL_5N2},
    {"SERIAL_6N2",SERIAL_6N2},
    {"SERIAL_7N2",SERIAL_7N2},
    {"SERIAL_8N2",SERIAL_8N2},
    {"SERIAL_5E1",SERIAL_5E1},
    {"SERIAL_6E1",SERIAL_6E1},
    {"SERIAL_7E1",SERIAL_7E1},
    {"SERIAL_8E1",SERIAL_8E1},
    {"SERIAL_5E2",SERIAL_5E2},
    {"SERIAL_6E2",SERIAL_6E2},
    {"SERIAL_7E2",SERIAL_7E2},
    {"SERIAL_8E2",SERIAL_8E2},
    {"SERIAL_5O1",SERIAL_5O1},
    {"SERIAL_6O1",SERIAL_6O1},
    {"SERIAL_7O1",SERIAL_7O1},
    {"SERIAL_8O1",SERIAL_8O1},
    {"SERIAL_5O2",SERIAL_5O2},
    {"SERIAL_6O2",SERIAL_6O2},
    {"SERIAL_7O2",SERIAL_7O2},
    {"SERIAL_8O2",SERIAL_8O2},
};
enum SerialConfig getSerialConfigEnum(String SerialConfigStr)
{
    if (SerialConfigMap.count(SerialConfigStr) == 0)
    {
        return SERIAL_8N1;
    }
    else
    {
        return SerialConfigMap[SerialConfigStr];
    }
}

// DNSServer dnsServer;
ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
void configSaved();

WebConfParameter::WebConfParameter()
{
}
WebConfParameter::WebConfParameter(
    const char *label,
    const char *id,
    char *valueBuffer,
    int length,
    const char *type,
    const char *placeholder,
    const char *defaultValue,
    const char *customHtml)
{
    this->label = label;
    this->id = id;
    this->valueBuffer = valueBuffer;
    this->length = length;
    this->type = type;
    this->placeholder = placeholder;
    this->customHtml = customHtml;
}
WebConfParameter::WebConfParameter(
    const char *id,
    char *valueBuffer,
    int length,
    const char *customHtml,
    const char *type)
{
    this->label = NULL;
    this->id = id;
    this->valueBuffer = valueBuffer;
    this->length = length;
    this->type = type;
    this->customHtml = customHtml;
}

WebConfSeparator::WebConfSeparator() : WebConfParameter(NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL)
{
}

////////////////////////////////////////////////////////////////

WebConf::WebConf(const char *defaultThingName,
                 const char *initialApPassword,
                 const char *configVersion)
{
    strncpy(this->_thingName, defaultThingName, WEBCONF_WORD_LEN);
    String suffix = "-" + String(ESP.getChipId(), HEX);
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), (int (*)(int))toupper);
    strncat(this->_thingName, suffix.c_str(), suffix.length());

    // this->_dnsServer = &dnsServer;
    this->_server = &webServer;
    this->_initialApPassword = initialApPassword;
    //this->_configVersion = configVersion;
    strncpy(this->_configVersion, configVersion, WEBCONF_CONFIG_VESION_LENGTH);
    itoa(this->_apTimeoutMs / 1000, this->_apTimeoutStr, 10);

    this->_thingNameParameter = WebConfParameter("Thing name", "iwcThingName", this->_thingName, WEBCONF_WORD_LEN);
    this->_apPasswordParameter = WebConfParameter("AP password", "iwcApPassword", this->_apPassword, WEBCONF_WORD_LEN, "password", "********");
    this->_wifiSsidParameter = WebConfParameter("WiFi SSID", "iwcWifiSsid", this->_wifiSsid, WEBCONF_WORD_LEN);
    this->_wifiPasswordParameter = WebConfParameter("WiFi password", "iwcWifiPassword", this->_wifiPassword, WEBCONF_WORD_LEN, "password", "********");
    this->_apTimeoutParameter = WebConfParameter("Startup delay (seconds)", "iwcApTimeout", this->_apTimeoutStr, WEBCONF_WORD_LEN, "number", NULL, "min='5' max='600'");
    this->_SerialBaudRateParameter = WebConfParameter("Serial Baud Rate", "iwcSerialBaudRate", this->_SerialBaudRateStr, WEBCONF_WORD_LEN, "SerialBaudRate");
    this->_SerialConfigParameter = WebConfParameter("Serial Config", "iwcSerialConfig", this->_SerialConfigStr, WEBCONF_WORD_LEN, "SerialConfig");

	this->_scanAutoParameter = WebConfParameter("Loadbalancer", "scanAuto", this->_scanAuto, WEBCONF_WORD_LEN, "AutoScan");
    this->_dhcpAutoParameter = WebConfParameter("DHCP", "dhcpAuto", this->_dhcpAuto, WEBCONF_WORD_LEN, "DHCP");
    this->_ipParameter = WebConfParameter("IP", "ip", this->_ip, WEBCONF_WORD_LEN, "IP");
    this->_netmaskParameter = WebConfParameter("Netmask", "netmask", this->_netmask, WEBCONF_WORD_LEN, "IP");
    this->_gwParameter = WebConfParameter("Gateway", "gw", this->_gw, WEBCONF_WORD_LEN, "IP");
    // this->_ip1Parameter = WebConfParameter("dns", "dns", this->_dns, 3, "IP");

	this->_brokerIpParameter = WebConfParameter("Broker Ip", "brokerIp", this->_brokerIp, WEBCONF_WORD_LEN, "IP");
	this->_brokerPortParameter = WebConfParameter("Broker Port", "brokerPort", this->_brokerPortStr, WEBCONF_WORD_LEN, "number");

    this->addParameter(&this->_thingNameParameter);
    this->addParameter(&this->_apPasswordParameter);
    this->addParameter(&this->_wifiSsidParameter);
    this->addParameter(&this->_wifiPasswordParameter);
    this->addParameter(&this->_apTimeoutParameter);
    this->addParameter(&this->_SerialBaudRateParameter);
    this->addParameter(&this->_SerialConfigParameter);

	this->addParameter(&this->_scanAutoParameter);
    this->addParameter(&this->_dhcpAutoParameter);
    this->addParameter(&this->_ipParameter);
    this->addParameter(&this->_netmaskParameter);
    this->addParameter(&this->_gwParameter);

	this->addParameter(&this->_brokerIpParameter);
    this->addParameter(&this->_brokerPortParameter);
}

char *WebConf::getThingName()
{
    return this->_thingName;
}

void WebConf::setConfigPin(int configPin)
{
    this->_configPin = configPin;
}

void WebConf::setStatusPin(int statusPin)
{
    this->_statusPin = statusPin;
}

void WebConf::setupUpdateServer(ESP8266HTTPUpdateServer *updateServer, const char *updatePath)
{
    this->_updateServer = updateServer;
    this->_updatePath = updatePath;
}

boolean WebConf::init()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    webConf.setStatusPin(LED_BUILTIN);
    webConf.setConfigPin(INIT_BUTTON);
    // webConf.setConfigSavedCallback(&configSaved);
    webConf.setupUpdateServer(&httpUpdater);

    //Setup pins.
    if (this->_configPin >= 0)
    {
        pinMode(this->_configPin, INPUT_PULLUP);
        this->_forceDefaultPassword = (digitalRead(this->_configPin) == LOW);
    }
    if (WEBCONF_STATUS_ENABLED)
    {
        pinMode(this->_statusPin, OUTPUT);
        digitalWrite(this->_statusPin, WEBCONF_STATUS_ON);
    }

    //Load configuration from EEPROM.
    this->configInit();
    boolean validConfig = this->configLoad();
    if (!validConfig)
    {
        //No config
        strncpy(this->_apPassword, INIT_AP_PASSWORD, WEBCONF_WORD_LEN);
        //this->_apPassword[0] != '\0';
        this->_wifiSsid[0] = '\0';
        this->_wifiPassword[0] = '\0';
        this->_apTimeoutMs = WEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS;
        this->_SerialBaudRate = 115200;
        this->_SerialConfig = SERIAL_8N1;
		strncpy(this->_scanAuto, "False", WEBCONF_WORD_LEN);
        strncpy(this->_dhcpAuto, "True", WEBCONF_WORD_LEN);

		strncpy(this->_brokerIp, DEFAULT_BROKER_IP, WEBCONF_WORD_LEN);
		strncpy(this->_brokerPortStr, DEFAULT_BROKER_PORT, WEBCONF_WORD_LEN);
        //this->_dhcpAuto[0] = '\0';
    }
    else
    {
        this->_apTimeoutMs = atoi(this->_apTimeoutStr) * 1000;
        this->_SerialBaudRate = atoi(this->_SerialBaudRateStr);
        this->_SerialConfig = getSerialConfigEnum(this->_SerialConfigStr);

		this->_brokerPort = atoi(this->_brokerPortStr);
		if(strcmp(this->_scanAuto, "True") == 0)
			this->autoScan = true;
		else
			this->autoScan = false;
    }

    //Set up required URL handlers on the web server.
    webServer.on("/", [] { webConf.handleRoot(); });
    webServer.on("/config", [] { webConf.handleConfig(); });
    webServer.onNotFound([]() { webConf.handleNotFound(); });

    return validConfig;
}

//////////////////////////////////////////////////////////////////

bool WebConf::addParameter(WebConfParameter *parameter)
{
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("Adding parameter '");
    Serial.print(parameter->id);
    Serial.println("'");
#endif
    if (this->_firstParameter == NULL)
    {
        this->_firstParameter = parameter;
        WEBCONF_DEBUG_LINE(F("Adding as first"));
        return true;
    }
    WebConfParameter *current = this->_firstParameter;
    while (current->_nextParameter != NULL)
    {
        current = current->_nextParameter;
    }

    current->_nextParameter = parameter;
    return true;
}

void WebConf::configInit()
{
    int size = 0;
    WebConfParameter *current = this->_firstParameter;
    while (current != NULL)
    {
        size += current->length;
        current = current->_nextParameter;
    }
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("Config size: ");
    Serial.println(size);
#endif

    EEPROM.begin(WEBCONF_CONFIG_START + WEBCONF_CONFIG_VESION_LENGTH + size);
}

/**
 * Load the configuration from the eeprom.
 */
boolean WebConf::configLoad()
{
    if (this->configTestVersion())
    {
        WebConfParameter *current = this->_firstParameter;
        int start = WEBCONF_CONFIG_START + WEBCONF_CONFIG_VESION_LENGTH;
        while (current != NULL)
        {
            if (current->id != NULL)
            {
                this->readEepromValue(start, current->valueBuffer, current->length);
#ifdef WEBCONF_DEBUG_TO_SERIAL
                Serial.print("Loaded config '");
                Serial.print(current->id);
                Serial.print("'= '");
                Serial.print(current->valueBuffer);
                Serial.println("'");
#endif

                start += current->length;
            }
            current = current->_nextParameter;
        }
        return true;
    }
    else
    {
        WEBCONF_DEBUG_LINE(F("Wrong config version."));
        return false;
    }
}

void WebConf::configSave()
{
    this->configSaveConfigVersion();
    WebConfParameter *current = this->_firstParameter;
    int start = WEBCONF_CONFIG_START + WEBCONF_CONFIG_VESION_LENGTH;
    while (current != NULL)
    {
        if (current->id != NULL)
        {
#ifdef WEBCONF_DEBUG_TO_SERIAL
            Serial.print("Saving config '");
            Serial.print(current->id);
            Serial.print("'= '");
            Serial.print(current->valueBuffer);
            Serial.println("'");
#endif

            this->writeEepromValue(start, current->valueBuffer, current->length);
            start += current->length;
        }
        current = current->_nextParameter;
    }
    EEPROM.commit();

    // if (this->_configSavedCallback != NULL)
    // {
    //     this->_configSavedCallback();
    // }
}

void WebConf::readEepromValue(int start, char *valueBuffer, int length)
{
    for (unsigned int t = 0; t < (unsigned int)length; t++)
    {
        *((char *)valueBuffer + t) = EEPROM.read(start + t);
    }
}
void WebConf::writeEepromValue(int start, char *valueBuffer, int length)
{
    for (unsigned int t = 0; t < (unsigned int)length; t++)
    {
        EEPROM.write(start + t, *((char *)valueBuffer + t));
    }
}

boolean WebConf::configTestVersion()
{
    for (byte t = 0; t < WEBCONF_CONFIG_VESION_LENGTH; t++)
    {
        if (EEPROM.read(WEBCONF_CONFIG_START + t) != this->_configVersion[t])
        {
            return false;
        }
    }
    return true;
}

void WebConf::configSaveConfigVersion()
{
    for (byte t = 0; t < WEBCONF_CONFIG_VESION_LENGTH; t++)
    {
        EEPROM.write(WEBCONF_CONFIG_START + t, this->_configVersion[t]);
    }
    EEPROM.commit();
}

void WebConf::setWifiConnectionCallback(void (*func)(void))
{
    this->_wifiConnectionCallback = func;
}

void WebConf::setConfigSavedCallback(void (*func)(void))
{
    this->_configSavedCallback = func;
}

void WebConf::setFormValidator(boolean (*func)(void))
{
    this->_formValidator = func;
}

void WebConf::setWifiConnectionTimeoutMs(unsigned long millis)
{
    this->_wifiConnectionTimeoutMs = millis;
}

////////////////////////////////////////////////////////////////////////////////

void WebConf::handleConfig()
{
    // if (this->_state == WEBCONF_STATE_ONLINE)
    // {
    //     //Authenticate
    //     if (!this->_server->authenticate(WEBCONF_ADMIN_USER_NAME, this->_apPassword))
    //     {
    //         WEBCONF_DEBUG_LINE(F("Requesting authentication."));
    //         this->_server->requestAuthentication();
    //         return;
    //     }
    // }

    //Clean previous error messages.
    //boolean autoDHCP = false;
    WebConfParameter *current = this->_firstParameter;
    while (current != NULL)
    {
        current->errorMessage = NULL;
        current = current->_nextParameter;
    }

    if (!this->_server->hasArg("iotSave") || !this->validateForm())
    {
        //Display config portal
        WEBCONF_DEBUG_LINE(F("Configuration page requested."));
        String page = FPSTR(WEBCONF_HTTP_HEAD);
        page.replace("{v}", "Config");
        page += FPSTR(WEBCONF_HTTP_SCRIPT);
        page += FPSTR(WEBCONF_HTTP_STYLE);
        page += FPSTR(WEBCONF_HTTP_HEAD_END);

        page += FPSTR(WEBCONF_HTTP_FORM_START);
        char parLength[5];
        //Add parameters to the form
        WebConfParameter *current = this->_firstParameter;
        while (current != NULL)
        {
            if (current->id == NULL)
            {
#ifdef WEBCONF_DEBUG_TO_SERIAL
                Serial.println("Rendering separator");
#endif
                page += "</fieldset><fieldset>";
            }
            else
            {
#ifdef WEBCONF_DEBUG_TO_SERIAL
                Serial.print("Rendering '");
                Serial.print(current->id);
                Serial.print("' with value: ");
                Serial.println(current->valueBuffer);
#endif

                String pitem = FPSTR(WEBCONF_HTTP_FORM_PARAM);
                if (current->label != NULL)
                {
                    if (strcmp("SerialBaudRate", current->type) == 0)
                    {
                        pitem = FPSTR(WEBCONF_HTTP_FORM_PARAM_SERIAL_BAUD);
                        if (current->valueBuffer != NULL)
                        {
                            pitem.replace(">" + String(current->valueBuffer) + "</option>", "selected='selected'>" + String(current->valueBuffer) + "</option>");
                        }
                    }
                    if (strcmp("SerialConfig", current->type) == 0)
                    {
                        pitem = FPSTR(WEBCONF_HTTP_FORM_PARAM_SERIAL_CONFIG);
                        if (current->valueBuffer != NULL)
                        {
                            pitem.replace(">" + String(current->valueBuffer) + "</option>", "selected='selected'>" + String(current->valueBuffer) + "</option>");
                        }
                    }
                    if (strcmp("DHCP", current->type) == 0)
                    {
                        if (current->valueBuffer != NULL)
                        {
                            if (strcmp("True", current->valueBuffer) == 0)
                            {
                                pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><label for='dhcpAuto' style='padding-left:20px'>Auto</label><input type='radio' id='dhcpAuto' name='dhcpAuto' value='True' checked style='width:auto;padding-left:10px'><label for='dhcpAuto'style='padding-left:20px'>Manual</label><input type='radio' id='dhcpAuto' name='dhcpAuto' value='False' style='width:auto;padding-left:10px'><div class='em'>{e}</div></div>";
                                // pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><input type='checkbox' id='dhcpAuto' name='dhcpAuto' value='True' checked style='width:auto;margin-left:15px'><div class='em'>{e}</div></div>";
								//autoDHCP = true;
							}
                            else
                            {
                                pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><label for='dhcpAuto' style='padding-left:20px'>Auto</label><input type='radio' id='dhcpAuto' name='dhcpAuto' value='True' style='width:auto;padding-left:10px'><label for='dhcpAuto' style='padding-left:20px'>Manual</label><input type='radio' id='dhcpAuto' name='dhcpAuto' value='False' checked style='width:auto;padding-left:10px'><div class='em'>{e}</div></div>";
                                // pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><input type='checkbox' id='dhcpAuto' name='dhcpAuto' value='True' style='width:auto;margin-left:15px'><div class='em'>{e}</div></div>";
								//autoDHCP = false;
							}
                        }
                    }

					if (strcmp("AutoScan", current->type) == 0)
                    {
                        if (current->valueBuffer != NULL)
                        {
                            if (strcmp("True", current->valueBuffer) == 0)
                            {
                                pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><label for='scanAuto' style='padding-left:20px'>On</label><input type='radio' id='scanAuto' name='scanAuto' value='True' checked style='width:auto;padding-left:10px'><label for='scanAuto'style='padding-left:20px'>Off</label><input type='radio' id='scanAuto' name='scanAuto' value='False' style='width:auto;padding-left:10px'><div class='em'>{e}</div></div>";
                                // pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><input type='checkbox' id='dhcpAuto' name='dhcpAuto' value='True' checked style='width:auto;margin-left:15px'><div class='em'>{e}</div></div>";
								autoScan = true;
							}
                            else
                            {
                                pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><label for='scanAuto' style='padding-left:20px'>On</label><input type='radio' id='scanAuto' name='scanAuto' value='True' style='width:auto;padding-left:10px'><label for='scanAuto' style='padding-left:20px'>Off</label><input type='radio' id='scanAuto' name='scanAuto' value='False' checked style='width:auto;padding-left:10px'><div class='em'>{e}</div></div>";
                                // pitem = "<p></p><div class='{s}'><label for='{i}'>{b}</label><input type='checkbox' id='dhcpAuto' name='dhcpAuto' value='True' style='width:auto;margin-left:15px'><div class='em'>{e}</div></div>";
								autoScan = false;
							}
                        }
                    }

					/*
					if((strcmp("IP", current->label) == 0) && (autoDHCP == true))
					{
						current = current->_nextParameter;
						continue;
					}
					if((strcmp("Netmask", current->label) == 0) && (autoDHCP == true))
					{
						current = current->_nextParameter;
						continue;
					}
					if((strcmp("Gateway", current->label) == 0) && (autoDHCP == true))
					{
						current = current->_nextParameter;
						continue;
					}*/
					
                    pitem.replace("{b}", current->label);
                    pitem.replace("{t}", current->type);
                    pitem.replace("{i}", current->id);
                    pitem.replace("{p}", current->placeholder);
                    snprintf(parLength, 5, "%d", current->length);
                    pitem.replace("{l}", parLength);
                    if (strcmp("password", current->type) == 0)
                    {
                        //Value of password is not rendered
                        pitem.replace("{v}", "");
                    }
                    else if (this->_server->hasArg(current->id))
                    {
                        //Value from previous submit
                        pitem.replace("{v}", this->_server->arg(current->id));
                    }
                    else
                    {
                        //Value from config
                        pitem.replace("{v}", current->valueBuffer);
                    }
                    pitem.replace("{c}", current->customHtml);
                    pitem.replace("{e}", current->errorMessage);
                    pitem.replace("{s}", current->errorMessage == NULL ? "" : "de"); // Div style class.
                }
                else
                {
                    pitem = current->customHtml;
                }

                page += pitem;
            }
            current = current->_nextParameter;
        }

        page += FPSTR(WEBCONF_HTTP_FORM_END);

        if (this->_updatePath != NULL)
        {
            String pitem = FPSTR(WEBCONF_HTTP_UPDATE);
            pitem.replace("{u}", this->_updatePath);
            page += pitem;
        }

        //Fill config version string;
        {
            String pitem = FPSTR(WEBCONF_HTTP_CONFIG_VER);
            pitem.replace("{v}", this->_configVersion);
            page += pitem;
        }

        page += FPSTR(WEBCONF_HTTP_END);

        this->_server->sendHeader("Content-Length", String(page.length()));
        this->_server->send(200, "text/html", page);
    }
    else
    {
        //Save config
        WEBCONF_DEBUG_LINE(F("Updating configuration"));
        char temp[WEBCONF_WORD_LEN];

        WebConfParameter *current = this->_firstParameter;
        while (current != NULL)
        {
            if (current->id != NULL)
            {
                if ((strcmp("password", current->type) == 0) && (current->length <= WEBCONF_WORD_LEN))
                {
                    // TODO: Passwords longer than WEBCONF_WORD_LEN not supported.
                    this->readParamValue(current->id, temp, current->length);
                    if (temp[0] != '\0')
                    {
                        //Value was set.
                        strncpy(current->valueBuffer, temp, current->length);
#ifdef WEBCONF_DEBUG_TO_SERIAL
                        Serial.print(current->id);
                        Serial.println(" was set");
#endif
                    }
                    else
                    {
#ifdef WEBCONF_DEBUG_TO_SERIAL
                        Serial.print(current->id);
                        Serial.println(" was not changed");
#endif
                    }
                }
                else
                {
                    this->readParamValue(current->id, current->valueBuffer, current->length);
#ifdef WEBCONF_DEBUG_TO_SERIAL
                    Serial.print(current->id);
                    Serial.print("='");
                    Serial.print(current->valueBuffer);
                    Serial.println("'");
#endif
                }
            }
            current = current->_nextParameter;
        }

        this->configSave();

        String page = FPSTR(WEBCONF_HTTP_HEAD);
        page.replace("{v}", "Config");
        page += FPSTR(WEBCONF_HTTP_SCRIPT);
        page += FPSTR(WEBCONF_HTTP_STYLE);
        //    page += _customHeadElement;
        page += FPSTR(WEBCONF_HTTP_HEAD_END);
        page += "Configuration saved. Changes will take effect on the next startup.";
        if (this->_apPassword[0] == '\0')
        {
            page += F("You must change the default AP password to continue. Return to <a href=''>configuration page</a>.");
        }
        else if (this->_wifiSsid[0] == '\0')
        {
            page += F("You must provide the local wifi settings to continue. Return to <a href=''>configuration page</a>.");
        }
        else if (this->_state == WEBCONF_STATE_NOT_CONFIGURED)
        {
            page += F("Please disconnect from WiFi AP to continue!");
        }
        else
        {
            page += F("Return to <a href='/'>home page</a>.");
        }
        page += FPSTR(WEBCONF_HTTP_END);

        this->_server->sendHeader("Content-Length", String(page.length()));
        this->_server->send(200, "text/html", page);
    }
}

void WebConf::readParamValue(const char *paramName, char *target, unsigned int len)
{
    String value = this->_server->arg(paramName);
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("Value of arg '");
    Serial.print(paramName);
    Serial.print("' is:");
    Serial.println(value);
#endif
    value.toCharArray(target, len);
}

boolean WebConf::validateForm()
{
    //move to the begin of handleConfig(), to make the clean take effect
    // //Clean previous error messages.
    // WebConfParameter *current = this->_firstParameter;
    // while (current != NULL)
    // {
    //     current->errorMessage = NULL;
    //     current = current->_nextParameter;
    // }

    //Call external validator.
    boolean valid = true;
    if (this->_formValidator != NULL)
    {
        valid = this->_formValidator();
    }

    //Internal validation.
    int l = 0;
    int value = 0;
    if (this->_server->hasArg(this->_thingNameParameter.id))
    {
        l = this->_server->arg(this->_thingNameParameter.id).length();
        if (3 > l)
        {
            this->_thingNameParameter.errorMessage = "Give a name with at least 3 characters.";
            valid = false;
        }
    }
    if (this->_server->hasArg(this->_apPasswordParameter.id))
    {
        l = this->_server->arg(this->_apPasswordParameter.id).length();
        if ((0 < l) && (l < 8))
        {
            this->_apPasswordParameter.errorMessage = "Password length must be at least 8 characters.";
            valid = false;
        }
    }
    if (this->_server->hasArg(this->_wifiSsidParameter.id))
    {
        l = this->_server->arg(this->_wifiSsidParameter.id).length();
        if (3 > l)
        {
            this->_wifiSsidParameter.errorMessage = "Give a name with at least 3 characters.";
            valid = false;
        }
    }
    if (this->_server->hasArg(this->_wifiPasswordParameter.id))
    {
        l = this->_server->arg(this->_wifiPasswordParameter.id).length();
        if ((0 < l) && (l < 8))
        {
            this->_wifiPasswordParameter.errorMessage = "Password length must be at least 8 characters.";
            valid = false;
        }
    }
    if (this->_server->hasArg(this->_apTimeoutParameter.id))
    {
        value = this->_server->arg(this->_apTimeoutParameter.id).toInt();
        if ((600 < value) || (value < 5))
        {
            this->_apTimeoutParameter.errorMessage = "The timeout must be greater than 5 and less than 600.";
            valid = false;
        }
    }
    if (this->_server->arg(this->_ipParameter.id) != "")
    {
        if (!IPAddress::isValid(this->_server->arg(this->_ipParameter.id)))
        {
            this->_ipParameter.errorMessage = "Not valid.";
            valid = false;
        }
    }
    if (this->_server->arg(this->_netmaskParameter.id) != "")
    {
        if (!IPAddress::isValid(this->_server->arg(this->_netmaskParameter.id)))
        {
            this->_netmaskParameter.errorMessage = "Not valid.";
            valid = false;
        }
    }
    if (this->_server->arg(this->_gwParameter.id) != "")
    {
        if (!IPAddress::isValid(this->_server->arg(this->_gwParameter.id)))
        {
            this->_gwParameter.errorMessage = "Not valid.";
            valid = false;
        }
    }
    return valid;
}

void WebConf::handleNotFound()
{
    if (this->handleCaptivePortal())
    { // If captive portal redirect instead of displaying the error page.
        return;
    }
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("Requested non-existing page '");
    Serial.print(this->_server->uri());
    Serial.print("' arguments(");
    Serial.print(this->_server->method() == HTTP_GET ? "GET" : "POST");
    Serial.print("):");
    Serial.println(this->_server->args());
#endif
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += this->_server->uri();
    message += "\nMethod: ";
    message += (this->_server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += this->_server->args();
    message += "\n";

    for (uint8_t i = 0; i < this->_server->args(); i++)
    {
        message += " " + this->_server->argName(i) + ": " + this->_server->arg(i) + "\n";
    }
    this->_server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    this->_server->sendHeader("Pragma", "no-cache");
    this->_server->sendHeader("Expires", "-1");
    this->_server->sendHeader("Content-Length", String(message.length()));
    this->_server->send(404, "text/plain", message);
}

/**
 * Redirect to captive portal if we got a request for another domain.
 * Return true in that case so the page handler do not try to handle the request again.
 * (Code from WifiManager project.)
 */
boolean WebConf::handleCaptivePortal()
{
    String host = this->_server->hostHeader();
    String thingName = String(this->_thingName);
    thingName.toLowerCase();
    if (!isIp(host) && !host.startsWith(thingName))
    {
#ifdef WEBCONF_DEBUG_TO_SERIAL
        Serial.print("Request for ");
        Serial.print(host);
        Serial.print(" redirected to ");
        Serial.println(this->_server->client().localIP());
#endif
        this->_server->sendHeader("Location", String("http://") + toStringIp(this->_server->client().localIP()), true);
        this->_server->send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        this->_server->client().stop();             // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

/** Is this an IP? */
boolean WebConf::isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

/** IP to String */
String WebConf::toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

/** String to IP */
IPAddress WebConf::stringToIp(const char *address)
{
    IPAddress defaultAddress(0,0,0,0);
    uint8_t bytes[4];
    uint16_t acc = 0; // Accumulator
    uint8_t dots = 0;

    while (*address)
    {
        char c = *address++;
        if (c >= '0' && c <= '9')
        {
            acc = acc * 10 + (c - '0');
            if (acc > 255)
            {
                // Value out of [0..255] range
                return defaultAddress;
            }
        }
        else if (c == '.')
        {
            if (dots == 3)
            {
                // Too much dots (there must be 3 dots)
                return defaultAddress;
            }
            bytes[dots++] = acc;
            acc = 0;
        }
        else
        {
            // Invalid char
            return defaultAddress;
        }
    }

    if (dots != 3)
    {
        // Too few dots (there must be 3 dots)
        return defaultAddress;
    }
    bytes[3] = acc;
    return IPAddress(bytes);
}
/////////////////////////////////////////////////////////////////////////////////

void WebConf::doLoop()
{
	if(this->_state == WEBCONF_STATE_OFFLINE) return;
    doBlink();
    //Setup pins.
    /*
    if (this->_configPin >= 0)
    {
        this->_forceDefaultPassword = (digitalRead(this->_configPin) == LOW);
        if (this->_forceDefaultPassword == true)
        {
            //this->_apPasswordParameter = WebConfParameter("AP password", "iwcApPassword", INIT_AP_PASSWORD, WEBCONF_WORD_LEN, "password");
            this->_configVersion[0] = '\0';
			this->configSaveConfigVersion();
            //delay(1000);
            ESP.restart();
        }
    }*/
    if (this->_state == WEBCONF_STATE_BOOT)  // 刚开始初始化是 WEBCONF_STATE_BOOT
    {
        //After boot, fall immediately to AP mode.
        this->changeState(WEBCONF_STATE_AP_MODE);
#ifdef WEBCONF_DEBUG_TO_SERIAL
        Serial.print("In AP mode, timeout after ");
        Serial.print(this->_apTimeoutStr);
        Serial.print(" seconds if no client are connected");
#endif
    }
    else if (
        (this->_state == WEBCONF_STATE_NOT_CONFIGURED) || (this->_state == WEBCONF_STATE_AP_MODE))
    {
        //We must only leave the AP mode, when no slaves are connected.
        //Other than that AP mode has a timeout. E.g. after boot, or when retry connecting to WiFi
        checkConnection();
        checkApTimeout();
        // this->_dnsServer->processNextRequest();
        this->_server->handleClient();
    }
    else if (this->_state == WEBCONF_STATE_CONNECTING)
    {
        if (checkWifiConnection())
        {
            this->changeState(WEBCONF_STATE_ONLINE);
            checkConnection();
            this->_apStartTimeMs = millis() + this->_apTimeoutMs; //ap_sta -> sta timeout start
            // Serial.print("WEBCONF_STATE_CONNECTING _apStartTimeMs: ");
            // Serial.println(this->_apStartTimeMs);

            //Setup mdns
#ifdef WEBCONF_CONFIG_USE_MDNS
                MDNS.begin("this->_thingName", WiFi.localIP());
            MDNS.begin("this->_thingName");
            MDNS.addService("http", "tcp", 80);
#ifdef WEBCONF_DEBUG_TO_SERIAL
            Serial.println("MDNS.addService ");
#endif
#endif
            return;
        }
    }
    else if (this->_state == WEBCONF_STATE_ONLINE)
    {
        if (this->_apConnectionStatus != WEBCONF_AP_CONNECTION_STATE_CO)
        {
            checkConnection();
            checkApTimeout();
        }
        //In server mode we provide web interface. And check whether it is time to run the client.
        this->_server->handleClient();
        if (WiFi.status() != WL_CONNECTED)
        {
            this->changeState(WEBCONF_STATE_CONNECTING);
            return;
        }
    }

}

/**
 * What happens, when a state changed...
 * 改变一个状态对应也要进行相应的处理程序
 */
void WebConf::changeState(byte newState)
{
//     // switch (newState)
//     // {
//     // case WEBCONF_STATE_AP_MODE:
//     // {
//         //In AP mode we must override the default AP password. Otherwise we stay in STATE_NOT_CONFIGURED.
//         boolean forceDefaultPassword = false;
//         if (this->_forceDefaultPassword || (this->_apPassword[0] == '\0'))
//         {
// #ifdef WEBCONF_DEBUG_TO_SERIAL
//             if (this->_forceDefaultPassword)
//             {
//                 Serial.println("AP mode forced by reset pin");
//             }
//             else
//             {
//                 Serial.println("AP password was not set in configuration");
//             }
// #endif
//             newState = WEBCONF_STATE_NOT_CONFIGURED;
//         }
//     //     break;
//     // }
//     // default:
//     //     break;
//     // }
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("State changing from: ");
    Serial.print(this->_state);
    Serial.print(" to ");
    Serial.println(newState);
#endif
    byte oldState = this->_state;
    this->_state = newState;
    this->stateChanged(oldState, newState);
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("State changed from: ");
    Serial.print(oldState);
    Serial.print(" to ");
    Serial.println(newState);
#endif
}

/**
 * What happens, when a state changed...
 */
void WebConf::stateChanged(byte oldState, byte newState)
{
    //  updateOutput();
    switch (newState)
    {
    case WEBCONF_STATE_AP_MODE:
    case WEBCONF_STATE_NOT_CONFIGURED:
        if (newState == WEBCONF_STATE_AP_MODE)
        {
            this->blinkInternal(300, 90);
        }
        else
        {
            this->blinkInternal(300, 50);
        }
        setupAp();
        if (this->_updateServer != NULL)
        {
            this->_updateServer->setup(this->_server, this->_updatePath);
        }
        this->_server->begin();
        this->_apConnectionStatus = WEBCONF_AP_CONNECTION_STATE_NC;
        this->_apStartTimeMs = millis();
        break;
    case WEBCONF_STATE_CONNECTING:
        if ((oldState == WEBCONF_STATE_AP_MODE) || (oldState == WEBCONF_STATE_NOT_CONFIGURED))
        {
            // stopAp();
            WiFi.mode(WIFI_AP_STA);
        }
        this->blinkInternal(1000, 50);
#ifdef WEBCONF_DEBUG_TO_SERIAL
        Serial.print("Connecting to [");
        Serial.print(this->_wifiSsid);
        Serial.print("] with password [");
        Serial.print(this->_wifiPassword);
        Serial.println("]");
#endif
        this->_wifiConnectionStart = millis();
        WiFi.hostname(String(this->_thingName));
        if (strcmp("False", this->_dhcpAuto) == 0)
        {
            IPAddress ip = stringToIp(this->_ip);
            IPAddress gateway = stringToIp(this->_gw);
            IPAddress subnet = stringToIp(this->_netmask);

            WiFi.config(ip, gateway, subnet);
        }
        WiFi.begin(this->_wifiSsid, this->_wifiPassword);
        break;
    case WEBCONF_STATE_ONLINE:
        this->blinkInternal(8000, 2);
        if (this->_updateServer != NULL)
        {
            this->_updateServer->updateCredentials(WEBCONF_ADMIN_USER_NAME, this->_apPassword);
        }
        this->_server->begin();
        WEBCONF_DEBUG_LINE(F("Accepting connection"));
        if (this->_wifiConnectionCallback != NULL)
        {
            this->_wifiConnectionCallback();
        }
        break;
    default:
        break;
    }
}

// 是否超时
boolean WebConf::smallerCheckOverflow(unsigned long prevMillis, unsigned long diff, unsigned long currentMillis)
{
    if ((prevMillis < currentMillis) && ((prevMillis + diff) < prevMillis))
    //current does not overflows, but pref+diff does
    {
        return false;
    }
    return prevMillis + diff < currentMillis;
}

void WebConf::checkApTimeout()
{
	//MBOX_DEBUG_PRINTLN("checkApTimeout start");
	//MBOX_DEBUG_PRINTLN("this->_wifiSsid[0] = " + String(this->_wifiSsid[0]));
	//MBOX_DEBUG_PRINTLN("this->_apPassword[0] = " + String(this->_apPassword[0]));
	//MBOX_DEBUG_PRINTLN("this->_forceDefaultPassword = " + String(this->_forceDefaultPassword));
	
    if ((this->_wifiSsid[0] != '\0') && (this->_apPassword[0] != '\0') && (!this->_forceDefaultPassword))
    {
    	//MBOX_DEBUG_PRINTLN("_wifiSsid check ");
        //Only move on, when we have a valid WiFi and AP configured.
        if (
            // (this->_apConnectionStatus == WEBCONF_AP_CONNECTION_STATE_DC) ||
            (WebConf::smallerCheckOverflow(this->_apStartTimeMs, this->_apTimeoutMs, millis()) && (this->_apConnectionStatus != WEBCONF_AP_CONNECTION_STATE_C)))
        {
        	MBOX_DEBUG_PRINTLN("smallerCheckOverflow ");
			
            if(this->_state == WEBCONF_STATE_ONLINE)
            {
                // Serial.print("checkApTimeout _apStartTimeMs: ");
                // Serial.println(this->_apStartTimeMs);
                // Serial.print("checkApTimeout millis: ");
                // Serial.println(millis());

                WEBCONF_DEBUG_LINE(F("Stop AP."));
                stopAp();
                this->_apConnectionStatus = WEBCONF_AP_CONNECTION_STATE_CO;
            }
            else
            {
            	// 在刚开始 WEBCONF_STATE_AP_MODE（startAp）,到现在没有 client 请求连接,改为 WEBCONF_STATE_CONNECTING
                //this->changeState(WEBCONF_STATE_CONNECTING);
                // Serial.print("WEBCONF_STATE_CONNECTING checkApTimeout _apStartTimeMs: ");
                // Serial.println(this->_apStartTimeMs);

				
				this->_state = WEBCONF_STATE_OFFLINE;
				stopAp();
				
            }
        }
    }
}

/**
 * Checks whether we have anyone joined to our AP. 
 * If so, we must not change state. But when our guest leaved, we can immediately move on.
 */
void WebConf::checkConnection()
{
    // if ((this->_apConnectionStatus == WEBCONF_AP_CONNECTION_STATE_NC) && (WiFi.softAPgetStationNum() > 0))
    if ((this->_apConnectionStatus != WEBCONF_AP_CONNECTION_STATE_C) && (WiFi.softAPgetStationNum() > 0))
    {
        this->_apConnectionStatus = WEBCONF_AP_CONNECTION_STATE_C;
        WEBCONF_DEBUG_LINE(F("Connection to AP."));
    }
    else if ((this->_apConnectionStatus == WEBCONF_AP_CONNECTION_STATE_C) && (WiFi.softAPgetStationNum() == 0))
    {
        this->_apConnectionStatus = WEBCONF_AP_CONNECTION_STATE_DC;
        WEBCONF_DEBUG_LINE(F("Disconnected from AP."));
        if (this->_forceDefaultPassword)
        {
            WEBCONF_DEBUG_LINE(F("Releasing forced AP mode."));
            this->_forceDefaultPassword = false;
        }
    }
}

boolean WebConf::checkWifiConnection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (WebConf::smallerCheckOverflow(this->_wifiConnectionStart, this->_wifiConnectionTimeoutMs, millis()))
        {
            //WiFi not available, fall back to AP mode.
            WEBCONF_DEBUG_LINE(F("Giving up."));
            WiFi.disconnect(true);
            this->changeState(WEBCONF_STATE_AP_MODE);
        }
        return false;
    }

#ifdef WEBCONF_DEBUG_TO_SERIAL
    //Connected
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif

    return true;
}

void WebConf::setupAp()
{
    //set default ap lan
    IPAddress softLocal(192, 168, 5, 1);
    IPAddress softGateway(192, 168, 5, 1);
    IPAddress softSubnet(255, 255, 255, 0);
    WiFi.softAPConfig(softLocal, softGateway, softSubnet);

    WiFi.mode(WIFI_AP);

#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print("Setting up AP: ");
    Serial.println(this->_thingName);
#endif
    if (this->_state == WEBCONF_STATE_NOT_CONFIGURED)
    {
#ifdef WEBCONF_DEBUG_TO_SERIAL
        Serial.print("With default password: ");
        Serial.println(this->_initialApPassword);
#endif
        WiFi.softAP(this->_thingName, this->_initialApPassword);
    }
    else
    {
#ifdef WEBCONF_DEBUG_TO_SERIAL
        Serial.print("Use password: ");
        Serial.println(this->_apPassword);
#endif
        WiFi.softAP(this->_thingName, this->_apPassword);
    }

#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.print(F("AP IP address: "));
    Serial.println(WiFi.softAPIP());
#endif
    //  delay(500); // Without delay I've seen the IP address blank
    //  Serial.print(F("AP IP address: "));
    //  Serial.println(WiFi.softAPIP());

    // /* Setup the DNS server redirecting all the domains to the apIP */
    // this->_dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    // this->_dnsServer->start(WEBCONF_DNS_PORT, "*", WiFi.softAPIP());
}

void WebConf::stopAp()
{
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
}

////////////////////////////////////////////////////////////////////

void WebConf::blink(unsigned long repeatMs, byte dutyCyclePercent)
{
    if (repeatMs == 0)
    {
        this->_blinkOnMs = this->_internalBlinkOnMs;
        this->_blinkOffMs = this->_internalBlinkOffMs;
    }
    else
    {
        this->_blinkOnMs = repeatMs * dutyCyclePercent / 100;
        this->_blinkOffMs = repeatMs * (100 - dutyCyclePercent) / 100;
    }
}

void WebConf::blinkInternal(unsigned long repeatMs, byte dutyCyclePercent)
{
    this->blink(repeatMs, dutyCyclePercent);
    this->_internalBlinkOnMs = this->_blinkOnMs;
    this->_internalBlinkOffMs = this->_blinkOffMs;
}

void WebConf::doBlink()
{
    if (WEBCONF_STATUS_ENABLED)
    {
        unsigned long now = millis();
        unsigned long delayMs =
            this->_blinkState == LOW ? this->_blinkOnMs : this->_blinkOffMs;
        if (smallerCheckOverflow(this->_lastBlinkTime, delayMs, now))
        {
            this->_blinkState = 1 - this->_blinkState;
            this->_lastBlinkTime = now;
            digitalWrite(this->_statusPin, this->_blinkState);
        }
    }
}

byte WebConf::getState()
{
    return this->_state;
}

String WebConf::getWifiSSID()
{
	String wifiSSID = String(this->_wifiSsid);
	return wifiSSID;
}

String WebConf::getWifiPassword()
{
	return String(this->_wifiPassword);
}

bool WebConf::getDHCPMode()
{
	if(strcmp("True", this->_dhcpAuto) == 0) return true;
	else return false;		
}

IPAddress WebConf::getStaticIP()
{
	return stringToIp(this->_ip);
}

IPAddress WebConf::getNetMask()
{
	return stringToIp(this->_netmask);
}

IPAddress WebConf::getGateWay()
{
	return stringToIp(this->_gw);
}

String WebConf::getBrokerIp()
{
	return String(this->_brokerIp);
}

unsigned long WebConf::getBrokerPort()
{
	return this->_brokerPort;
}

SerialConfig WebConf::getSerialConfig()
{
	return this->_SerialConfig;
}

unsigned long WebConf::getSerialSpeed()
{
	return this->_SerialBaudRate;
}

bool WebConf::isAPMode()
{
	return this->_state == WEBCONF_STATE_AP_MODE;
}

bool WebConf::isSTAMode()
{
	return this->_state == WEBCONF_STATE_OFFLINE;
}

RouteInfo WebConf::getRouteInfo(){
	return this->bestRoute;
}

void WebConf::setRouteInfo(RouteInfo routeInfo){
	this->bestRoute = routeInfo;
}

bool WebConf::getAuotoScanMode(){
	return this->autoScan;
}

void configSaved()
{
#ifdef WEBCONF_DEBUG_TO_SERIAL
    Serial.println("Configuration was updated, Rebooting after 1 second.");
#endif
    //webConf.delay(1000);
    ESP.restart();
}

/**
 * Handle web requests to "/" path.
 */
void WebConf::handleRoot()
{
    //Let WebConf test and handle captive portal requests.
    // if (this->handleCaptivePortal())
    // {
    //     //Captive portal request were already served.
    //     return;
    // }
    // //Authenticate
    // if (!this->_server->authenticate(WEBCONF_ADMIN_USER_NAME, this->_apPassword))
    // {
    //     WEBCONF_DEBUG_LINE(F("Requesting authentication."));
    //     this->_server->requestAuthentication();
    //     return;
    // }

    String s = F("<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>");
    s += FPSTR(WEBCONF_HTTP_STYLE);
    s += "<title>M-Box-WebConf</title></head><div style='display:inline-block;min-width:260px;'>";
    s += "<div>";
    s += webConf.getThingName();
    s += "</div><div>";
    s += toStringIp(WiFi.softAPIP());//localIP display soft ap ip
    s += "</div><div>";
    // s += "<button type='button' onclick=\"location.href='?restart=true'\" >Restart</button>";
    s += "<button type='button' onclick=\"location.href='" + String("http://") + toStringIp(this->_server->client().localIP()) + "/?restart=true'\" >Restart</button>";
    s += "</div>";
    s += "<div>Go to <a href='config'>configure page</a> to change values.</div>";
    s += "</body></html>\n";

    if (this->_server->hasArg("restart"))
    {
        String action = this->_server->arg("restart");
        if (action.equals("true"))
        {
            this->_server->sendHeader("Location", String("http://") + toStringIp(this->_server->client().localIP()), true);
            this->_server->send(302, "text/plain", "");
            this->_server->client().stop();
#ifdef WEBCONF_DEBUG_TO_SERIAL
            Serial.println("Configuration was updated, Rebooting after 1 second.");
            Serial.flush();
#endif
            ESP.restart();
        }
    }
    else
    {
        webServer.send(200, "text/html", s);
    }
}

void WebConf::findBestRoute(){
	list<RouteInfo> routeA, routeB, routeC, routeD;

	bestRoute.rssi = -100000;
	bestRoute.status = autoScan;
	int n = WiFi.scanNetworks();
	if(n == 0){
		MBOX_DEBUG_PRINTLN("no networks found");
	}else{
		MBOX_DEBUG_PRINT(n);
		MBOX_DEBUG_PRINTLN(" network(s) found.");
		for(int i =0; i < n; i++){
			Serial1.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSIDstr(i).c_str());
			if(WiFi.SSID(i) == WIFI_SSID_A){
				RouteInfo tmpRoute;
				tmpRoute.ssid = WIFI_SSID_A;
				tmpRoute.password = WIFI_PASSWORD_A;
				tmpRoute.bssid = WiFi.BSSIDstr(i);
				tmpRoute.channel = WiFi.channel(i);
				tmpRoute.rssi = WiFi.RSSI(i);
				routeA.push_back(tmpRoute);
			}else if(WiFi.SSID(i) == WIFI_SSID_B){
				RouteInfo tmpRoute;
				tmpRoute.ssid = WIFI_SSID_B;
				tmpRoute.password = WIFI_PASSWORD_B;
				tmpRoute.bssid = WiFi.BSSIDstr(i);
				tmpRoute.channel = WiFi.channel(i);
				tmpRoute.rssi = WiFi.RSSI(i);
				routeB.push_back(tmpRoute);
			}else if(WiFi.SSID(i) == WIFI_SSID_C){
				RouteInfo tmpRoute;
				tmpRoute.ssid = WIFI_SSID_C;
				tmpRoute.password = WIFI_PASSWORD_C;
				tmpRoute.bssid = WiFi.BSSIDstr(i);
				tmpRoute.channel = WiFi.channel(i);
				tmpRoute.rssi = WiFi.RSSI(i);
				routeC.push_back(tmpRoute);
			} else if(WiFi.SSID(i) == WIFI_SSID_D){
				RouteInfo tmpRoute;
				tmpRoute.ssid = WIFI_SSID_D;
				tmpRoute.password = WIFI_PASSWORD_D;
				tmpRoute.bssid = WiFi.BSSIDstr(i);
				tmpRoute.channel = WiFi.channel(i);
				tmpRoute.rssi = WiFi.RSSI(i);
				routeD.push_back(tmpRoute);
			}
		}
	}

	for(int i = 0; i < routeA.size(); i++){
		auto it_i = std::next(routeA.begin(),i);
		if(bestRoute.rssi < it_i->rssi){
			bestRoute.ssid = it_i->ssid;
			bestRoute.bssid = it_i->bssid;
			bestRoute.rssi = it_i->rssi;
			bestRoute.channel = it_i->channel;
			bestRoute.password = it_i->password;
		}
	}

	for(int i = 0; i < routeB.size(); i++){
		auto it_i = std::next(routeB.begin(),i);
		if(bestRoute.rssi < it_i->rssi){
			bestRoute.ssid = it_i->ssid;
			bestRoute.bssid = it_i->bssid;
			bestRoute.rssi = it_i->rssi;
			bestRoute.channel = it_i->channel;
			bestRoute.password = it_i->password;
		}
	}

	for(int i = 0; i < routeC.size(); i++){
		auto it_i = std::next(routeC.begin(),i);
		if(bestRoute.rssi < it_i->rssi){
			bestRoute.ssid = it_i->ssid;
			bestRoute.bssid = it_i->bssid;
			bestRoute.rssi = it_i->rssi;
			bestRoute.channel = it_i->channel;
			bestRoute.password = it_i->password;
		}
	}

	for(int i = 0; i < routeD.size(); i++){
		auto it_i = std::next(routeD.begin(),i);
		if(bestRoute.rssi < it_i->rssi){
			bestRoute.ssid = it_i->ssid;
			bestRoute.bssid = it_i->bssid;
			bestRoute.rssi = it_i->rssi;
			bestRoute.channel = it_i->channel;
			bestRoute.password = it_i->password;
		}
	}
}


WebConf webConf;
