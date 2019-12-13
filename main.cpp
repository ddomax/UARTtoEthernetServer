#include "bssamplerwidget.h"
#include <QApplication>
#include "api/appinit.h"

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    QApplication a(argc, argv);

    AppInit *initApp = new AppInit;
    initApp->loadInfo();

    BSSamplerWidget w;
    w.show();

    return a.exec();
}
