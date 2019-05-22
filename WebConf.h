#ifndef WebConf_h
#define WebConf_h

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include "common.h"


//Logs progress information to Serial if enabled.
//#define WEBCONF_DEBUG_TO_SERIAL

//Helper define for serial debug
#ifdef WEBCONF_DEBUG_TO_SERIAL
#define WEBCONF_DEBUG_LINE(MSG) Serial.println(MSG)
#else
#define WEBCONF_DEBUG_LINE(MSG)
#endif

//Initial AP password
#define DEFAULT_BROKER_IP ("mcloud.kongtrolink.com")

#define DEFAULT_BROKER_PORT "1883"

//Initial AP SSID
#define HOSTNAME "M-Box"

//Initial AP password
#define INIT_AP_PASSWORD "123456789"

//Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "M-Box"

//Maximal length of any string used in WebConfig configuration (e.g. ssid, password).
#define WEBCONF_WORD_LEN 33

//WebConf tries to connect to the local network for an amount of time before falling back to AP mode.
#define WEBCONF_DEFAULT_WIFI_CONNECTION_TIMEOUT_MS 30000

//Thing will stay in AP mode for an amount of time on boot, before retrying to connect to a WiFi network.
#define WEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS 15000
//#define WEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS 30000

//mDNS should allow you to connect to this device with a hostname provided by the device. E.g. mything.local
// #define WEBCONF_CONFIG_USE_MDNS
#ifdef WEBCONF_CONFIG_USE_MDNS
#include <ESP8266mDNS.h>
#endif

//EEPROM config starts with a special prefix of length defined here.
#define WEBCONF_CONFIG_VESION_LENGTH 12
// #define WEBCONF_DNS_PORT 53

//State of the Thing
#define WEBCONF_STATE_BOOT 0
#define WEBCONF_STATE_NOT_CONFIGURED 1
#define WEBCONF_STATE_AP_MODE 2
#define WEBCONF_STATE_CONNECTING 3
#define WEBCONF_STATE_ONLINE 4

#define WEBCONF_STATE_OFFLINE 5


//AP connection state
//No connection on AP.
#define WEBCONF_AP_CONNECTION_STATE_NC 0
//Has connection on AP.
#define WEBCONF_AP_CONNECTION_STATE_C 1
//All previous connection on AP was disconnected.
#define WEBCONF_AP_CONNECTION_STATE_DC 2
//AP was closed.
#define WEBCONF_AP_CONNECTION_STATE_CO 3

//Status indicator output logical levels.
#define WEBCONF_STATUS_ON LOW
#define WEBCONF_STATUS_OFF HIGH

//Init Status
#define WEBCONF_INIT_OK true
#define WEBCONF_INIT_FAIL false

/**
 *   WebConfParameters is a configuration item of the config portal.
 *   The parameter will have its input field on the configuration page,
 *   and the provided value will be saved to the EEPROM.
 */
class WebConfParameter
{
  public:
    /**
     * Create a parameter for the config portal.
     * 
     *   @label - Displayable label at the config portal.
     *   @id - Identifier used for HTTP queries and as configuration key. Must not contain spaces nor other special characters.
     *   @valueBuffer - Configuration value will be loaded to this buffer from the EEPROM.
     *   @length - The buffer should have a length provided here.
     *   @type (optional, default="text") - The type of the html input field.
     *       The type="password" has a special handling, as the value will be overwritten in the EEPROM
     *       only if value was provided on the config portal. Because of this logic, "password" type field with
     *       length more then WEBCONF_WORD_LEN characters are not supported.
     *   @placeholder (optional) - Text appear in an empty input box.
     *   @defaultValue (optional) - Value should be pre-filled if none was specified before.
     *   @customHtml (optional) - The text of this parameter will be added into the HTML INPUT field.
     */
    WebConfParameter(
        const char *label,
        const char *id,
        char *valueBuffer,
        int length,
        const char *type = "text",
        const char *placeholder = NULL,
        const char *defaultValue = NULL,
        const char *customHtml = NULL);

    /**
     * Same as normal constructor, but config portal does not render a default input field
     * for the item, instead uses the customHtml provided.
     * Note the @type parameter description above!
     */
    WebConfParameter(
        const char *id,
        char *valueBuffer,
        int length,
        const char *customHtml,
        const char *type = "text");

    /**
     * For internal use only.
     */
    WebConfParameter();

    const char *label;
    const char *id = 0;
    char *valueBuffer;
    int length;
    const char *type;
    const char *placeholder;
    const char *customHtml;
    const char *errorMessage;

    //For internal use only
    WebConfParameter *_nextParameter = NULL;
};

/**
 * A separator for separating field sets.
 */
class WebConfSeparator : public WebConfParameter
{
  public:
    WebConfSeparator();
};

/**
 * Main class of the module.
 */
class WebConf
{
  public:
    /**
     * Create a new configuration handler.
     *   @thingName - Initial value for the thing name. Used in many places like AP name, can be changed by the user.
     *   @initialApPassword - Initial value for AP mode. Can be changed by the user.
     *   @configVersion - When the software is updated and the configuration is changing, this key should also be changed,
     *     so that the config portal will force the user to reenter all the configuration values.
     */
    WebConf(
        const char *thingName = HOSTNAME,
        const char *initialApPassword = INIT_AP_PASSWORD,
        const char *configVersion = CONFIG_VERSION);

    /**
     * Provide an Arduino pin here, that has a button connected to it with the other end of the pin is connected to GND.
     * The button pin is queried at for input on boot time (init time).
     * If the button was pressed, the thing will enter AP mode with the initial password.
     * Must be called before init()!
     *   @configPin - An Arduino pin. Will be configured as INPUT_PULLUP!
     */
    void setConfigPin(int configPin);

    /**
     * Provide an Arduino pin for status indicator (LOW = on). Blink codes:
     *   - Rapid blinks - The thing is in AP mode with default password.
     *   - Rapid blinks, but mostly on - AP mode, waiting for configuration changes.
     *   - Normal blinks - Connecting to WiFi.
     *   - Mostly off with rare rapid blinks - WiFi is connected performing normal operation.
     * User can also apply custom blinks. See blink() method!
     * Must be called before init()!
     *   @statusPin - An Arduino pin. Will be configured as OUTPUT!
     */
    void setStatusPin(int statusPin);

    /**
     * Add an UpdateServer instance to the system. The firmware update link will appear on the config portal.
     * The UpdateServer will be added to the WebServer with the path provided here (or with "firmware",
     * if none was provided).
     * Login user will be WEBCONF_ADMIN_USER_NAME, password is the password provided in the config portal.
     * Should be called before init()!
     *   @updateServer - An uninitialized UpdateServer instance.
     *   @updatePath - (Optional) The path to set up the UpdateServer with. Will be also used in the config portal.
     */
    void setupUpdateServer(
        ESP8266HTTPUpdateServer *updateServer,
        const char *updatePath = "/firmware");

    /**
     * Start up the WebConf module.
     * Loads all configuration from the EEPROM, and initialize the system.
     * Will return false, if no configuration (with specified config version) was found in the EEPROM.
     */
    boolean init();

    /**
     * WebConf is a non-blocking, state controlled system. Therefor it should be
     * regularly triggered from the user code.
     * So call this method any time you can.
     */
    void doLoop();

    /**
     * Each WebServer URL handler method should start with calling this method.
     * If this method return true, the request was already served by it.
     */
    boolean handleCaptivePortal();

    /**
     * Root URL web request handler. Call this method to handle root request.
     */
    void handleRoot();

    /**
     * Config URL web request handler. Call this method to handle config request.
     */
    void handleConfig();

    /**
     * URL-not-found web request handler. Used for handling captive portal request.
     */
    void handleNotFound();

    /**
     * Specify a callback method, that will be called upon WiFi connection success.
     * Should be called before init()!
     */
    void setWifiConnectionCallback(void (*func)(void));

    /**
     * Specify a callback method, that will be called when settings have been changed.
     * Should be called before init()!
     */
    void setConfigSavedCallback(void (*func)(void));

    /**
     * Specify a callback method, that will be called when form validation is required.
     * If the method will return false, the configuration will not be saved.
     * Should be called before init()!
     */
    void setFormValidator(boolean (*func)(void));

    /**
     * Add a custom parameter, that will be handled by the WebConf module.
     * The parameter will be saved to/loaded from EEPROM automatically, 
     * and will appear on the config portal.
     * Will return false, if adding was not successful.
     * Must be called before init()!
     */
    bool addParameter(WebConfParameter *parameter);

    /**
     * Getter for the actually configured thing name.
     */
    char *getThingName();

    /**
     * WebConf tries to connect to the local network for an amount of time before falling back to AP mode.
     * The default amount can be updated with this setter.
     * Should be called before init()!
     */
    void setWifiConnectionTimeoutMs(unsigned long millis);

    /**
     * Interrupts internal blinking cycle and applies new values for
     * blinking the status LED (if one configured with setStatusPin() prior init() ).
     *   @repeatMs - Defines the the period of one on-off cycle in milliseconds.
     *   @dutyCyclePercent - LED on/off percent. 100 means always on, 0 means always off.
     * When called with repeatMs = 0, then internal blink cycle will be continued.
     */
    void blink(unsigned long repeatMs, byte dutyCyclePercent);

    /**
     * Helper method to check time elapse while checking number overflow.
     * Will return true while the sum of the two numbers are smaller than the third one.
     */
    static boolean smallerCheckOverflow(unsigned long prevMillis, unsigned long diff, unsigned long currentMillis);
    byte getState();
	void findBestRoute();

    unsigned long _SerialBaudRate;

    /**
     * Support 5, 6, 7, 8 data bits, odd (O), even (E), and no (N) parity, and 1 or 2 stop bits.
     */
    SerialConfig _SerialConfig;

	IPAddress getStaticIP();
	IPAddress getNetMask();
	IPAddress getGateWay();
	String getWifiSSID();
	String getWifiPassword();
	String getBrokerIp();
	unsigned long getBrokerPort();
	unsigned long getSerialSpeed();
	SerialConfig getSerialConfig();
	bool getDHCPMode();
	bool isAPMode();
	bool isSTAMode();
	void setRouteInfo(RouteInfo routeInfo);
	RouteInfo getRouteInfo();
	bool getAuotoScanMode();

  private:
    const char *_initialApPassword = NULL;
    char _configVersion[WEBCONF_CONFIG_VESION_LENGTH];
    // DNSServer *_dnsServer;
    ESP8266WebServer *_server;
    ESP8266HTTPUpdateServer *_updateServer = NULL;
    int _configPin = -1;
    int _statusPin = -1;
    const char *_updatePath = NULL;
    boolean _forceDefaultPassword = false;
    WebConfParameter *_firstParameter = NULL;
    WebConfParameter _thingNameParameter;
    WebConfParameter _apPasswordParameter;
    WebConfParameter _wifiSsidParameter;
    WebConfParameter _wifiPasswordParameter;
    WebConfParameter _apTimeoutParameter;
    WebConfParameter _SerialBaudRateParameter;
    WebConfParameter _SerialConfigParameter;

	WebConfParameter _scanAutoParameter;
    WebConfParameter _dhcpAutoParameter;
    WebConfParameter _ipParameter;
    WebConfParameter _netmaskParameter;
    WebConfParameter _gwParameter;

	WebConfParameter _brokerIpParameter;
    WebConfParameter _brokerPortParameter;
	
    char _thingName[WEBCONF_WORD_LEN];
    char _apPassword[WEBCONF_WORD_LEN];
    char _wifiSsid[WEBCONF_WORD_LEN];
    char _wifiPassword[WEBCONF_WORD_LEN];
    char _apTimeoutStr[WEBCONF_WORD_LEN]; 
    char _SerialBaudRateStr[WEBCONF_WORD_LEN];
    char _SerialConfigStr[WEBCONF_WORD_LEN];

	char _scanAuto[WEBCONF_WORD_LEN];
    char _dhcpAuto[WEBCONF_WORD_LEN];
    char _ip[WEBCONF_WORD_LEN];
    char _gw[WEBCONF_WORD_LEN];
    char _netmask[WEBCONF_WORD_LEN]; 

	char _brokerIp[WEBCONF_WORD_LEN];
    char _brokerPortStr[WEBCONF_WORD_LEN];
	unsigned long _brokerPort = 1883;

    unsigned long _apTimeoutMs = WEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS;
    unsigned long _wifiConnectionTimeoutMs = WEBCONF_DEFAULT_WIFI_CONNECTION_TIMEOUT_MS;
    byte _state = WEBCONF_STATE_BOOT;
    unsigned long _apStartTimeMs = 0;
	// ap 模式连接状况
    byte _apConnectionStatus = WEBCONF_AP_CONNECTION_STATE_NC;
    void (*_wifiConnectionCallback)(void) = NULL;
    void (*_configSavedCallback)(void) = NULL;
    boolean (*_formValidator)(void) = NULL;
    unsigned long _internalBlinkOnMs = 500;
    unsigned long _internalBlinkOffMs = 500;
    unsigned long _blinkOnMs = 500;
    unsigned long _blinkOffMs = 500;
    byte _blinkState = WEBCONF_STATUS_ON;
    unsigned long _lastBlinkTime = 0;
    unsigned long _wifiConnectionStart = 0;
	bool autoScan = false;

	RouteInfo bestRoute;

    void configInit();
    boolean configLoad();
    void configSave();
    boolean configTestVersion();
    void configSaveConfigVersion();
    void readEepromValue(int start, char *valueBuffer, int length);
    void writeEepromValue(int start, char *valueBuffer, int length);

    void readParamValue(const char *paramName, char *target, unsigned int len);
    boolean validateForm();

    void changeState(byte newState);
    void stateChanged(byte oldState, byte newState);
    boolean isIp(String str);
    String toStringIp(IPAddress ip);
    IPAddress stringToIp(const char *address);
    void doBlink();
    void blinkInternal(unsigned long repeatMs, byte dutyCyclePercent);

    void checkApTimeout();
    void checkConnection();
    boolean checkWifiConnection();
    void setupAp();
    void stopAp();
};

extern WebConf webConf;
#endif
