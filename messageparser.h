#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#define PAYLOAD 50
#define DELIVERY_ATTEMPS 3

#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <QCryptographicHash>


class messageParser : public QObject
{
    Q_OBJECT
public:
    explicit messageParser(QObject *parent = nullptr);

public slots:
    quint16 processIncomingData(QByteArray *data);
    void sendFile (QSharedPointer<QFile> fl);

private slots:
    void onFileStartTimer();
    void onFirstBlockTimer();
    void onIncomingDatagramTimer();
    void onBlockDeliveryError();

private:
    QWidget *chatWindow;
    QByteArray *data;
    QByteArray sendingData;

    QSharedPointer<QFile> file;
    QString fileStartStr;

    QTimer *fileStartTimer = nullptr;
    QTimer *blockDeliveryTimer = nullptr;

    QTimer *firstBlockTimer = nullptr;
    QTimer *incomingDatagramTimer = nullptr;

    quint64 incomingFileSize = 0;
    QString incomingFileName = "";
    QByteArray incomingFileBuffer;
    quint32 incomingFlSizeInDatagrams = 0;
    quint16 incomingFlFirstDatagrInLastBlock = 0;
    quint32 currentIncomingFilePositionInDatagrams = 0;
    quint16 incomingBlockSizeInDatagrams = 10;
    //QVector<bool> currentIncomingBlockMap;
    bool currentIncomingBlockMap[10];

    quint64 outcomingFileSize = 0;
    QByteArray outcomingFileBuffer;
    quint64 currentOutcomingFilePosition = 0;
    quint32 currentOutcomingFilePositionInDatagrams = 0;
    quint16 outcomingBlockSizeInDatagrams = 10;
    QByteArray currentOutcomingHashDatagram;
    quint8 numberOfDileveryAttemps = DELIVERY_ATTEMPS;


    void processMessage();
    void processReply();

    void processStartFile();
    void processFlReply();
    void processFlDatagram();
    void processFlHash();
    void resetFileIncomingParameters();

    void startFileTransfer();
    void sendDataBlock();
    void sendFileDatagram(quint32 currentNumberOfDatagram,  quint16 positionInCurrentBlock);
    void sendHashDatagram(quint32 blockSizeInBytes);
    void resetFileTranferParameters();

    void saveFile();
    void resetCurrentIncomingBlockMap();


signals:
    void preparedMessage(QByteArray *data);
    void sendData(QByteArray *data);
    void replyNumber(uint number);


};

#endif // MESSAGEPARSER_H
