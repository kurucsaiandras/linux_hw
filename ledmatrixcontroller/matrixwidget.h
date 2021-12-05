#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H

#include <QWidget>
#include <QtWebSockets/QWebSocket>
#include <QProcess>

class matrixWidget : public QWidget
{
    Q_OBJECT
public:
    explicit matrixWidget(QWidget *parent = nullptr);

signals:
    void signalSend(QByteArray b);
    void signalFileChosen(bool);

public slots:
    void slotClear();
    void slotInvert();
    void slotSave();
    void slotOpen();
    void slotSendOut();
    void slotSliderValChanged(int i);
    void slotDeleteFrame();
    void slotCopyFrame();
    void slotPasteFrame();
    void slotCheckMic();
    void slotMic(bool toggled);

protected:
    //a kepkockak matrixait tarolo tomb (tobb kepkocka eseten 3 dimenzios)
    QVector<QVector<QVector<int>>> animFrameBuffer;

    //a vagolapra masolt kepkocka
    QVector<QVector<int>> copiedFrame;

    //a mikrofon figyelo modhoz betoltott kepkocka
    QVector<QVector<int>> micDisplayFrame;

    //a NetHandler-nek kuldendo, LED kijelzovel kompatibilis bájtsor
    uint8_t dataOut[64];

    //az eppen kijelzett kepkocka indexe
    int currentFrame;

protected:
    void init();
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void paintEvent(QPaintEvent* e);

private:

    //mikrofonfigyelo modban a kijelzo torlesehez szukseges valtozo
    //ha a mikrofon allapota megvaltozott, eleg egyszer kiirni,
    //nem kell minden ciklusban
    typedef enum{
        START,
        DISPLAYED,
        CLEARED,
    } LED_state_typedef;
    LED_state_typedef LED_state;

    //a mikrofon allapotat lekero bash process
    QProcess* proc;

    //a mikrofon allapotat idokozonkent lekero timer
    QTimer* checkMicTimer;
};

#endif // MATRIXWIDGET_H
