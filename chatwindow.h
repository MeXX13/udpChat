#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QMainWindow>
#include <QtEvents>
#include <QHostAddress>
#include "messageparser.h"
#include "htmlviewer.h"
#include "commonTypes.h"

#include <QtDebug>

namespace Ui {
class chatWindow;
}

class chatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit chatWindow(QWidget *parent = nullptr, QHostAddress addr = QHostAddress::LocalHost , quint16 port = 5555);
    ~chatWindow();

private slots:
    void on_pushButton_clicked();
    void preparedMessage(QByteArray *data);
    void addConfirmedMessageNumber(uint number);


    void on_filePushButton_clicked();

public slots:
    void processMessage(QByteArray data);
    void sendData(QByteArray *data);


private:
    Ui::chatWindow *ui;
    QHostAddress addr;
    quint16 port;
    messageParser *mParser;
    htmlViewer *htmlVwr;
    uint messageNumber = 9999;
    QList <uint> confirmedMessagesList;

    void closeEvent(QCloseEvent *event);

signals:
    void messageToSend(QByteArray data, QHostAddress addr, quint16 port);
};

#endif // CHATWINDOW_H
