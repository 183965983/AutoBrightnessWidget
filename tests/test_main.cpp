#include <QTest>
#include <QApplication>

// Forward declarations
class TestBrightness;
class TestUI;
class TestNewFeatures;
class TestMultiMonitor;

// External test class declarations (defined in their respective .cpp files)
extern QObject* createTestBrightnessInstance();
extern QObject* createTestUIInstance();
extern QObject* createTestNewFeaturesInstance();
extern QObject* createTestMultiMonitorInstance();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    int status = 0;
    
    // Run brightness calculation tests
    {
        QObject* tc = createTestBrightnessInstance();
        status |= QTest::qExec(tc, argc, argv);
        delete tc;
    }
    
    // Run UI tests
    {
        QObject* tc = createTestUIInstance();
        status |= QTest::qExec(tc, argc, argv);
        delete tc;
    }
    
    // Run new features tests
    {
        QObject* tc = createTestNewFeaturesInstance();
        status |= QTest::qExec(tc, argc, argv);
        delete tc;
    }
    
    // Run multi-monitor tests
    {
        QObject* tc = createTestMultiMonitorInstance();
        status |= QTest::qExec(tc, argc, argv);
        delete tc;
    }
    
    return status;
}
