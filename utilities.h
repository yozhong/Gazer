#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QApplication>

class Utilities
{
public:
    static QString getDataPath();
    static QString newSavedVideoName();
    static QString getSavedVideoPath(QString name, QString postfix);
    static void notifyMobile(int cameraID);
};

#endif // UTILITIES_H
