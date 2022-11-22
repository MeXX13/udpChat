#ifndef ADDRESSWINDOW_H
#define ADDRESSWINDOW_H

#include <QMainWindow>
#include <QHostAddress>

namespace Ui {
class addressWindow;
}

class addressWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit addressWindow(QWidget *parent = nullptr);
    ~addressWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::addressWindow *ui;

signals:
    void newConnection(QHostAddress addr, quint16 port);
};

#endif // ADDRESSWINDOW_H
