#include "messageparser.h"

#include <stdlib.h>
#include <QFileDialog>
#include <QDebug>



messageParser::messageParser(QObject *parent) : QObject(parent)
{
    chatWindow = qobject_cast<QWidget*>(parent);
}



quint16 messageParser::processIncomingData(QByteArray *data)
{
   this->data = data;

   if (data->startsWith("msg"))
    {
        processMessage();
        return 1;
    }

    if (data->startsWith("rpl"))
    {
        //qInfo() << *data;
        processReply();
        return 2;
    }

    if (data->startsWith("fil"))
    {
        processStartFile();
        return 3;
    }

    if (data->startsWith("flr"))
    {
        processFlReply();
        return 4;
    }

    if (data->startsWith("fld"))
    {
        processFlDatagram();
        return 5;
    }

    if (data->startsWith("hsh"))
    {
        //qInfo() << "HASH" << *data;
        processFlHash();
        return 6;
    }

    return 0;
}



void messageParser::processMessage()
{
    sendingData = "rpl";
    sendingData.push_back(data->mid(3, 5));
    emit sendData(&sendingData);

    data->remove(0, 8);
    emit preparedMessage(data);
}


void messageParser::processReply()
{
    data->remove(0, 3);
    uint number = data->toUInt();
    emit replyNumber(number);
}




/////////////////////////////////////
///                File
/////////////////////////////////////
void messageParser::processStartFile()
{
    data->remove(0, 3);
    QString incomingFileParameters = *data;
    incomingFileName =  incomingFileParameters.section('/', 1);
    incomingFileSize = incomingFileParameters.section('/', -1).toULong();

    firstBlockTimer = new QTimer(this);
    connect(firstBlockTimer, &QTimer::timeout, this, &messageParser::onFirstBlockTimer);
    firstBlockTimer->start(2000);


    incomingFileBuffer.resize(incomingFileSize);

    incomingFlSizeInDatagrams = incomingFileSize/PAYLOAD;
    if (incomingFileSize%PAYLOAD)
    {
        incomingFlSizeInDatagrams++;
    }
    quint16 sizeInFullBlocks = incomingFlSizeInDatagrams / incomingBlockSizeInDatagrams;
    incomingFlFirstDatagrInLastBlock = sizeInFullBlocks * incomingBlockSizeInDatagrams;

    resetCurrentIncomingBlockMap();

    sendingData = "flr";
    sendingData.push_back(data->data());
    emit sendData(&sendingData);

//    qInfo() << incomingFileName;
//    qInfo() << incomingFileSize;
}


void messageParser::processFlReply()
{
    data->remove(0, 3);
    if (fileStartTimer)
    {
        if (*data == fileStartStr)
        {
            fileStartTimer->stop();
            delete fileStartTimer;
            fileStartTimer = nullptr;

            startFileTransfer();
        }
    }
}



void messageParser::processFlDatagram()
{
    static quint8 s_numberOfDatagrInCurrentAttemp = 255;
    static quint32 s_curentBlockSizeInBytes;
    static quint16 s_currentBlockSizeInDatagrams;

    if (firstBlockTimer)
    {
        firstBlockTimer->stop();
        delete firstBlockTimer;
        firstBlockTimer = nullptr;
        s_numberOfDatagrInCurrentAttemp = 0;

        incomingDatagramTimer = new QTimer(this);
        connect(incomingDatagramTimer, &QTimer::timeout, this, &messageParser::onIncomingDatagramTimer);
        incomingDatagramTimer->start(500);
    }

    quint32 receivedDatagramNumber = data->mid(3, 5).toULong();

    if (receivedDatagramNumber < currentIncomingFilePositionInDatagrams)
    {
        currentIncomingFilePositionInDatagrams -= incomingBlockSizeInDatagrams;
    }
    qint8 datagramNumberInCurrentBlock = receivedDatagramNumber - currentIncomingFilePositionInDatagrams;

    currentIncomingBlockMap[datagramNumberInCurrentBlock] = true;

    if(s_numberOfDatagrInCurrentAttemp == 0)
    {
        s_currentBlockSizeInDatagrams = incomingBlockSizeInDatagrams;
        s_curentBlockSizeInBytes = incomingBlockSizeInDatagrams*PAYLOAD;
        if (currentIncomingFilePositionInDatagrams == incomingFlFirstDatagrInLastBlock)
        {
            s_currentBlockSizeInDatagrams = incomingFlSizeInDatagrams - incomingFlFirstDatagrInLastBlock;
            s_curentBlockSizeInBytes = incomingFileSize - currentIncomingFilePositionInDatagrams*PAYLOAD;
        }
    }

    s_numberOfDatagrInCurrentAttemp++;

    //data->remove(0, 8);
    memcpy(incomingFileBuffer.data() + receivedDatagramNumber*PAYLOAD, data->data()+8, PAYLOAD);

    if (s_numberOfDatagrInCurrentAttemp >= s_currentBlockSizeInDatagrams)
    {

        bool allDatagramsInBlockRecieved = true;
        for(quint8 i=0; i<s_currentBlockSizeInDatagrams; i++)
        {
            if (currentIncomingBlockMap[i] == false)
            {
                allDatagramsInBlockRecieved = false;
                break;
            }
        }


        if (allDatagramsInBlockRecieved)
        {
            sendHashDatagram(s_curentBlockSizeInBytes);

            if (s_currentBlockSizeInDatagrams == incomingBlockSizeInDatagrams)
            {
                currentIncomingFilePositionInDatagrams += incomingBlockSizeInDatagrams;
                resetCurrentIncomingBlockMap();
                s_numberOfDatagrInCurrentAttemp = 0;
            }
            else
            {
                //file has been received
                incomingDatagramTimer->stop();
                saveFile();

                resetFileIncomingParameters();
            }
        }

    }

    if (incomingDatagramTimer)
    {
        incomingDatagramTimer->start(500);
    }

}



void messageParser::processFlHash()
{
    quint32 receivedHashNumber = data->mid(3, 5).toULong();
    data->remove(0, 8);

    qInfo() <<"HASH" <<*data <<"HASH" << currentOutcomingHashDatagram <<receivedHashNumber <<currentOutcomingFilePositionInDatagrams;


    if ((receivedHashNumber == currentOutcomingFilePositionInDatagrams/outcomingBlockSizeInDatagrams)&&(*data == currentOutcomingHashDatagram))
    {
        qInfo() <<"HASH_OK!!!!";

        currentOutcomingFilePositionInDatagrams += outcomingBlockSizeInDatagrams;
        currentOutcomingFilePosition += outcomingBlockSizeInDatagrams*PAYLOAD;
        numberOfDileveryAttemps = DELIVERY_ATTEMPS;
        blockDeliveryTimer->start(500);

        sendDataBlock();
    }
    else
    {
        onBlockDeliveryError();
    }

}



void messageParser::resetFileIncomingParameters()
{
    incomingFileSize = 0;
    incomingFileName = "";
    incomingFileBuffer = "";
    incomingFlSizeInDatagrams = 0;
    incomingFlSizeInDatagrams = 0;
    incomingFlFirstDatagrInLastBlock = 0;
    currentIncomingFilePositionInDatagrams = 0;
    incomingBlockSizeInDatagrams = 10;

    resetCurrentIncomingBlockMap();

    if (incomingDatagramTimer)
    {
        delete incomingDatagramTimer;
        incomingDatagramTimer = nullptr;
    }
}



void messageParser::startFileTransfer()
{
    if (file->open(QIODevice::ReadOnly))
    {
       outcomingFileBuffer = file->readAll();

       blockDeliveryTimer = new QTimer(this);
       connect(blockDeliveryTimer, &QTimer::timeout, this, &messageParser::onBlockDeliveryError);

       sendDataBlock();
    }
    else
    {
        resetFileTranferParameters();
        QMessageBox::critical(chatWindow, "Error!", "File hasn't been opened");
    }
}


void messageParser::resetFileTranferParameters()
{
    file = nullptr;
    fileStartStr = "";
    outcomingFileSize = 0;
    outcomingFileBuffer = "";
    currentOutcomingFilePosition = 0;
    currentOutcomingFilePositionInDatagrams = 0;
    outcomingBlockSizeInDatagrams = 10;

    if (blockDeliveryTimer)
    {
        delete blockDeliveryTimer;
        blockDeliveryTimer = nullptr;
    }
}


void messageParser::saveFile()
{
    QString fileStr = QFileDialog::getSaveFileName(chatWindow, "Saved received file");
    QFile file(fileStr);
       if (file.open(QFile::WriteOnly))
       {
          file.write(incomingFileBuffer);
          QMessageBox::information(chatWindow, "File saving", "File has benn succesfully saved");
       }
       else
       {
             QMessageBox::critical(chatWindow, "File saving", "File saving error");
       }
}




void messageParser::resetCurrentIncomingBlockMap()
{
    for(quint8 i=0; i<10; i++)
    {
        currentIncomingBlockMap[i] = false;
    }
}




void messageParser::sendDataBlock()
{
    quint32 blockSizeInBytes = outcomingBlockSizeInDatagrams*PAYLOAD;

    quint32 currentNumberOfDatagram = currentOutcomingFilePositionInDatagrams;

    quint16 numberOfWholeDiagramsInCurrentBlock;
    quint16 lastPayloadSize = 0;

    quint16 positionInCurrentBlock = currentOutcomingFilePosition;

    if (currentOutcomingFilePosition+blockSizeInBytes <= outcomingFileSize)
    {
       numberOfWholeDiagramsInCurrentBlock = outcomingBlockSizeInDatagrams;
    }
    else
    {
        blockSizeInBytes = outcomingFileSize - currentOutcomingFilePosition;
        numberOfWholeDiagramsInCurrentBlock = blockSizeInBytes/PAYLOAD;
        lastPayloadSize = blockSizeInBytes%PAYLOAD;
    }


    for (quint16 i = 0; i<numberOfWholeDiagramsInCurrentBlock; i++)
    {
        sendFileDatagram(currentNumberOfDatagram, positionInCurrentBlock);

        currentNumberOfDatagram++;
        positionInCurrentBlock += PAYLOAD;
    }

    if(lastPayloadSize)
    {
        sendFileDatagram(currentNumberOfDatagram, positionInCurrentBlock);
    }

    //calculate HASH
    char *firstBlockSymbal = outcomingFileBuffer.data() + currentOutcomingFilePosition;
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(firstBlockSymbal, blockSizeInBytes);
    currentOutcomingHashDatagram = hash.result();

    blockDeliveryTimer->start(500);

}



void messageParser::sendFileDatagram(quint32 currentNumberOfDatagram,  quint16 positionInCurrentBlock)
{
    QString currentHeader = "fld" + QString::number(currentNumberOfDatagram).rightJustified(5, '0');
    sendingData.resize(PAYLOAD+8);
    sendingData.push_front(currentHeader.toLocal8Bit());
    memcpy(sendingData.data()+8, outcomingFileBuffer.data()+positionInCurrentBlock, PAYLOAD);
    //qInfo() << currentDatagram;
    emit sendData(&sendingData);
}



void messageParser::sendHashDatagram(quint32 blockSizeInBytes)
{
    char *firstBlockSymbal = incomingFileBuffer.data() + currentIncomingFilePositionInDatagrams*PAYLOAD;

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(firstBlockSymbal, blockSizeInBytes);

    sendingData = hash.result();
    quint32 currentPositionInBlocks = currentIncomingFilePositionInDatagrams/incomingBlockSizeInDatagrams;
    QString hashHeader = "hsh" + QString::number(currentPositionInBlocks).rightJustified(5, '0');

    sendingData.push_front(hashHeader.toLocal8Bit());
    emit sendData(&sendingData);

}



void messageParser::sendFile(QSharedPointer<QFile> fl)
{
   file = fl;

   QString path =  file->fileName();
   QString name = path.section('/', -1);
   outcomingFileSize = file->size();
   QString size = QString::number(outcomingFileSize);
   fileStartStr = name + '/' + size;
   QString sentString = "fil" + fileStartStr;

   sendingData = sentString.toLocal8Bit();
   fileStartTimer = new QTimer(this);
   fileStartTimer->start(2000);
   connect(fileStartTimer, &QTimer::timeout, this, &messageParser::onFileStartTimer);

   emit sendData(&sendingData);

}



void messageParser::onFileStartTimer()
{
    if (fileStartTimer)
    {
        delete fileStartTimer;
        fileStartTimer = nullptr;
    }

    resetFileTranferParameters();
    QMessageBox::critical(chatWindow, "File transfer error", "Remote host hasn't accepted transfer");
}



void messageParser::onFirstBlockTimer()
{
    if (firstBlockTimer)
    {
        delete firstBlockTimer;
        firstBlockTimer = nullptr;
    }

    resetFileIncomingParameters();
    QMessageBox::critical(chatWindow, "File receiving error", "Remote host has interrupted tranfer");

}


void messageParser::onIncomingDatagramTimer()
{

    if (incomingDatagramTimer)
    {
        delete incomingDatagramTimer;
        incomingDatagramTimer = nullptr;
    }

    resetFileIncomingParameters();
    QMessageBox::critical(chatWindow, "File receiving error", "Remote host has interrupted tranfer");

}


void messageParser::onBlockDeliveryError()
{
    if (numberOfDileveryAttemps>0)
    {
        numberOfDileveryAttemps--;
        sendDataBlock();
    }
    else
    {
        resetFileTranferParameters();
        QMessageBox::critical(chatWindow, "File transfer error", "Transfer has interrupted");
    }
}



