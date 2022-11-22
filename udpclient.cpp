#include "udpclient.h"

udpClient::udpClient(QObject *parent) : QObject(parent)
{
    mySoc = new QUdpSocket(this);
    bindSocket(QHostAddress::Any, 5558);
}


void udpClient::bindSocket(QHostAddress addr, quint16 port)
{

    if(mySoc->bind(addr, port))
    {
        connect(mySoc, &QUdpSocket::readyRead, this, &udpClient::readyRead);
    }
    else
    {
        emit clientError(1);
        qInfo() << "binding error";
    }
}



void udpClient::readyRead()
{
    QByteArray incomingData;
    int dataSize = mySoc->pendingDatagramSize();

    if (dataSize>0)
    {
        incomingData.resize(dataSize);
    }

    QHostAddress addr;
    quint16 port;
    int size = mySoc->readDatagram(incomingData.data(), dataSize, &addr, &port);



    if ((dataSize == size)&&(size != -1))
    {

    QHostAddress ipv4Addr(addr.toIPv4Address());

    emit dataReceived(incomingData, ipv4Addr, port);

    qInfo() << incomingData;
    qInfo() <<  ipv4Addr;
    qInfo() << port;
    }
    else
    {
        emit clientError(2);
        qInfo() << "receiving error";
    }
}



void udpClient::sendData(QByteArray data, QHostAddress addr, quint16 port)
{
    int dataSize = mySoc->writeDatagram(data, addr, port);

    if (dataSize != data.length())
    {
        emit clientError(3);
        qInfo() << "transmition error";
    }
}
