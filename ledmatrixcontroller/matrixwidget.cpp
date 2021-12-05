#include "matrixwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QtNetwork>

matrixWidget::matrixWidget(QWidget *parent) : QWidget(parent)
{
    init();
    //mikrofon allapotanak lekereseert felelo timer
    checkMicTimer = new QTimer(this);

    //mikrofon allapotat lekero bash process letrehozasa:
    proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->QProcess::start("/bin/bash", QStringList());
    proc->waitForStarted();

    //a timer 1 masodpercenkent meghivja az allapot lekero fgv-t
    connect(checkMicTimer, SIGNAL(timeout()), this, SLOT(slotCheckMic()));
}

void matrixWidget::init()
{
    //0. frame létrehozása
    currentFrame=0;
    animFrameBuffer.resize(1);
    animFrameBuffer[0].resize(32);
    for(int i=0; i<32; i++){
        animFrameBuffer[0][i].resize(8);
    }

    //ha elinditanank a mikrofon figyelo modot, ez legyen a kezdoallapot
    LED_state=START;
}

//minden ledet kikapcsol a kijelzon
void matrixWidget::slotClear()
{
    for(int x=0; x<32; x++)
        {
            for(int y=0; y<8; y++)
            {
                animFrameBuffer[currentFrame][x][y]=0;
            }
        }
    update();
}

//minden ledet invertal a kijelzon
void matrixWidget::slotInvert()
{
    for(int x=0; x<32; x++)
    {
        for(int y=0; y<8; y++)
        {
            animFrameBuffer[currentFrame][x][y]=(animFrameBuffer[currentFrame][x][y]+1)%2;
        }
    }
    update();
}

//eppen megjelenitett kepkocka fajlba mentese
void matrixWidget::slotSave()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save LED Screen Preset"), "",
            tr("LED Screen (*.lds);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        QTextStream stream(&file);
        for(int x=0; x<32; x++)
        {
            for(int y=0; y<8; y++)
            {
                stream << animFrameBuffer[currentFrame][x][y];
            }
        }
    }
}

//fajlba mentett kepkocka megnyitasa
void matrixWidget::slotOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
           tr("Open LED Screen Preset"), "",
           tr("LED Screen (*.lds);;All Files (*)"));
    if (fileName.isEmpty())
            return;
        else {

            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }
            QTextStream in(&file);
            for(int x=0; x<32; x++)
            {
                for(int y=0; y<8; y++)
                {
                    animFrameBuffer[currentFrame][x][y]=in.read(1).toInt();
                }
            }
    }
}

//az adatkuldest elokeszito, majd az adatkuldo fgv-t meghivo fgv
//meghivodik, ha:
// - Send to Screen gombra kattintunk
// - Mikrofon figyelo valtozast eszlel
// - Live View modban vagyunk (ekkor periodikusan)
void matrixWidget::slotSendOut()
{
    // a kuldendo adatszerkezet letrehozasa az eppen aktiv kepkockabol
    // a LED kijelzo dokumentacio szerint a kijelzes mukodese:
        // 1 db 8x8-as kijelzon:
        // 2 bajt kikuldesevel az elso bajt egy adott sorhoz tartozo regisztert jelol ki
        // a masodik bajt pedig bitenkent megadja a sorban bekapcsolt LED-eket
        // pl: 0x02 0xFF -> a masodik sorban az osszes LED vilagit
        // mivel 4 db ilyen kijelzo van kaszkadositva, ezert 4 bajt part (8 bajt)
        // kell kikuldeni egy sor beallitasahoz
    int i=0;
    for(int row=1; row<9; row++){
        for(int col=0; col<4; col++){
            dataOut[i]=row;//regisztercim
            i++;
            dataOut[i]=animFrameBuffer[currentFrame][col*8][row-1];
            for(int j=1; j<8; j++){//beleshifteljük a 8 bitet
                dataOut[i]=dataOut[i]<<1;
                dataOut[i]=dataOut[i]+animFrameBuffer[currentFrame][col*8+j][row-1];
            }
            i++;
        }
    }

    QByteArray buf(64, 0);
    for (int i=0; i<64; i++){
        buf[i]=dataOut[i];
    }

    qDebug()<<"data package created";

    //meghivjuk a NetHandler adatkuldo fgv-et, es atadjuk a kuldendo adatot
    emit signalSend(buf);
}

//az animacio kezeleset segito slider ertekenk megvaltozasakor frissiti a kepet
void matrixWidget::slotSliderValChanged(int i)
{
    //hibakezeles
    if(i<0 || animFrameBuffer.size()<i)
        return;

    //ha az új értékhez még nincs frame, létrehozunk egy ureset
    //ez akkor tortenhet, mikor az utolso kepkockan allva rakattintunk a Next frame gombra
    if(animFrameBuffer.size()==i){
        QVector<QVector<int>> temp;
        temp.resize(32);
        for(int j=0; j<32; j++){
            temp[j].resize(8);
        }
        animFrameBuffer.append(temp);
    }

    //a kijelzett frame indexenek beallitasa es kep frissitese
    currentFrame = i;
    update();
}

//egy kepkocka kitorlese
void matrixWidget::slotDeleteFrame()
{
    //ha csak 1 kepkocka van, nem tudunk torolni
    if(animFrameBuffer.size()!=1){
        animFrameBuffer.remove(currentFrame);
        update();
    }
}

//kepkocka masolasa
void matrixWidget::slotCopyFrame()
{
    copiedFrame.resize(32);
    for(int i=0; i<32; i++){
        copiedFrame[i].resize(8);
    }

    for(int i=0; i<32; i++){
        for(int j=0; j<8; j++){
            copiedFrame[i][j]=animFrameBuffer[currentFrame][i][j];
        }
    }
}

//kepkocka beillesztese
void matrixWidget::slotPasteFrame()
{
    if(copiedFrame.size()!=0){
        for(int i=0; i<32; i++){
            for(int j=0; j<8; j++){
                animFrameBuffer[currentFrame][i][j]=copiedFrame[i][j];
            }
        }
    }
    update();
}

//a menu Microphone monitor menupontjara kattintva hivodik meg
//kivalaszthatjuk azt a kepet amit meg szeretnenk jeleniteni,
//ha a mikrofon aktiv, illetve elinditja a mikrofon figyelo timert
void matrixWidget::slotMic(bool toggled)
{
    if(toggled){
        QString fileName = QFileDialog::getOpenFileName(this,
               tr("Open File to display when Mic is active"), "",
               tr("LED Screen (*.lds);;All Files (*)"));
        if (fileName.isEmpty()){
            //ha nem valasztunk fajlt, a mikrofon figyeles nem indul el
            //ezt jelezni kell mainwindow-nak, hogy a menuopciok ne legyenek kikapcsolva
            emit signalFileChosen(false);
            return;
        }
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                                         file.errorString());
                emit signalFileChosen(false);
                return;
            }

            //feltoltjuk a kepkockat a beolvasott adatokkal
            QTextStream in(&file);

            micDisplayFrame.resize(32);
            for(int i=0; i<32; i++){
                micDisplayFrame[i].resize(8);
            }

            for(int x=0; x<32; x++)
            {
                for(int y=0; y<8; y++)
                {
                    micDisplayFrame[x][y]=in.read(1).toInt();
                }
            }
            //ha a fajl betoltese sikeres, elinditjuk a figyelo timert,
            //es jelzunk, hogy a menu gombok legyenek letiltva
            emit signalFileChosen(true);
            LED_state = START;
            checkMicTimer->start(1000);
        }
    }

    else{
        //ha a gombot kikapcsoljuk, visszaallitjuk kezdoallapotba es kikapcsoljuk a figyelest
        LED_state = START;
        checkMicTimer->stop();
        emit signalFileChosen(false);   //hogy ismet elerhetoek legyenek a menu gombok
        qDebug() << "microphone check STOP";
    }
}

void matrixWidget::slotCheckMic()
{
    qDebug() << "checking microphone";
    //ez a command visszaadja az adott fajlt (esetunkben a mikrofon device fajlja)
    //hasznalo process PID-jet, ha van ilyen process
    //ha nincs, tehat a mikrofont nem hasznaljuk, nem ad vissza semmit
    proc->write("fuser /dev/snd/pcmC0D0c\n");
    QString output;
    output = proc->readAllStandardOutput();
    //ha nem hasznaljuk a mikrofont, de most indult a figyeles, vagy ki van jelezve hogy hasznaljuk
    //akkor a kijelzot le kell torolni
    if(output == "" && (LED_state == START || LED_state==DISPLAYED)){
        qDebug() << "CLOSED";
        for(int i=0; i<32; i++){
            for(int j=0; j<8; j++){
                animFrameBuffer[currentFrame][i][j]=0;
            }
        }
        update();
        slotSendOut();
        LED_state = CLEARED;
    }
    //ha hasznaljuk a mikrofont, de a figyeles most indult vagy a kijelzo le volt torolve,
    //akkor ki kell jelezni a beallitott frame-et
    else if(output != "" && (LED_state == START || LED_state == CLEARED)){
        qDebug() << "OPEN";
        if(micDisplayFrame.size()!=0){
            for(int i=0; i<32; i++){
                for(int j=0; j<8; j++){
                    animFrameBuffer[currentFrame][i][j]=micDisplayFrame[i][j];
                }
            }
        }
        update();
        slotSendOut();
        LED_state = DISPLAYED;
    }
}

//a LED feluleten torteno rajzolashoz az animFrameBuffer megfelelo bitjeit
//atallitja
void matrixWidget::mouseMoveEvent(QMouseEvent *e)
{
    QRect rect=geometry();
    int w=rect.width();
    int h=rect.height();
    int bw=w/32;
    int bh=h/8;

    int x=e->x();
    int y=e->y();
    int bx=x/bw;
    int by=y/bh;
    if(0<=bx && bx<32 && 0<=by && by<8){
        //balklikkel bekapcsoljuk a LED-et
        if(e->buttons()==Qt::LeftButton){
            animFrameBuffer[currentFrame][bx][by]=1;
        }
        //jobbklikkel kikapcsoljuk
        else if(e->buttons()==Qt::RightButton){
            animFrameBuffer[currentFrame][bx][by]=0;
        }
        update();
    }
}

//az animFrameBuffer GUI-ra torteno kirajzolasat megvalosito fgv
void matrixWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QRect rect = geometry();
    int w = rect.width();
    int h = rect.height();
    int bw = w / 32;
    int bh = h / 8;
    QPainter p(this);

    QPen OFFpen(Qt::gray, 2);
    QBrush OFFbrush(Qt::black);
    QBrush Backbrush(Qt::darkGray);
    QPen ONpen(Qt::gray, 1);
    QBrush ONbrush(Qt::red);
    p.fillRect(rect, Backbrush);
    for(int x=0; x<32; x++)
    {
        for(int y=0; y<8; y++)
        {
            if(animFrameBuffer[currentFrame][x][y]==1){
                p.setBrush(ONbrush);
                p.setPen(ONpen);
                p.drawEllipse(x*bw+10,y*bh+10,bw-10,bh-10);
            }
            else{
                p.setPen(OFFpen);
                p.setBrush(OFFbrush);
                p.drawEllipse(x*bw+10,y*bh+10,bw-10,bh-10);
            }
        }
    }
}
