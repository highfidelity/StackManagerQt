//
//  DownloadManager.cpp
//  StackManagerQt/src
//
//  Created by Mohammed Nafees on 07/09/14.
//  Copyright (c) 2014 High Fidelity. All rights reserved.
//

#include "DownloadManager.h"
#include "ui_DownloadManager.h"
#include "GlobalData.h"

#include <QFileInfo>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProgressBar>
#include <QMessageBox>
#include <QApplication>

DownloadManager::DownloadManager(QNetworkAccessManager* manager, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DownloadManager),
    _manager(manager) {
    ui->setupUi(this);
    ui->table->setColumnCount(3);
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table->setHorizontalHeaderLabels(QStringList() << "Name" << "Progress" << "Status");
}

DownloadManager::~DownloadManager() {
    delete ui;
    _downloaderHash.clear();
}

void DownloadManager::downloadFile(QUrl url) {
    for (int i = 0; i < _downloaderHash.size(); ++i) {
        if (_downloaderHash.keys().at(i)->getUrl() == url) {
            qDebug() << "Downloader for URL " << url << " already initialised.";
            return;
        }
    }

    Downloader* downloader = new Downloader(url);
    connect(downloader, SIGNAL(downloadCompleted(QUrl)), SLOT(onDownloadCompleted(QUrl)));
    connect(downloader, SIGNAL(downloadStarted(Downloader*,QUrl)),
            SLOT(onDownloadStarted(Downloader*,QUrl)));
    connect(downloader, SIGNAL(downloadFailed(QUrl)), SLOT(onDownloadFailed(QUrl)));
    connect(downloader, SIGNAL(downloadProgress(QUrl,int)), SLOT(onDownloadProgress(QUrl,int)));
    connect(downloader, SIGNAL(installingFiles(QUrl)), SLOT(onInstallingFiles(QUrl)));
    connect(downloader, SIGNAL(filesSuccessfullyInstalled(QUrl)), SLOT(onFilesSuccessfullyInstalled(QUrl)));
    connect(downloader, SIGNAL(filesInstallationFailed(QUrl)), SLOT(onFilesInstallationFailed(QUrl)));
    downloader->start(_manager);
}

void DownloadManager::onDownloadStarted(Downloader* downloader, QUrl url) {
    int rowIndex = ui->table->rowCount();
    ui->table->setRowCount(rowIndex + 1);
    QTableWidgetItem* nameItem = new QTableWidgetItem(QFileInfo(url.toString()).fileName());
    ui->table->setItem(rowIndex, 0, nameItem);
    QProgressBar* progressBar = new QProgressBar;
    ui->table->setCellWidget(rowIndex, 1, progressBar);
    QTableWidgetItem* statusItem = new QTableWidgetItem;
    if (QFile(QDir::toNativeSeparators(GlobalData::getInstance()->getClientsLaunchPath() + "/" + QFileInfo(url.toString()).fileName())).exists()) {
        statusItem->setText("Updating");
    } else {
        statusItem->setText("Downloading");
    }
    ui->table->setItem(rowIndex, 2, statusItem);
    _downloaderHash.insert(downloader, rowIndex);
}

void DownloadManager::onDownloadCompleted(QUrl url) {
    ui->table->item(downloaderRowIndexForUrl(url), 2)->setText("Download Complete");
}

void DownloadManager::onDownloadProgress(QUrl url, int percentage) {
    qobject_cast<QProgressBar*>(ui->table->cellWidget(downloaderRowIndexForUrl(url), 1))->setValue(percentage);
}

void DownloadManager::onDownloadFailed(QUrl url) {
    ui->table->item(downloaderRowIndexForUrl(url), 2)->setText("Download Failed");
    _downloaderHash.remove(downloaderForUrl(url));
}

void DownloadManager::onInstallingFiles(QUrl url) {
    ui->table->item(downloaderRowIndexForUrl(url), 2)->setText("Installing");
}

void DownloadManager::onFilesSuccessfullyInstalled(QUrl url) {
    ui->table->item(downloaderRowIndexForUrl(url), 2)->setText("Successfully Installed");
    _downloaderHash.remove(downloaderForUrl(url));
    emit fileSuccessfullyInstalled(url);
    if (_downloaderHash.size() == 0) {
        close();
    }
}

void DownloadManager::onFilesInstallationFailed(QUrl url) {
    ui->table->item(downloaderRowIndexForUrl(url), 2)->setText("Installation Failed");
    _downloaderHash.remove(downloaderForUrl(url));
}

void DownloadManager::closeEvent(QCloseEvent*) {
    if (_downloaderHash.size() > 0) {
        QMessageBox msgBox;
        msgBox.setText("There are active downloads that need to be installed for the proper functioning of Stack Manager. Do you want to stop the downloads and exit?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Yes:
            qApp->quit();
            break;
        case QMessageBox::No:
            msgBox.close();
            break;
        }
    }
}

int DownloadManager::downloaderRowIndexForUrl(QUrl url) {
    QHash<Downloader*, int>::const_iterator i = _downloaderHash.constBegin();
    while (i != _downloaderHash.constEnd()) {
        if (i.key()->getUrl() == url) {
            return i.value();
        } else {
            ++i;
        }
    }

    return -1;
}

Downloader* DownloadManager::downloaderForUrl(QUrl url) {
    QHash<Downloader*, int>::const_iterator i = _downloaderHash.constBegin();
    while (i != _downloaderHash.constEnd()) {
        if (i.key()->getUrl() == url) {
            return i.key();
        } else {
            ++i;
        }
    }

    return NULL;
}
