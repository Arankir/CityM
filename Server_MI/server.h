#ifndef SERVER_H
#define SERVER_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QFile>
#include <QThreadPool>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <requestprocessing.h>
class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);
    ~Server();
    void incomingConnection(qintptr socketDescriptor);
public slots:
    void StartServer();
private:
    std::unique_ptr<QThreadPool> ThreadPool;
    //QThreadPool* ThreadPool;
    QSqlDatabase _db;

    QString _apiKey="dee7415fc86c414eb2fe205acd2b6b56";
    QString _bazeUrl;
};

#endif // SERVER_H
