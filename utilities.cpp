#include "utilities.h"

QString Utilities::getDataPath()
{
    QString userMoviePath = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0];
    QDir movieDir(userMoviePath);

    movieDir.mkpath("Gazer");

    return movieDir.absoluteFilePath("Gazer");
}

QString Utilities::newSavedVideoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getSavedVideoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}

void Utilities::notifyMobile(int cameraID)
{
    // CHANGE endpoint TO YOURS HERE:
    QString endpoint = "https://maker.ifttt.com/trigger/Motion-Detected-by-Gazer/with/key/cEy-Jf4lzQ0EbDwnGeqxQR";
    QNetworkRequest request = QNetworkRequest(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json.insert("value1", QString("%1").arg(cameraID));
    json.insert("value2", QHostInfo::localHostName());
    QNetworkAccessManager nam;
    QNetworkReply *rep = nam.post(request, QJsonDocument(json).toJson());
    while(!rep->isFinished()) {
        QApplication::processEvents();
    }
    rep->deleteLater();
}
