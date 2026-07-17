#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QNetworkAccessManager>

// Fetches the list of releases from the GitHub API and compares the newest one
// against the currently running build. Supports scheduled checks (every N days)
// and on-demand checks. Runtime state (last check time, last detected build,
// skipped build) is persisted in the VEDIS-backed Cache.
//
// The project has no semantic versioning: releases are tagged
// "auto-build-<github.run_id>" and marked as prereleases. The build identity is
// therefore the GitHub Actions run id — a monotonically increasing integer. The
// running build learns its own id from the APP_BUILD_ID define injected by CI
// (0 for local/dev builds, which disables scheduled checks).
class UpdateChecker : public QObject
{
    Q_OBJECT

    public:
        struct ReleaseInfo {
            qint64 buildId = 0; // parsed from the "auto-build-<id>" tag
            QString tagName;    // raw tag, e.g. "auto-build-123456789"
            QString name;       // release title, e.g. "Automatic build 123456789"
            QString url;        // html_url of the release page
            QString notes;      // release body / changelog
            bool prerelease = false;
        };

        static UpdateChecker& instance();

        // Runs a check only if scheduled checks are enabled, the running build
        // has a known id, and the configured interval has elapsed since the
        // last check. Intended for startup.
        void checkIfDue();

        // Runs a check now. When userInitiated is true the check runs even if
        // scheduled checks are disabled, and noUpdateAvailable() is emitted
        // when the running build is up to date (so the UI can inform the user).
        // Skipped builds are ignored only for non-user-initiated checks.
        void checkNow(bool userInitiated = true);

        // Marks a build as skipped — scheduled checks will no longer report it
        // as available. An explicit user-initiated check still reports it.
        void skipBuild(qint64 buildId);

        // Build id of the running application, or 0 when unknown (local build).
        qint64 currentBuildId() const;
        qint64 lastDetectedBuildId() const;
        qint64 skippedBuildId() const;
        QDateTime lastCheckTime() const;

        // Parses "auto-build-<id>" into the numeric id, or 0 if it does not match.
        static qint64 parseBuildId(const QString& tag);

    signals:
        void updateAvailable(const UpdateChecker::ReleaseInfo& release, bool userInitiated);
        void noUpdateAvailable(bool userInitiated);
        void checkFailed(const QString& error, bool userInitiated);

    private:
        explicit UpdateChecker(QObject *parent = nullptr);

        void sendRequest(bool userInitiated);
        void handleResponse(const QByteArray& data, bool userInitiated);
        void storeState(const ReleaseInfo& release);

        QNetworkAccessManager* m_networkManager;
        bool m_inProgress = false;
};

#endif // UPDATECHECKER_H
