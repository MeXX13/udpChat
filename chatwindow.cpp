#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "random.hpp"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QSharedPointer>

chatWindow::chatWindow(QWidget *parent, QHostAddress addr, quint16 port) :
    QMainWindow(parent),
    ui(new Ui::chatWindow)
{
    ui->setupUi(this);
    mParser = new messageParser(this);
    connect(mParser, &messageParser::preparedMessage, this, &chatWindow::preparedMessage);
    connect(mParser, &messageParser::sendData, this, &chatWindow::sendData);
    connect(mParser, &messageParser::replyNumber, this, &chatWindow::addConfirmedMessageNumber);

    htmlVwr = new htmlViewer(this, ui->chatTextEdit);

    this->port = port;
    this->addr = addr;

    ui->messageTextEdit->setFocus();
}

chatWindow::~chatWindow()
{
    delete ui;
}


void chatWindow::on_pushButton_clicked()
{

    QString messageString = ui->messageTextEdit->toPlainText();
    if (messageString != "")
    {
        messageNumber++;
        QByteArray message= "msg";
        message.push_back(QString::number(messageNumber).toLocal8Bit());
        message.push_back(messageString.toLocal8Bit());

        sendData(&message);

        htmlVwr->addMessage(message, messageNumber, messagesTypes::outgoing);
        ui->messageTextEdit->clear();

    }

}




void chatWindow::processMessage(QByteArray data)
{
    mParser->processIncomingData(&data);
}


void chatWindow::sendData(QByteArray *data)
{
    emit messageToSend(*data, addr, port);
}


void chatWindow::preparedMessage(QByteArray *data)
{
//    ui->chatTextEdit->append("\r\n");
//    ui->chatTextEdit->append(*data);
//    ui->messageTextEdit->clear();
    htmlVwr->addMessage(*data, messageNumber, messagesTypes::incoming);
}


void chatWindow::addConfirmedMessageNumber(uint number)
{
    //confirmedMessagesList.push_back(number);
    htmlVwr->makeMessageMarkedConfirmes(number);
}



void chatWindow::closeEvent(QCloseEvent *event)
{
   event->ignore();
   this->hide();
}



void chatWindow::on_filePushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file for transfer");
    QSharedPointer<QFile> file;
    file = QSharedPointer<QFile>(new QFile(fileName, this));

    //QFile file(fileName, this);


    quint64 size = file->size();
    if (size > 20000000)
    {
        QMessageBox::warning(this, "Warning!", "File is too big");
    }
    else
    {
        mParser->sendFile(file);
    }

}

