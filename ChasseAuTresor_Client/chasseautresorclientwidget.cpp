#include "chasseautresorclientwidget.h"
#include "ui_chasseautresorclientwidget.h"

ChasseAuTresorClientWidget::ChasseAuTresorClientWidget(const int _taille, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChasseAuTresorClientWidget)
    , TAILLE(_taille)
    , socket(new QTcpSocket(this))
{
    ui->setupUi(this);
    QVBoxLayout *layoutPrincipal = ui->verticalLayout;

    // Création de la grille de jeu
    grille = new QGridLayout();
    for (int ligne = 0; ligne < TAILLE; ligne++) {
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            QToolButton *bouton = new QToolButton();
            grille->addWidget(bouton, ligne, colonne, 1, 1);
        }
    }
    layoutPrincipal->insertLayout(1,grille);
    layoutPrincipal->setContentsMargins(50, 20, 50, 20);

    // Désactivation de la rose des vents
    ui->pushButtonUp->setDisabled(true);
    ui->pushButtonDown->setDisabled(true);
    ui->pushButtonLeft->setDisabled(true);
    ui->pushButtonRight->setDisabled(true);

    // Connexions des signaux et slots liés à la communication réseau.
    connect(socket,&QTcpSocket::connected, this,&ChasseAuTresorClientWidget::onQTcpSocketConnected);
    connect(socket,&QTcpSocket::disconnected, this,&ChasseAuTresorClientWidget::onQTcpSocketDisconnected);
    connect(socket,&QTcpSocket::readyRead, this,&ChasseAuTresorClientWidget::onQTcpSocketReadyRead);
    connect(socket,&QTcpSocket::errorOccurred, this,&ChasseAuTresorClientWidget::onQTcpSocketErrorOccured);

}

ChasseAuTresorClientWidget::~ChasseAuTresorClientWidget()
{
    if (socket) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected();
        }
        delete socket;
        socket = nullptr;
    }
    delete ui;
}

void ChasseAuTresorClientWidget::EnvoyerCommande(QChar _commande)
{
    QBuffer tampon;
    QDataStream out(&tampon);
    int taille = 0;
    if(socket->state() == QAbstractSocket::ConnectedState && tampon.open(QIODevice::WriteOnly))
    {
        out << taille << _commande;
        taille = tampon.size() - sizeof(taille);
        tampon.seek(0);
        out << taille;
        socket->write(tampon.buffer());
        qDebug() << tampon.buffer();
    }
    else
        qDebug() << "Le message ne peut pas être envoyé au serveur";
}

void ChasseAuTresorClientWidget::onQTcpSocketConnected()
{
    ui->pushButtonConnexion->setText("Déconnexion");
    ui->pushButtonUp->setDisabled(false);
    ui->pushButtonDown->setDisabled(false);
    ui->pushButtonLeft->setDisabled(false);
    ui->pushButtonRight->setDisabled(false);
    qDebug() << "Connecté au serveur avec le port : " << ui->spinBoxPortServeur->value();
}

void ChasseAuTresorClientWidget::onQTcpSocketDisconnected()
{
    ui->pushButtonConnexion->setText("Connexion");
    ui->pushButtonUp->setDisabled(true);
    ui->pushButtonDown->setDisabled(true);
    ui->pushButtonLeft->setDisabled(true);
    ui->pushButtonRight->setDisabled(true);
    qDebug() << "Déconnecté du serveur";
}

void ChasseAuTresorClientWidget::onQTcpSocketReadyRead()
{
    qint16 taille;
    int coordonneeX, coordonneeY;
    QPoint coordonnees;
    QString message;
    double distanceTresor;

    if(socket->bytesAvailable() >= static_cast<qint64>(sizeof(taille)))
    {
        QDataStream entree(socket);
        entree >> taille;
        entree >> coordonneeX >> coordonneeY;
        entree >> message;
        entree >> distanceTresor;
        coordonnees.setX(coordonneeX);
        coordonnees.setY(coordonneeY);
        qDebug() << "Message reçus :" << taille << coordonnees << message << distanceTresor;
    }

    if(pointPrec.y() != -1 && pointPrec.x() != -1) {
        QLayoutItem *item = grille->itemAtPosition(pointPrec.y(), pointPrec.x());
        item->widget()->setStyleSheet("background-color: white");
    }

    if(coordonnees.y() != -1 && coordonnees.x() != -1) {
        QLayoutItem *item = grille->itemAtPosition(coordonnees.y(), coordonnees.x());
        item->widget()->setStyleSheet("background-color: black");
        pointPrec.setY(coordonnees.y());
        pointPrec.setX(coordonnees.x());
    }

    if( message == "collision") {
        ui->labelInformations->setStyleSheet("color : magenta;");
        ui->labelInformations->setText(message);
    }

    if(message.left(11) == "Victoire de") {

        QString gagnant = message.mid(12);
        QString monIdentifiant = QString("%1:%2")
                                     .arg(socket->localAddress().toString())
                                     .arg(socket->localPort()); // Identifiant du client

        if (gagnant == monIdentifiant)
        {
            ui->labelInformations->setStyleSheet("color : green;");
            ui->labelInformations->setText(message);
            // traitement lorsque la partie est gagnée
            QLayoutItem *item = grille->itemAtPosition(coordonnees.y(), coordonnees.x());
            item->widget()->setStyleSheet("background-color: green");

        }
        else
        {
            ui->labelInformations->setStyleSheet("color : red;");
            ui->labelInformations->setText(message);
            // traitement lorsque la partie est gagnée par un autre client
        }
    }
    ui->lcdNumberDistance->display(distanceTresor);
}

void ChasseAuTresorClientWidget::onQTcpSocketErrorOccured(QAbstractSocket::SocketError socketError)
{
    QString messageErreur;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        messageErreur = "Hôte non trouvé. Vérifier l'adresse du serveur et le port";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        messageErreur = "Connexion refusée par le serveur";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        messageErreur = "L'hôte distant a fermé la connexion.";
        break;
    default:
        messageErreur = "Erreur inattendue : " + socket->errorString();
        break;
    }
    qDebug() << messageErreur;
}

void ChasseAuTresorClientWidget::on_pushButtonConnexion_clicked()
{
    if(ui->pushButtonConnexion->text() == "Connexion")
    {
        // Connexion au serveur
        socket->connectToHost(ui->lineEditAdresseServeur->text(),ui->spinBoxPortServeur->value());

        // Réinitialisation de la grille
        for (int ligne = 0; ligne < TAILLE; ++ligne)
        {
            for (int colonne = 0; colonne < TAILLE; ++colonne)
            {
                QWidget *widget = grille->itemAtPosition(ligne, colonne)->widget();
                if (widget)
                    widget->setStyleSheet("");
            }
        }
        pointPrec.setX(-1);
        pointPrec.setY(-1);
    }
    else
    {
        //deconexion du serveur
        socket->disconnectFromHost();
    }
}

void ChasseAuTresorClientWidget::on_pushButtonUp_clicked()
{
    EnvoyerCommande('U');
    commandeEnvoyee = 'U';
}

void ChasseAuTresorClientWidget::on_pushButtonLeft_clicked()
{
    EnvoyerCommande('L');
    commandeEnvoyee = 'L';
}

void ChasseAuTresorClientWidget::on_pushButtonRight_clicked()
{
    EnvoyerCommande('R');
    commandeEnvoyee = 'R';
}

void ChasseAuTresorClientWidget::on_pushButtonDown_clicked()
{
    EnvoyerCommande('D');
    commandeEnvoyee = 'D';
}






