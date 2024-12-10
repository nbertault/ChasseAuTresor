#include "chasseautresorserveurwidget.h"
#include "ui_chasseautresorserveurwidget.h"

ChasseAuTresorServeurWidget::ChasseAuTresorServeurWidget(const int _taille, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChasseAuTresorServeurWidget)
    , tresor(-1,-1)
    , TAILLE(_taille)
    , socketEcoute(new QTcpServer(this))
{
    ui->setupUi(this);
    tresor = DonnerPositionUnique();

    // connexion du signal newConnection au slot correspondant
    connect(socketEcoute, &QTcpServer::newConnection,this,&ChasseAuTresorServeurWidget::onQTcpServerNewConnection);
}

ChasseAuTresorServeurWidget::~ChasseAuTresorServeurWidget()
{
    delete ui;

    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if (clientSocket) {
        listeSocketsClient.removeAll(clientSocket);
        clientSocket->deleteLater();
    }
}

void ChasseAuTresorServeurWidget::onQTcpServerNewConnection()
{
    QTcpSocket *clientSocket = socketEcoute->nextPendingConnection();

    listeSocketsClient.append(clientSocket);
    listePositions.append(DonnerPositionUnique());

    EnvoyerDonnees(clientSocket,listePositions.last(),"start");
    qDebug() << "Client connecté en : " << clientSocket->peerAddress().toString() << clientSocket->peerPort();

    connect(clientSocket, &QTcpSocket::disconnected,this,&ChasseAuTresorServeurWidget::onQTcpSocketDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead,this,&ChasseAuTresorServeurWidget::onQTcpSocketReadyRead);
    connect(clientSocket, &QTcpSocket::errorOccurred,this,&ChasseAuTresorServeurWidget::onQTcpSocketErrorOccured);
}

void ChasseAuTresorServeurWidget::onQTcpSocketDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        listeSocketsClient.removeAll(clientSocket);
        clientSocket->deleteLater();
        listePositions.removeLast();
        qDebug() << "Client déconnecté en : " << clientSocket->peerAddress().toString() << clientSocket->peerPort();
    }
}

void ChasseAuTresorServeurWidget::onQTcpSocketReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    qsizetype position = listeSocketsClient.indexOf(clientSocket);
    QPoint positionActuelle = listePositions.at(position);
    qint16 taille;
    QString message;

    QDataStream entree(clientSocket);
    entree >> taille;
    entree << message;
    qDebug() << taille << message;

    if(message == 'U' && positionActuelle.x()-1 != -1) {
        positionActuelle.setX(positionActuelle.x()-1);
    }

    if(message == 'D' && positionActuelle.x()+1 != 20) {
        positionActuelle.setX(positionActuelle.x()+1);
    }

    if(message == 'L' && positionActuelle.y()-1 != -1) {
        positionActuelle.setY(positionActuelle.y()-1);
    }

    if(message == 'R' && positionActuelle.y()+1 != 20) {
        positionActuelle.setY(positionActuelle.y()+1);
    }

}

void ChasseAuTresorServeurWidget::onQTcpSocketErrorOccured(QAbstractSocket::SocketError socketError)
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        qDebug() << "Erreur avec le client :" << clientSocket->errorString();;
    }
}

void ChasseAuTresorServeurWidget::on_pushButtonLancementServeur_clicked()
{
    if(!socketEcoute->listen(QHostAddress::Any, ui->spinBoxPort->value()))
        qDebug() << "Echec au lancement du serveur : " << socketEcoute->errorString();
    else
        qDebug() << "Serveur lancé sur le port : "<< QString::number(ui->spinBoxPort->value());
}

void ChasseAuTresorServeurWidget::EnvoyerDonnees(QTcpSocket *_client, QPoint _pt, QString _msg)
{
    QBuffer tampon;
    QDataStream sortie(&tampon);
    quint16 taille = 0;

    sortie << taille << _pt << _msg << CalculerDistance(_pt);
    taille = tampon.size() - sizeof(taille);
    tampon.seek(0);
    sortie << taille;
    _client->write(tampon.buffer());
}

QPoint ChasseAuTresorServeurWidget::DonnerPositionUnique()
{
    QRandomGenerator gen;
    gen.seed(QDateTime::currentMSecsSinceEpoch());
    int ligne, colonne;
    QPoint positionUnique;
    do {
        ligne = gen.bounded(TAILLE);
        colonne = gen.bounded(TAILLE);
        positionUnique = QPoint(colonne, ligne);
    } while (listePositions.contains(positionUnique) && positionUnique != tresor);
    return positionUnique;
}

double ChasseAuTresorServeurWidget::CalculerDistance(QPoint _pos)
{
    double distance;
    int xVecteur = tresor.x() - _pos.x();
    int yVecteur = tresor.y() - _pos.y();
    distance = sqrt((xVecteur*xVecteur + yVecteur*yVecteur));
    return distance;
}

