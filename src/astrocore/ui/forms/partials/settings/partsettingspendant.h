// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTSETTINGSPENDANT_H
#define PARTSETTINGSPENDANT_H

#include <QWidget>

namespace Ui {
class partSettingsPendant;
}

class PartSettingsPendant : public QWidget
{
        Q_OBJECT

    public:
        explicit PartSettingsPendant(QWidget *parent = nullptr);
        ~PartSettingsPendant();

        void setWifiSsid(const QString &ssid);
        QString wifiSsid() const;

        void setWifiPassword(const QString &password);
        QString wifiPassword() const;

        void setHostIp(const QString &ip);
        QString hostIp() const;

        void setPort(int port);
        int port() const;

        void setEnabled(bool enabled);
        bool enabled() const;

    private slots:
        void onAutoSelectClicked();

    private:
        Ui::partSettingsPendant *ui;
};

#endif // PARTSETTINGSPENDANT_H
