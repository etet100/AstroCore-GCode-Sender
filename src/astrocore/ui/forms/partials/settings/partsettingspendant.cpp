// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partsettingspendant.h"
#include "ui_partsettingspendant.h"
#include <QNetworkInterface>
#include <QHostAddress>

PartSettingsPendant::PartSettingsPendant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsPendant)
{
    ui->setupUi(this);

    connect(ui->btnAutoSelect, &QPushButton::clicked, this, &PartSettingsPendant::onAutoSelectClicked);
}

PartSettingsPendant::~PartSettingsPendant()
{
    delete ui;
}

void PartSettingsPendant::setWifiSsid(const QString &ssid)
{
    ui->txtWifiSsid->setText(ssid);
}

QString PartSettingsPendant::wifiSsid() const
{
    return ui->txtWifiSsid->text();
}

void PartSettingsPendant::setWifiPassword(const QString &password)
{
    ui->txtWifiPassword->setText(password);
}

QString PartSettingsPendant::wifiPassword() const
{
    return ui->txtWifiPassword->text();
}

void PartSettingsPendant::setHostIp(const QString &ip)
{
    ui->txtHostIp->setText(ip);
}

QString PartSettingsPendant::hostIp() const
{
    return ui->txtHostIp->text();
}

void PartSettingsPendant::setPort(int port)
{
    ui->txtPort->setText(QString::number(port));
}

int PartSettingsPendant::port() const
{
    return ui->txtPort->text().toInt();
}

void PartSettingsPendant::setEnabled(bool enabled)
{
    return ui->chkEnabled->setChecked(enabled);
}

bool PartSettingsPendant::enabled() const
{
    return ui->chkEnabled->checked();
}

void PartSettingsPendant::onAutoSelectClicked()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();

    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            !address.isLoopback() &&
            !address.isNull()) {
            ui->txtHostIp->setText(address.toString());
            return;
        }
    }
}
