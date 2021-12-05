#include "nethandler.h"

NetHandler::NetHandler(QObject *parent) :
    QObject(parent)
{
    m_pSocket = NULL;
}

NetHandler::~NetHandler()
{
    disconnect(this, SLOT(slotDisconnected()));

    if(m_pSocket) delete m_pSocket;
}

// Kapcsolodas a szerverhez.
void NetHandler::Connect(QString addr)
{
    // Ha letezik mar a socket, visszateres
    if (m_pSocket) return;

    // Letrehozzuk a socket-et.
    m_pSocket = new QTcpSocket();

    // Jelezzuk a felhasznalo fele, hogy eppen kapcsolodni probalunk.
    emit signalConnectionStatus(Connecting);

    // Bekotjuk a kapcsolat bontasanak erzekeleset. Amikor a masik oldal lezarja
    // a kapcsolatot, akkor a kliens socket egy disconnected() szignalt general.
    connect(m_pSocket, SIGNAL(disconnected()),
            this, SLOT(slotDisconnected()));


    // A connect() fuggveny meghivasaval kapcsolodunk a szerverhez.
    m_pSocket->connectToHost(addr, 8083);
    if (!m_pSocket->waitForConnected(5000)) {
        delete m_pSocket;
        m_pSocket = NULL;
        //Ha 5 mp-ig nem sikerul csatlakozni
        emit signalConnectionStatus(Error);
        return;
    }

    // Jelezzuk hogy sikerult csatlakozni
    emit signalConnectionStatus(Connected);

}

// A kapcsolat lezarasanak erzekelese.
// Ha lezartak a kapcsolatot, akkor a szignal hatasara ez a fuggveny hivodik
// meg.
void NetHandler::slotDisconnected()
{
    // Kikotjuk a socket lezarasanak erzekeleset, mert kulonben a sajatunkat is
    // erzekelnenk.
    disconnect(this, SLOT(slotDisconnected()));

    // Ha letezik meg kliens socket akkor lezarjuk.
    if (m_pSocket) {
        m_pSocket->deleteLater();
        m_pSocket = NULL;
    }

    // Jelezzuk a kapcsolat allapotanak valtozasat.
    emit signalConnectionStatus(Disconnected);

}

// Az adott frame elkuldese a Raspberry-nek (matrixWidget signalSend()-je hívja meg)
void NetHandler::slotSend(QByteArray b)
{
    // Ha nem letezne a kliens socket akkor visszater a fuggveny.
    if (!m_pSocket) return;

    // A matrixWidget::slotSendOut() -ban osszeallitott QByteArray elkuldese
    m_pSocket->write(b);
    m_pSocket->waitForBytesWritten();
    qDebug()<<"data sent out";
}

