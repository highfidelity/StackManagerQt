//
//  Downloader.h
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/09/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QUrl url, QObject* parent = 0);

    QUrl& getUrl() { return _url; }

    void start(QNetworkAccessManager* manager);

private slots:
    void error(QNetworkReply::NetworkError error);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();

signals:
    void downloadStarted(Downloader* downloader, QUrl url);
    void downloadCompleted(QUrl url);
    void downloadProgress(QUrl url, int percentage);
    void downloadFailed(QUrl url);
    void installingFiles(QUrl url);
    void filesSuccessfullyInstalled(QUrl url);
    void filesInstallationFailed(QUrl url);

private:
    QUrl _url;
};

#endif
