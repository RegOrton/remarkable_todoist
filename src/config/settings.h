#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

/**
 * @brief AppSettings provides static methods for configuration storage.
 *
 * Uses QSettings with IniFormat to store configuration at:
 * ~/.config/remarkable-todoist/config.ini
 *
 * API token is stored with restrictive file permissions (0600).
 */
class AppSettings
{
public:
    /**
     * @brief Get the stored API token
     * @return The API token, or empty string if not set
     */
    static QString getApiToken();

    /**
     * @brief Set the API token
     * @param token The Todoist API token to store
     *
     * This method also sets file permissions to 0600 (owner read/write only)
     * to protect the token from other users.
     */
    static void setApiToken(const QString& token);

    /**
     * @brief Check if an API token is configured
     * @return true if a non-empty API token exists
     */
    static bool hasApiToken();

    /**
     * @brief Get the path to the configuration file
     * @return The full path to the config file (for debugging/user info)
     */
    static QString configFilePath();

private:
    // No instances needed - all methods are static
    AppSettings() = delete;
};

#endif // SETTINGS_H
