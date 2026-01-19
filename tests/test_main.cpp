#include <QTest>
#include <QApplication>

// Forward declarations
class TestBrightness;
class TestUI;
class TestNewFeatures;
class TestUpdateChecker;

// External test class declarations (defined in their respective .cpp files)
extern QObject* createTestBrightnessInstance();
extern QObject* createTestUIInstance();
extern QObject* createTestNewFeaturesInstance();
extern QObject* createTestUpdateCheckerInstance();

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
    
    // Run update checker tests
    {
        QObject* tc = createTestUpdateCheckerInstance();
        status |= QTest::qExec(tc, argc, argv);
        delete tc;
    }
    
    return status;
}
