#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class NetHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slotConnectionStatus(int status);
    void slotFileChosen(bool);

private slots:
    void on_liveView_toggled(bool checked);
    void on_actionAnimation_toggled(bool toggled);
    void slotConnect();
    void on_nextFrame_clicked();
    void on_prevFrame_clicked();
    void on_deleteFrame_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_playpauseButton_clicked();
    void updateFrame();

signals:

private:
    Ui::MainWindow *ui;
    NetHandler* m_pNetHandler;
    QTimer* playTimer;
    QTimer* liveVievTimer;
};
#endif // MAINWINDOW_H
