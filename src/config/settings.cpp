#include "settings.h"

#include <QSettings>
#include <QFile>
#include <QFileDevice>
#include <QDir>

namespace {
    const char* ORGANIZATION = "remarkable-todoist";
    const char* APPLICATION = "config";
    const char* API_TOKEN_KEY = "auth/api_token";

    QSettings createSettings()
    {
        return QSettings(QSettings::IniFormat, QSettings::UserScope,
                        ORGANIZATION, APPLICATION);
    }
}

QString AppSettings::getApiToken()
{
    QSettings settings = createSettings();
    return settings.value(API_TOKEN_KEY).toString();
}

void AppSettings::setApiToken(const QString& token)
{
    QSettings settings = createSettings();
    settings.setValue(API_TOKEN_KEY, token);
    settings.sync();  // Force write to disk

    // Set restrictive permissions (owner read/write only = 0600)
    QString filePath = settings.fileName();
    QFile file(filePath);
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

bool AppSettings::hasApiToken()
{
    return !getApiToken().isEmpty();
}

QString AppSettings::configFilePath()
{
    QSettings settings = createSettings();
    return settings.fileName();
}
