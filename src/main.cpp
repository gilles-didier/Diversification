#include "diversificationwindow.h"
#include <QApplication>
#include <locale.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "C");
    qRegisterMetaType<TypeEstimation>("TypeEstimation");
    QApplication a(argc, argv);
    DiversificationWindow w;
    w.show();

    return a.exec();
}
