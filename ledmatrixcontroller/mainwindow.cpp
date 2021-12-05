#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include "matrixwidget.h"
#include "nethandler.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //a socket kommunikacioert felelos osztaly:
    m_pNetHandler = new NetHandler(this);

    //animacios modban a frame-ek lejatszasaert felelos idozito:
    playTimer = new QTimer(this);

    //elo modban a folyamatos adatkuldesert felelos idozito:
    liveVievTimer = new QTimer(this);

        //a halozati kapcsolat allapotat kommunikalja
        connect(m_pNetHandler,
                SIGNAL(signalConnectionStatus(int)),
                this,
                SLOT(slotConnectionStatus(int)));

        //a kikuldendo bajtokat atadjuk a NetHandlernek
        connect(ui->LEDmatrixWidget,
                SIGNAL(signalSend(QByteArray)),
                m_pNetHandler,
                SLOT(slotSend(QByteArray)));

        //az animacio lejatszasat az updateFrame vegzi
        connect(playTimer,
                SIGNAL(timeout()),
                this,
                SLOT(updateFrame()));

        //elo modban periodusonkent adatkuldes
        connect(liveVievTimer,
                SIGNAL(timeout()),
                ui->LEDmatrixWidget,
                SLOT(slotSendOut()));

        //mikrofon figyelo modban a menuelemek kikapcsolasaert felelo kapcsolat
        connect(ui->LEDmatrixWidget,
                SIGNAL(signalFileChosen(bool)),
                this,
                SLOT(slotFileChosen(bool)));

        //alapertelmezetten az animacios modot kikapcsoljuk es a Disconnect opciot is letiltjuk
        ui->animToolWidget->hide();
        ui->actionDisconnect->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// A kapcsolat allapotanak kijelzese es a menu opciok engedelyezese
void MainWindow::slotConnectionStatus(int status)
{
    // A halozati kapcsolat allapotanak kiirasa a status bar-ra.
    switch(status)
    {
    case NetHandler::Error:
        statusBar()->showMessage("Error: Can not connect to Raspberry");
        break;
    case NetHandler::Connecting:
        statusBar()->showMessage("Connecting...");
        break;
    case NetHandler::Connected:
        statusBar()->showMessage("Connected to Raspberry");
        break;
    case NetHandler::Disconnected:
        statusBar()->showMessage("Disconnected");
        break;
    }
    if((status == NetHandler::Error) || (status == NetHandler::Disconnected))
    {
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
    }
    else if((status == NetHandler::Connecting) || (status == NetHandler::Connected))
    {
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
    }
}


//elo (valos ideju) adattovabbitas engedelyezese a LED kijelzore
void MainWindow::on_liveView_toggled(bool checked)
{
    if(checked){
        ui->sendtoScreen->setEnabled(false);
        liveVievTimer->start(100);  //a timer signal-t kuld az adattovabbitashoz 0.1 masodpercenkent
    }
    else{
        ui->sendtoScreen->setEnabled(true);
        liveVievTimer->stop();
        qDebug() << "STOP";
    }
}

//animacios mod ezkoztaranak be/kikapcsolasa
void MainWindow::on_actionAnimation_toggled(bool toggled)
{
    if(toggled){
        ui->animToolWidget->show();
    }
    else
        ui->animToolWidget->hide();

}

//a menuben a Connect menupont hatasara hivodik meg
//segitsegevel kapcsolodunk a szerverhez
void MainWindow::slotConnect()
{
    // Bekerjuk a masik gep cimet. Ha ez nem nulla hosszusagu, akkor kapcsolodunk hozza.
        bool ok;
        QString addr = QInputDialog::getText(this,
          "Server address", "Please enter the server host:",
          QLineEdit::Normal, "192.168.137.71", &ok);
        if(ok && !addr.isEmpty())
        {
            m_pNetHandler->Connect(addr);
        }
}

//kovetkezo kepkockara lepes
void MainWindow::on_nextFrame_clicked()
{
    int max = ui->horizontalSlider->maximum();
    int val = ui->horizontalSlider->sliderPosition();
    if(val == max){
        ui->horizontalSlider->setMaximum(max+1);
    }
    ui->horizontalSlider->setValue(val+1);
}

//elozo kepkockara lepes
void MainWindow::on_prevFrame_clicked()
{
    int val = ui->horizontalSlider->sliderPosition();
    ui->horizontalSlider->setValue(qMax(0,val-1));
}

//kepkocka torlesekor a slider allitasa
void MainWindow::on_deleteFrame_clicked()
{
    int max = ui->horizontalSlider->maximum();
    int val = ui->horizontalSlider->sliderPosition();
    if(max==0 && val==0)
        return;
    if(val==max){
        val = val-1;
        ui->horizontalSlider->setValue(val);
    }
    ui->horizontalSlider->setMaximum(max-1);
    //lehet, hogy a slider allapota nem modosul ezert itt kulon allitanunk kell a
    //kepkockak kiirasaert felelos label-t
    ui->frameLabel->setText(QString("frame %1 / %2").arg(val).arg(max-1));
}

//ha a slider allapota valtozik, frissitjuk a label-t
void MainWindow::on_horizontalSlider_valueChanged(int val)
{
    int max = ui->horizontalSlider->maximum();
    ui->frameLabel->setText(QString("frame %1 / %2").arg(val).arg(max));
}

//animacio lejatszasanak inditasa/leallitasa
void MainWindow::on_playpauseButton_clicked()
{
    if(!playTimer->isActive()){
        playTimer->start(250);
        ui->playpauseButton->setText("Pause");
    }
    else{
        playTimer->stop();
        ui->playpauseButton->setText("Play");
    }
}

//animacio lejatszasakor a playTimer ezt a fgv-t hivja periodikusan, 0.25 masodpercenkent
void MainWindow::updateFrame()
{
    int val = ui->horizontalSlider->sliderPosition();
    int max = ui->horizontalSlider->maximum();
    if(val==max){
        if(ui->toggleLoop->isChecked())
            ui->horizontalSlider->setValue(0);  //loop modban folyamatos lejatszas
        else{
            playTimer->stop();                  //egyebkent a vegen megall
            ui->playpauseButton->setText("Play");
        }
    }
    else
        ui->horizontalSlider->setValue(val+1);  //kepkocka leptetese
}

//mikrofon figyelo mod elinditasakor es befejezesekor (vagy az inditaskor hiba eseten)
//hivodik meg, kikapcsolja a kezelofeluletet ha a mikrofon figyelo mod aktiv
void MainWindow::slotFileChosen(bool chosen)
{
    if(!chosen){
        ui->actionMicrphone_monitor->setChecked(0);
        ui->menuButtons->setEnabled(true);
        ui->LEDmatrixWidget->setEnabled(true);
        ui->animToolWidget->setEnabled(true);
    }
    else {
        ui->liveView->setChecked(0);
        ui->menuButtons->setEnabled(false);
        ui->LEDmatrixWidget->setEnabled(false);
        ui->animToolWidget->setEnabled(false);
    }
}
