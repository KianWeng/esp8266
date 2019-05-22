/**
 * \file config_io.h
 * \author your name
 * \brief 
 * @version 0.1
 * \date 2018-12-28
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#ifndef CONFIG_IO
#define CONFIG_IO

#include <memory>
#include "FS.h"
#include "common.h"

using namespace std;

namespace mbox_dev_p3 {

class ConfigDataAccess;
typedef std::shared_ptr<ConfigDataAccess> ConfigDataAccessPtr;

class ConfigDataAccess {
public:
    ~ConfigDataAccess();

    static ConfigDataAccessPtr Instance();

    /**
     * \brief Set configuration file path.
     * 
     * \param filePath  Configuration file path.
     */
    void SetConfigFilePath(const String &filePath);

    /**
     * \brief Check whether configuration file exists.
     * 
     * \return File existence status. 
     */
    bool ConfigFileExists();

    /**
     * \brief Get configurations from file.
     * 
     * \return Configuration content.
     */
    //String GetConfig() const;
	char* GetConfig() const;
    
    /**
     * \brief Set configurations into file.
     * 
     * \param config  Configuration content.
     * 
     * \return Set result.
     */
    //bool SetConfig(const String &config);
	bool SetConfig(const char* config);
	void ClearConfig();

private:
    ConfigDataAccess();
    ConfigDataAccess(const ConfigDataAccess &);
    ConfigDataAccess& operator=(const ConfigDataAccess &);
    
    static ConfigDataAccessPtr _instance;
    String _cfgFilePath;
};

}

#endif //CONFIG_IO
