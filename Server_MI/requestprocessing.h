#ifndef REQUESTPROCESSING_H
#define REQUESTPROCESSING_H


#include <QRunnable>
#include <QTcpSocket>
#include <QTcpServer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QSql>
#include <request.h>
#include <getrequesthandler.h>
#include <postrequesthandler.h>
class RequestProcessing : public QRunnable
{
public:
    RequestProcessing(qintptr socket_id, QSqlDatabase* db, QString apiKey, QString bazeUrl);
    ~RequestProcessing();
    void run();
private:
    QString GetUri(const QString& body_http);
    void Responce();
    void GetRequest();
    void PostRequest();
private:
    qintptr _socket_id;
    QTcpSocket* _socket;
    QSqlDatabase* _db;
    Request* _request;
    QString _apiKey;
    QString _bazeUrl;
};

#endif // REQUESTPROCESSING_H
