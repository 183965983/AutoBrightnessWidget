#include "UpdateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QFont>

UpdateDialog::UpdateDialog(const QString& currentVersion,
                         const QString& latestVersion,
                         const QString& releaseUrl,
                         const QString& releaseNotes,
                         QWidget* parent)
    : QDialog(parent)
    , m_currentVersion(currentVersion)
    , m_latestVersion(latestVersion)
    , m_releaseUrl(releaseUrl)
    , m_releaseNotes(releaseNotes)
    , m_titleLabel(nullptr)
    , m_versionLabel(nullptr)
    , m_notesTextBrowser(nullptr)
    , m_downloadButton(nullptr)
    , m_skipButton(nullptr)
{
    setupUI();
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::setupUI()
{
    setWindowTitle("软件更新");
    setMinimumSize(500, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 标题
    m_titleLabel = new QLabel("发现新版本！", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);
    
    // 版本信息
    QString versionText = QString("当前版本: %1\n最新版本: %2")
                            .arg(m_currentVersion)
                            .arg(m_latestVersion);
    m_versionLabel = new QLabel(versionText, this);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    QFont versionFont = m_versionLabel->font();
    versionFont.setPointSize(10);
    m_versionLabel->setFont(versionFont);
    mainLayout->addWidget(m_versionLabel);
    
    mainLayout->addSpacing(10);
    
    // 发布说明
    if (!m_releaseNotes.isEmpty()) {
        QLabel* notesLabel = new QLabel("更新内容:", this);
        QFont notesLabelFont = notesLabel->font();
        notesLabelFont.setBold(true);
        notesLabel->setFont(notesLabelFont);
        mainLayout->addWidget(notesLabel);
        
        m_notesTextBrowser = new QTextBrowser(this);
        m_notesTextBrowser->setPlainText(m_releaseNotes);
        m_notesTextBrowser->setMaximumHeight(200);
        mainLayout->addWidget(m_notesTextBrowser);
    }
    
    mainLayout->addStretch();
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_downloadButton = new QPushButton("下载新版本", this);
    m_downloadButton->setMinimumWidth(120);
    connect(m_downloadButton, &QPushButton::clicked,
            this, &UpdateDialog::onDownloadClicked);
    buttonLayout->addWidget(m_downloadButton);
    
    buttonLayout->addSpacing(10);
    
    m_skipButton = new QPushButton("暂不更新", this);
    m_skipButton->setMinimumWidth(120);
    connect(m_skipButton, &QPushButton::clicked,
            this, &UpdateDialog::onSkipClicked);
    buttonLayout->addWidget(m_skipButton);
    
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

void UpdateDialog::onDownloadClicked()
{
    // 在浏览器中打开下载页面
    QDesktopServices::openUrl(QUrl(m_releaseUrl));
    accept();
}

void UpdateDialog::onSkipClicked()
{
    reject();
}
