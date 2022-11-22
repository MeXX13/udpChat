#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "udpclient.h"
#include "chatwindow.h"
#include "addresswindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void setNewConnectionParameters(QHostAddress addr, quint16 port);
    void dataReceived(QByteArray data, QHostAddress addr, quint16 port);

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    udpClient *myClient;

    typedef struct connectionStruct
    {
        QString addr;
        quint16 port;
        bool operator<( const connectionStruct& other) const {
                 return std::tie(addr,port) < std::tie(other.addr,other.port);
            }

    } connectionStruct;

    QMap<connectionStruct, chatWindow*>  connectionMap;


    void openOrCreateChatWindow(chatWindow **chatWind, QHostAddress addr, quint16 port);

};
#endif // MAINWINDOW_H
