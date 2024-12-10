#include "chasseautresorclientwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChasseAuTresorClientWidget w;
    w.show();
    return a.exec();
}
