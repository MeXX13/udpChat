#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>

#include <QDebug>

class udpClient : public QObject
{
    Q_OBJECT
public:
    explicit udpClient(QObject *parent = nullptr);
    void bindSocket(QHostAddress addr, quint16 port);
private:
    QUdpSocket *mySoc;

private slots:
    void readyRead();

public slots:
    void sendData(QByteArray data, QHostAddress addr, quint16 port);

signals:
    void dataReceived(QByteArray data, QHostAddress addr, quint16 port);
    void clientError(quint16 errorCode);


};

#endif // UDPCLIENT_H
