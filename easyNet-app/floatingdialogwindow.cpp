#include "floatingdialogwindow.h"
#include <QDesktopWidget>
#include <QSize>

FloatingDialogWindow::FloatingDialogWindow(QWidget *parent) : QMainWindow()
{
    QRect screenSize = QDesktopWidget().availableGeometry(this);
    this->resize(QSize(screenSize.width() * 0.4f, screenSize.height() * 0.7f));
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
}

FloatingDialogWindow::~FloatingDialogWindow()
{

}

