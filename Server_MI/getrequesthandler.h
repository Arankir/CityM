#ifndef GETREQUESTHANDLER_H
#define GETREQUESTHANDLER_H


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <request.h>



class GetRequestHandler
{
public:
    GetRequestHandler(QSqlDatabase* db, Request* request);
    QString stationHandler();
    QString priceHandler();
    QString pingHandler(QString station, QString column);
    QString authHandler();
    QString invoiceHandler();
    QString wkInfoHandler();
    QString guestCountHandler();

    QString authTableHandler();
    QString dishTableHandler();
    QString guestsTableHandler();
    QString ingredientsTableHandler();
    QString ingredients_dishTableHandler();
    QString ingredients_stockHandler();
    QString invoiceTableHandler();
    QString invoice_stockHandler();
    QString menuTableHandler();
    QString menu_dishTableHandler();
    QString roleTableHandler();
    QString stockTableHandler();
    QString type_menuTableHandler();

private:
    QSqlDatabase* _db;
    Request* Request_;
};

#endif // GETREQUESTHANDLER_H
