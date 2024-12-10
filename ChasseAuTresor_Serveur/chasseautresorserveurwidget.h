#ifndef CHASSEAUTRESORSERVEURWIDGET_H
#define CHASSEAUTRESORSERVEURWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

#include <QRandomGenerator>
#include <QDateTime>

#include <QBuffer>


QT_BEGIN_NAMESPACE
namespace Ui { class ChasseAuTresorServeurWidget; }
QT_END_NAMESPACE

class ChasseAuTresorServeurWidget : public QWidget
{
    Q_OBJECT

public:
    ChasseAuTresorServeurWidget(const int _taille = 20, QWidget *parent = nullptr);
    ~ChasseAuTresorServeurWidget();

private slots:
    void onQTcpServerNewConnection();
    void onQTcpSocketDisconnected();
    void onQTcpSocketReadyRead();
    void onQTcpSocketErrorOccured(QAbstractSocket::SocketError socketError);

    void on_pushButtonLancementServeur_clicked();

private:

    void EnvoyerDonnees(QTcpSocket * _client, QPoint _pt, QString _msg);
    QPoint DonnerPositionUnique();
    double CalculerDistance(QPoint _pos);

    Ui::ChasseAuTresorServeurWidget *ui;
    QTcpServer *socketEcoute;

    QList <QTcpSocket *> listeSocketsClient;
    QList <QPoint> listePositions;
    QPoint tresor;

    const int TAILLE;
};
#endif // CHASSEAUTRESORSERVEURWIDGET_H
