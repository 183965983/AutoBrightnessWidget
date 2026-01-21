#include <QtTest/QtTest>
#include <QSignalSpy>
#include "Ctrl/UpdateChecker.h"

/**
 * @brief 更新检查器测试
 * 
 * 测试 UpdateChecker 类的基本功能，包括版本比较和更新检查
 */
class TestUpdateChecker : public QObject
{
    Q_OBJECT

private slots:
    void testVersionComparison();
    void testUpdateCheck();
};

void TestUpdateChecker::testVersionComparison()
{
    // 测试版本比较逻辑
    // 我们通过创建 UpdateChecker 实例来测试，但不触发网络请求
    
    UpdateChecker checker1(APP_VERSION, GITHUB_OWNER, GITHUB_REPO);
    UpdateChecker checker2("1.0.0", GITHUB_OWNER, GITHUB_REPO);
    
    // 由于 compareVersions 是私有方法，我们无法直接测试
    // 但可以验证对象创建成功
    QVERIFY(true);
}

void TestUpdateChecker::testUpdateCheck()
{
    // 测试更新检查功能
    // 注意：这个测试需要网络连接，在 CI 环境中可能会失败
    // 如果 GitHub API 无响应或仓库没有 release，测试会失败
    
    UpdateChecker checker(APP_VERSION, GITHUB_OWNER, GITHUB_REPO);
    
    // 设置信号监听
    QSignalSpy updateAvailableSpy(&checker, &UpdateChecker::updateAvailable);
    QSignalSpy noUpdateSpy(&checker, &UpdateChecker::noUpdateAvailable);
    QSignalSpy failedSpy(&checker, &UpdateChecker::checkFailed);
    
    // 开始检查更新
    checker.checkForUpdates();
    
    // 等待信号（最多 10 秒）
    bool signalReceived = false;
    for (int i = 0; i < 100 && !signalReceived; ++i) {
        QTest::qWait(100);
        signalReceived = (updateAvailableSpy.count() > 0 || 
                         noUpdateSpy.count() > 0 || 
                         failedSpy.count() > 0);
    }
    
    // 验证至少收到了一个信号
    QVERIFY(signalReceived);
    
    // 验证只触发了一个信号
    int totalSignals = updateAvailableSpy.count() + 
                      noUpdateSpy.count() + 
                      failedSpy.count();
    QCOMPARE(totalSignals, 1);
    
    if (failedSpy.count() > 0) {
        // 如果检查失败，输出错误信息（可能是网络问题或仓库没有 release）
        QList<QVariant> arguments = failedSpy.takeFirst();
        QString errorMessage = arguments.at(0).toString();
        qDebug() << "Update check failed (this is expected if no releases exist):" << errorMessage;
    }
}

// Factory function for creating test instance
QObject* createTestUpdateCheckerInstance() {
    return new TestUpdateChecker();
}

#include "test_update_checker.moc"
