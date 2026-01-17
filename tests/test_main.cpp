#include <QTest>
#include <QApplication>

// Note: Including .cpp files here is intentional for Qt Test framework.
// Each test file contains Qt MOC includes that need to be in the same
// translation unit. This is a common pattern for Qt Test organization.
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
