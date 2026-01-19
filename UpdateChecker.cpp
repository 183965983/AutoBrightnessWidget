#include "UpdateChecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <algorithm>

UpdateChecker::UpdateChecker(const QString& currentVersion,
                           const QString& owner,
                           const QString& repo,
                           QObject* parent)
    : QObject(parent)
    , m_currentVersion(normalizeVersion(currentVersion))
    , m_owner(owner)
    , m_repo(repo)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

UpdateChecker::~UpdateChecker()
{
}

void UpdateChecker::checkForUpdates()
{
    // 构造 GitHub API URL 获取最新 Release
    QString apiUrl = QString("https://api.github.com/repos/%1/%2/releases/latest")
                        .arg(m_owner)
                        .arg(m_repo);
    
    QNetworkRequest request(apiUrl);
    // 设置 User-Agent，GitHub API 要求
    request.setRawHeader("User-Agent", "AutoBrightnessWidget");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished,
            this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::onReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 确保 reply 会被自动删除
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        if (reply->error() == QNetworkReply::ContentNotFoundError) {
            // 404 错误可能是因为还没有任何 Release
            errorMsg = "当前仓库暂无可用版本发布";
        } else {
            errorMsg = QString("网络请求失败: %1").arg(reply->errorString());
        }
        qWarning() << "Update check failed:" << errorMsg;
        emit checkFailed(errorMsg);
        return;
    }
    
    // 解析 JSON 响应
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit checkFailed("解析服务器响应失败");
        return;
    }
    
    QJsonObject releaseObj = doc.object();
    QString latestVersion = normalizeVersion(releaseObj["tag_name"].toString());
    QString releaseUrl = releaseObj["html_url"].toString();
    QString releaseNotes = releaseObj["body"].toString();
    
    if (latestVersion.isEmpty()) {
        emit checkFailed("无法获取版本信息");
        return;
    }
    
    qDebug() << "Current version:" << m_currentVersion;
    qDebug() << "Latest version:" << latestVersion;
    
    // 比较版本号
    int comparison = compareVersions(m_currentVersion, latestVersion);
    if (comparison < 0) {
        // 当前版本低于最新版本
        emit updateAvailable(latestVersion, releaseUrl, releaseNotes);
    } else {
        // 已是最新版本或当前版本更高
        emit noUpdateAvailable();
    }
}

int UpdateChecker::compareVersions(const QString& version1, const QString& version2)
{
    QStringList parts1 = version1.split('.');
    QStringList parts2 = version2.split('.');
    
    int maxLen = std::max(parts1.size(), parts2.size());
    
    for (int i = 0; i < maxLen; ++i) {
        int v1 = (i < parts1.size()) ? parts1[i].toInt() : 0;
        int v2 = (i < parts2.size()) ? parts2[i].toInt() : 0;
        
        if (v1 < v2) {
            return -1;
        } else if (v1 > v2) {
            return 1;
        }
    }
    
    return 0; // 版本相同
}

QString UpdateChecker::normalizeVersion(const QString& version)
{
    QString normalized = version.trimmed();
    // 移除开头的 'v' 或 'V'
    if (normalized.startsWith('v', Qt::CaseInsensitive)) {
        normalized = normalized.mid(1);
    }
    return normalized;
}
