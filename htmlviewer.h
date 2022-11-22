#ifndef HTMLVIEWER_H
#define HTMLVIEWER_H

#include <QObject>
#include <QTextEdit>
#include <QDateTime>
#include "commonTypes.h"

class htmlViewer : public QObject
{
    Q_OBJECT
public:
    explicit htmlViewer(QObject *parent = nullptr, QTextEdit *txtEdit = nullptr);

private:
    QByteArray createHtmlMessage(QByteArray message, uint number, messagesTypes type, QTime time);
    void modifyTextEdit();

    QTextEdit *txtEdit;

    typedef struct messageStruct
    {
        uint number;
        QByteArray message;
        QTime time;
        messagesTypes type;
        uint startPosition;
        uint length;
    } messageStruct;

    QByteArray htmlMessages;
    QMap <uint, messageStruct> messagesMap;
    uint currentPosition = 0;




public slots:
    void addMessage(QByteArray message, uint number, messagesTypes type);
    void makeMessageMarkedConfirmes(uint mumber);

signals:

};

#endif // HTMLVIEWER_H


//    QString time = QDateTime::currentDateTime().time().toString();
//    QString mes = "This is a message";

//    ui->chatTextEdit->setHtml("<p style=\"background-color:#C1FCF9; margin-left: 45px; margin-right: 20px;\">"
//    "<!--00000-->"
//    "<i>" + time +"</i>"
//    "<font color=#C1C3C3>"
//    "<br>"+mes+"</br>"
//    "</p>");

//    ui->chatTextEdit->append("<p style=\"background-color:#DBE773; margin-left: 20px; margin-right: 45px;\">"
//    "<!--00000-->"
//    "<i>" + time +"</i>"
//    "<font color=#0>"
//    "<br>"+mes+"</br>"
//    "</p>");
