#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

/**
 * @brief UpdateChecker 类用于检查 GitHub Releases 中的软件更新
 * 
 * 该类会异步查询 GitHub API，获取最新的 Release 信息，
 * 并与当前版本进行比较，发出相应的信号通知调用者。
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param currentVersion 当前应用版本号，例如 "0.1.0"
     * @param owner GitHub 仓库所有者
     * @param repo GitHub 仓库名称
     * @param parent 父对象
     */
    explicit UpdateChecker(const QString& currentVersion,
                          const QString& owner,
                          const QString& repo,
                          QObject* parent = nullptr);
    
    ~UpdateChecker();

    /**
     * @brief 开始检查更新（异步）
     */
    void checkForUpdates();

signals:
    /**
     * @brief 发现新版本时发出此信号
     * @param latestVersion 最新版本号
     * @param releaseUrl 发布页面 URL
     * @param releaseNotes 发布说明
     */
    void updateAvailable(const QString& latestVersion,
                        const QString& releaseUrl,
                        const QString& releaseNotes);
    
    /**
     * @brief 已是最新版本时发出此信号
     */
    void noUpdateAvailable();
    
    /**
     * @brief 检查更新失败时发出此信号
     * @param errorMessage 错误信息
     */
    void checkFailed(const QString& errorMessage);

private slots:
    void onReplyFinished();

private:
    /**
     * @brief 比较两个版本号
     * @param version1 版本1
     * @param version2 版本2
     * @return 如果 version1 < version2 返回 -1，相等返回 0，大于返回 1
     */
    int compareVersions(const QString& version1, const QString& version2);
    
    /**
     * @brief 从版本字符串中移除 'v' 前缀
     * @param version 版本字符串，如 "v0.1.0" 或 "0.1.0"
     * @return 规范化的版本字符串，如 "0.1.0"
     */
    QString normalizeVersion(const QString& version);

private:
    QString m_currentVersion;
    QString m_owner;
    QString m_repo;
    QNetworkAccessManager* m_networkManager;
};

#endif // UPDATECHECKER_H
