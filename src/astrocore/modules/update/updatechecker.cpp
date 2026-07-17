#include "updatechecker.h"
#include "configurationupdate.h"
#include "utils/cache.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// GitHub repository the releases are published under. Change these if the
// release repository ever moves.
static const QString GITHUB_OWNER = "etet100";
static const QString GITHUB_REPO = "Candle";

// Releases are tagged "auto-build-<github.run_id>".
static const QString BUILD_TAG_PREFIX = "auto-build-";

// VEDIS cache keys for the persisted runtime state.
static const QString KEY_LAST_CHECK = "update.lastCheck";
static const QString KEY_LAST_BUILD = "update.lastBuild";
static const QString KEY_SKIPPED_BUILD = "update.skippedBuild";

static const int SECONDS_PER_DAY = 86400;

UpdateChecker& UpdateChecker::instance()
{
    static UpdateChecker inst;

    return inst;
}

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

qint64 UpdateChecker::currentBuildId() const
{
#ifdef APP_BUILD_ID
    return APP_BUILD_ID;
#else
    return 0;
#endif
}

qint64 UpdateChecker::lastDetectedBuildId() const
{
    return Cache::instance().get(KEY_LAST_BUILD).toLongLong();
}

qint64 UpdateChecker::skippedBuildId() const
{
    return Cache::instance().get(KEY_SKIPPED_BUILD).toLongLong();
}

QDateTime UpdateChecker::lastCheckTime() const
{
    QString value = Cache::instance().get(KEY_LAST_CHECK);
    if (value.isEmpty()) {
        return {};
    }

    return QDateTime::fromSecsSinceEpoch(value.toLongLong());
}

void UpdateChecker::skipBuild(qint64 buildId)
{
    Cache::instance().set(KEY_SKIPPED_BUILD, QString::number(buildId));
    Cache::instance().flush();
}

void UpdateChecker::checkIfDue()
{
    if (!ConfigurationUpdate::instance().checkForUpdates()) {
        return;
    }

    // Local/dev builds have no id to compare against — skip scheduled checks.
    if (currentBuildId() == 0) {
        return;
    }

    QDateTime last = lastCheckTime();
    if (last.isValid()) {
        int intervalDays = ConfigurationUpdate::instance().checkIntervalDays();
        qint64 elapsed = QDateTime::currentSecsSinceEpoch() - last.toSecsSinceEpoch();
        if (elapsed < static_cast<qint64>(intervalDays) * SECONDS_PER_DAY) {
            return;
        }
    }

    checkNow(false);
}

void UpdateChecker::checkNow(bool userInitiated)
{
    if (m_inProgress) {
        return;
    }

    m_inProgress = true;
    sendRequest(userInitiated);
}

void UpdateChecker::sendRequest(bool userInitiated)
{
    // /releases/latest skips prereleases, so it cannot be used — every build is
    // published as a prerelease. Fetch the list and pick the newest instead.
    QUrl url(QString("https://api.github.com/repos/%1/%2/releases?per_page=10")
        .arg(GITHUB_OWNER, GITHUB_REPO));

    QNetworkRequest request(url);
    // GitHub rejects requests without a User-Agent header.
    request.setHeader(QNetworkRequest::UserAgentHeader, "AstroCore");
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, userInitiated]() {
        m_inProgress = false;

        if (reply->error() != QNetworkReply::NoError) {
            QString error = QString("Update check failed: %1").arg(reply->errorString());
            qWarning() << "[Update]" << error;
            emit checkFailed(error, userInitiated);
        } else {
            handleResponse(reply->readAll(), userInitiated);
        }

        reply->deleteLater();
    });
}

void UpdateChecker::handleResponse(const QByteArray& data, bool userInitiated)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        emit checkFailed("Update check failed: invalid response", userInitiated);

        return;
    }

    // Pick the release with the highest build id, ignoring drafts and any tag
    // that does not follow the "auto-build-<id>" scheme.
    ReleaseInfo newest;
    const QJsonArray releases = doc.array();
    for (const QJsonValue& value : releases) {
        QJsonObject obj = value.toObject();
        if (obj["draft"].toBool()) {
            continue;
        }

        qint64 buildId = parseBuildId(obj["tag_name"].toString());
        if (buildId <= newest.buildId) {
            continue;
        }

        newest.buildId = buildId;
        newest.tagName = obj["tag_name"].toString();
        newest.name = obj["name"].toString();
        newest.url = obj["html_url"].toString();
        newest.notes = obj["body"].toString();
        newest.prerelease = obj["prerelease"].toBool();
    }

    if (newest.buildId == 0) {
        emit checkFailed("Update check failed: no release found", userInitiated);

        return;
    }

    storeState(newest);

    if (newest.buildId <= currentBuildId()) {
        emit noUpdateAvailable(userInitiated);

        return;
    }

    // Honor "skip this build" only for automatic checks; a user-initiated check
    // always reports the update.
    if (!userInitiated && newest.buildId == skippedBuildId()) {
        emit noUpdateAvailable(userInitiated);

        return;
    }

    emit updateAvailable(newest, userInitiated);
}

void UpdateChecker::storeState(const ReleaseInfo& release)
{
    Cache& cache = Cache::instance();
    cache.set(KEY_LAST_CHECK, QString::number(QDateTime::currentSecsSinceEpoch()));
    cache.set(KEY_LAST_BUILD, QString::number(release.buildId));
    cache.flush();
}

qint64 UpdateChecker::parseBuildId(const QString& tag)
{
    if (!tag.startsWith(BUILD_TAG_PREFIX)) {
        return 0;
    }

    bool ok = false;
    qint64 id = tag.mid(BUILD_TAG_PREFIX.length()).toLongLong(&ok);

    return ok ? id : 0;
}
