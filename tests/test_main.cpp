#include <QTest>
#include <QApplication>

// Include test classes
#include "test_brightness.cpp"
#include "test_ui.cpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
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
