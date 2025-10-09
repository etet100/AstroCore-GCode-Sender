// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "camera.h"
#include "ui_camera.h"
#include <QMediaDevices>
#include <QMessageBox>
#include <QtWidgets>
#include <QImageCapture>

Camera::Camera(QWidget* parent) : QVideoWidget(parent), m_frameProcessor(this)
{
    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);

    connect(videoDevicesGroup, &QActionGroup::triggered, this, &Camera::updateCameraDevice);

    // popup menu to select camera
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, [this](const QPoint &pos){
        m_menu.exec(this->mapToGlobal(pos));
    });

    m_resizeTimer.setInterval(500);
    m_resizeTimer.setSingleShot(true);
    connect(&m_resizeTimer, &QTimer::timeout, this, [this](){
        findBestResolution(this->width(), this->height());
    });

    init();
}

Camera::~Camera()
{
    m_resizeTimer.stop();
}

void Camera::setCamera(const QCameraDevice &cameraDevice)
{
    if (!m_camera.isNull()) {
        m_camera->stop();
    }
    m_captureSession.setCamera(nullptr);

    qDebug() << "Camera device:" << cameraDevice.description();

    m_camera.reset(new QCamera(cameraDevice));
    m_captureSession.setCamera(m_camera.data());
    m_captureSession.setVideoSink(&m_videoSink);
    m_frameProcessor.setVideoSinks(
        m_captureSession.videoSink(),
        this->videoSink()
    );

    if (isVisible()) {
        m_camera->start();
    }

    findBestResolution(this->width(), this->height());
    updateCameraActive(m_camera->isActive());
}

void Camera::hideEvent(QHideEvent *event)
{
    QVideoWidget::hideEvent(event);

    stopCamera();
}

void Camera::showEvent(QShowEvent *event)
{
    QVideoWidget::showEvent(event);

    startCamera();
}

void Camera::startCamera()
{
    if (m_camera.isNull() || m_camera->isActive() || !m_camera->isAvailable()) {
        return;
    }

    m_camera->start();
}

void Camera::stopCamera()
{
    if (m_camera.isNull() || !m_camera->isActive()) {
        return;
    }

    m_camera->stop();
}

void Camera::displayCameraError()
{
    if (m_camera->error() != QCamera::NoError) {
        QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
    }
}

void Camera::updateCameraDevice(QAction *action)
{
    if (action->data().isNull()) {
        disableCamera();

        return;
    }

    setCamera(qvariant_cast<QCameraDevice>(action->data()));
}

void Camera::updateCameraActive(bool active)
{
    qDebug() << active;
}

void Camera::resizeEvent(QResizeEvent *event)
{
    if (m_resizeTimer.isActive()) {
        m_resizeTimer.stop();
    }
    if (!m_camera.isNull() && m_camera->isActive()) {
        m_resizeTimer.start();
    }

    QVideoWidget::resizeEvent(event);

    if (m_camera.isNull()) {
        showCameraDisabled();
    }
}

void Camera::showCameraDisabled()
{
    m_frameProcessor.setVideoSinks(nullptr, nullptr);
    m_captureSession.setCamera(nullptr);

    QImage image(width(), height(), QImage::Format_RGB32);

    QPainter painter(&image);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 25));
    painter.fillRect(rect(), Qt::black);
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::WordWrap);
    painter.drawText(
        QRect(5, 0, width() - 10, height()),
        "No camera found",
        textOption
    );

    this->videoSink()->setVideoFrame(QVideoFrame(image));
}

void Camera::disableCamera()
{
    stopCamera();
    showCameraDisabled();
}

void Camera::init()
{
    #if QT_CONFIG(permissions)
        // camera
        QCameraPermission cameraPermission;
        switch (qApp->checkPermission(cameraPermission)) {
            case Qt::PermissionStatus::Undetermined:
                qApp->requestPermission(cameraPermission, this, &Camera::init);
                return;
            case Qt::PermissionStatus::Denied:
                qWarning("Camera permission is not granted!");
                return;
            case Qt::PermissionStatus::Granted:
                break;
        }
    #endif

    videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    updateCameras();
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &Camera::updateCameras);
    connect(videoDevicesGroup, &QActionGroup::triggered, this, &Camera::updateCameraDevice);

    if (m_camerasCount && m_camera.isNull()) {
        QCameraDevice defaultCamera = QMediaDevices::defaultVideoInput();
        if (!defaultCamera.isNull()) {
            setCamera(QMediaDevices::defaultVideoInput());
        }
    }

    m_captureSession.setVideoOutput(this);
    m_captureSession.setVideoSink(&m_videoSink);

    if (m_camera.isNull()) {
        showCameraDisabled();
    }
}

void Camera::updateCameras()
{
    m_menu.clear();
    m_camerasCount = 0;

    QAction *videoDeviceAction = new QAction("Disable", videoDevicesGroup);
    videoDeviceAction->setCheckable(true);
    videoDeviceAction->setData(QVariant());

    m_menu.addAction(videoDeviceAction);

    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        if (m_camera.isNull()) {
            setCamera(cameraDevice);
        }

        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));

        if (!m_camera.isNull() && m_camera->cameraDevice() == cameraDevice) {
            videoDeviceAction->setChecked(true);
        }

        m_menu.addAction(videoDeviceAction);
        m_camerasCount++;
    }
}

void Camera::findBestResolution(int w, int h)
{
    QCameraFormat bestFormat;
    float bestScale = 1000.0;

    for (const QCameraFormat &format : m_camera->cameraDevice().videoFormats()) {
        if (format.pixelFormat() != QVideoFrameFormat::Format_NV12 && format.pixelFormat() != QVideoFrameFormat::Format_NV21) {
            continue;
        }

        QSize resolution = format.resolution();
        float scaleX = resolution.width() / (float)w;
        float scaleY = resolution.height() / (float)h;
        float maxScale = std::max(scaleX, scaleY);

        if (maxScale >= 1 && maxScale < bestScale) {
            bestScale = maxScale;
            bestFormat = format;
        }
    }

    if (!bestFormat.isNull() && m_camera->cameraFormat() != bestFormat) {
        qDebug() << "Setting camera format to" << bestFormat.resolution()
                 << (float) bestFormat.resolution().width() / (float) bestFormat.resolution().height()
                 << bestScale;
        m_camera->setCameraFormat(bestFormat);
    }

    if (bestFormat.isNull()) {
        qDebug() << "Matching camera format not found!";
    }
}

