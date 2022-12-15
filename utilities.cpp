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
