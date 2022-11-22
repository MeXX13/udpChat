#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myClient = new udpClient(this);
    connect(myClient, &udpClient::dataReceived, this, &MainWindow::dataReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

    addressWindow *aw = new addressWindow(this);
    aw->show();
    connect(aw, &addressWindow::newConnection, this, &MainWindow::setNewConnectionParameters);
}


void MainWindow::setNewConnectionParameters(QHostAddress addr, quint16 port)
{
    chatWindow *cw = nullptr;
    openOrCreateChatWindow(&cw, addr, port);
    if (cw)
    {
        cw->show();
    }
}



void MainWindow::dataReceived(QByteArray data, QHostAddress addr, quint16 port)
{
    chatWindow *cw = nullptr;

    openOrCreateChatWindow(&cw, addr, port);
    if (cw)
    {
        cw->show();
        cw->processMessage(data);
    }



}


void MainWindow::openOrCreateChatWindow(chatWindow **chatWind, QHostAddress addr, quint16 port)
{

    connectionStruct conStr;
    conStr.addr = addr.toString();
    conStr.port = port;

    chatWindow *cw;
    if (!connectionMap.contains(conStr))
    {
        cw = new chatWindow(this, addr, port);
        connect(cw, &chatWindow::messageToSend, myClient, &udpClient::sendData);

        connectionMap.insert(conStr, cw);

        QString connectionInfo = addr.toString();
        connectionInfo += " (";
        connectionInfo += QString::number(port);
        connectionInfo += ')';

        this->ui->listWidget->addItem(connectionInfo);
        qInfo() << "newChat";
    }
    else
    {
        cw = connectionMap.value(conStr);
        qInfo() << "existentChat";
    }

    *chatWind = cw;

}




void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
   QString str = item->text();
   str.remove('(');
   str.remove(')');

   QStringList strList = str.split(' ');

   connectionStruct conStr;
   conStr.addr = strList[0];
   conStr.port = strList[1].toUInt();

   chatWindow *cw =  connectionMap.value(conStr);
   cw->show();

}

