#include "chasseautresorserveurwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChasseAuTresorServeurWidget w;
    w.show();
    return a.exec();
}
