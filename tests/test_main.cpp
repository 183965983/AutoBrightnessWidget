#include <QTest>
#include <QCoreApplication>

// Forward declarations of test classes
class TestBrightness;
class TestUI;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    int status = 0;
    
    // Run brightness calculation tests
    {
        TestBrightness tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    // Run UI tests
    {
        TestUI tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    
    return status;
}
