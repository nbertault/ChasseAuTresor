#ifndef CHASSEAUTRESORCLIENTWIDGET_H
#define CHASSEAUTRESORCLIENTWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QTcpSocket>
#include <QToolButton>
#include <QBuffer>

QT_BEGIN_NAMESPACE
namespace Ui { class ChasseAuTresorClientWidget; }
QT_END_NAMESPACE

class ChasseAuTresorClientWidget : public QWidget
{
    Q_OBJECT

public:
    ChasseAuTresorClientWidget(const int _taille = 20, QWidget *parent = nullptr);
    ~ChasseAuTresorClientWidget();

private slots:
    void onQTcpSocketConnected();
    void onQTcpSocketDisconnected();
    void onQTcpSocketReadyRead();
    void onQTcpSocketErrorOccured(QAbstractSocket::SocketError socketError);

    void on_pushButtonConnexion_clicked();
    void on_pushButtonUp_clicked();
    void on_pushButtonLeft_clicked();
    void on_pushButtonRight_clicked();
    void on_pushButtonDown_clicked();

private:
    void EnvoyerCommande(QChar _commande);

    Ui::ChasseAuTresorClientWidget *ui;
    const int TAILLE;
    QGridLayout *grille;


    QPoint pointPrec; // position précédente sur la grille
    char commandeEnvoyee; // dernière commande envoyée au serveur

    QTcpSocket *socket;
};
#endif // CHASSEAUTRESORCLIENTWIDGET_H
