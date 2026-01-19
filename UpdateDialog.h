#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>

/**
 * @brief UpdateDialog 用于显示软件更新通知
 * 
 * 当检测到新版本时，此对话框会显示版本信息和下载链接
 */
class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param currentVersion 当前版本
     * @param latestVersion 最新版本
     * @param releaseUrl 发布页面 URL
     * @param releaseNotes 发布说明（可选）
     * @param parent 父窗口
     */
    explicit UpdateDialog(const QString& currentVersion,
                         const QString& latestVersion,
                         const QString& releaseUrl,
                         const QString& releaseNotes = QString(),
                         QWidget* parent = nullptr);
    
    ~UpdateDialog();

private slots:
    void onDownloadClicked();
    void onSkipClicked();

private:
    void setupUI();

private:
    QString m_currentVersion;
    QString m_latestVersion;
    QString m_releaseUrl;
    QString m_releaseNotes;
    
    QLabel* m_titleLabel;
    QLabel* m_versionLabel;
    QTextBrowser* m_notesTextBrowser;
    QPushButton* m_downloadButton;
    QPushButton* m_skipButton;
};

#endif // UPDATEDIALOG_H
