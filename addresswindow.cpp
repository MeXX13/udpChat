#include "addresswindow.h"
#include "ui_addresswindow.h"

#include <QMessageBox>

#include <QtDebug>

addressWindow::addressWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::addressWindow)
{
    ui->setupUi(this);
}

addressWindow::~addressWindow()
{
    delete ui;
}

void addressWindow::on_pushButton_clicked()
{
    QString addrStr = ui->ipLineEdit->text();
    QString portStr = ui->portLineEdit->text();

    QHostAddress addr(addrStr);

    bool convertOk;
    quint16 port = portStr.toUInt(&convertOk);

    //qDebug() << addr;

    if (convertOk && (addr.toString() != ""))
    {
        emit newConnection(addr, port);
        close();
    }
    else
    {
       QMessageBox::critical(this, "Error", "Wrong net parameters",  QMessageBox::Ok, QMessageBox::Ok);
    }

}

