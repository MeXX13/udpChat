#include "htmlviewer.h"
#include <QDebug>

htmlViewer::htmlViewer(QObject *parent, QTextEdit *txtEdit) : QObject(parent)
{
    txtEdit->clear();
    this->txtEdit = txtEdit;
}



QByteArray htmlViewer::createHtmlMessage(QByteArray message, uint number, messagesTypes type, QTime time)
{
    QByteArray currentHtmlMessage = "";

    QString firstPart;
    QString secondPart = "<!--" + QString::number(number) + "-->";
    QString strTime = time.toString();
    QString thirdPart = "<i>" + strTime +"</i>";
    QString fourthPart;
    QString lastPart = "</br></p>";


    switch (type) {

    case messagesTypes::incoming:
    {
        firstPart = "<p style=\"background-color:#DBE773; margin-left: 20px; margin-right: 45px;\">";
        fourthPart = "<font color=#0><br>";
    }
        break;
    case messagesTypes::outgoing:
    {
        firstPart = "<p style=\"background-color:#C1FCF9; margin-left: 45px; margin-right: 20px;\">";
        fourthPart = "<font color=#C1C3C3><br>";
    }
        break;
    case messagesTypes::outConfirmed:
    {
        firstPart = "<p style=\"background-color:#C1FCF9; margin-left: 45px; margin-right: 20px;\">";
        fourthPart = "<font color=#0><br>";
    }
        break;

    }

    currentHtmlMessage.push_back(firstPart.toLocal8Bit());
    currentHtmlMessage.push_back(secondPart.toLocal8Bit());
    currentHtmlMessage.push_back(thirdPart.toLocal8Bit());
    currentHtmlMessage.push_back(fourthPart.toLocal8Bit());
    currentHtmlMessage.push_back(message);
    currentHtmlMessage.push_back(lastPart.toLocal8Bit());

    return currentHtmlMessage;
}



void htmlViewer::modifyTextEdit()
{
    txtEdit->clear();
    txtEdit->setHtml(htmlMessages);
}




void htmlViewer::addMessage(QByteArray message, uint number, messagesTypes type)
{
    if (type == messagesTypes::incoming)
        number += 50000;

    QTime time = QDateTime::currentDateTime().time();

    QByteArray htmlMessage = createHtmlMessage(message, number, type, time);

    messageStruct mesStruct;
    mesStruct.length = htmlMessage.length();
    mesStruct.message = message;
    mesStruct.number = number;
    mesStruct.startPosition = currentPosition;
    mesStruct.time = time;
    mesStruct.type = type;

    messagesMap.insert(number, mesStruct);

    htmlMessages.push_back(htmlMessage);

    currentPosition += htmlMessage.length();

    modifyTextEdit();

}



void htmlViewer::makeMessageMarkedConfirmes(uint number)
{
    auto mapRecord = messagesMap.find(number);
    messageStruct *mS = &mapRecord.value();

    QByteArray *curMes = &mS->message;
    QTime messageTime = mS->time;
    uint oldLength = mS->length;
    uint startPosition = mS->startPosition;

    QByteArray newMes = createHtmlMessage(*curMes, number, messagesTypes::outConfirmed, messageTime);
    uint newLength = newMes.length();

    htmlMessages.replace(startPosition, oldLength, newMes);

    mS->message = newMes;
    mS->type = messagesTypes::outConfirmed;

    if (newLength != oldLength)
    {
        mS->length = newLength;
        int lengthDifference = newLength - oldLength;

        currentPosition += lengthDifference;

        while(mapRecord != messagesMap.end())
        {
            mapRecord++;
            mapRecord.value().startPosition += lengthDifference;
        }
    }

    modifyTextEdit();
}



