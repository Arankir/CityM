#ifndef POSTREQUESTHANDLER_H
#define POSTREQUESTHANDLER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <request.h>
#include <QDateTime>

class PostRequestHandler
{
public:
    PostRequestHandler(QSqlDatabase* db, Request* request);
    QString orderHandler(QString post);
//    QString newIngredientHandler(QString post);
//    QString newStockHandler(QString post);
//    QString newDishHandler(QString post);
//    QString newGuestHandler(QString post);
//    QString updateGuestHandler(QString post);
//    QString addMenuHandler(QString post);
//    QString addInvoiceHandler(QString post);

//    QString deleteTableHandler(QString post);
//    QString deleteByIdHandler(QString post);
//    QString deleteByWhereHandler(QString post);
//    QString insertHandler(QString post);
//    QString updateByIdHandler(QString post);
//    QString updateByWhereHandler(QString post);
private slots:
    int GetFuelID(QString a_fuelIdAPI);
    int ConformityPrice(QString fuel, QString price, QString diesel_price, QString diesel_premium_price, QString a80_price, QString a92_price, QString a92_premium_price, QString a95_price,
                        QString a95_premium_price, QString a98_price, QString a98_premium_price, QString a100_price, QString a100_premium_price, QString propane_price, QString metan_price);
    void SmashData(QString data, int* sideAdress, int* nozzle, int* trkFuelCode, int* trkVCode);
    void SmashMoneyData(QJsonObject data, int* requestTotalPriceDB, int* requestVolumeDB, int* requestUnitPriceDB, int* moneyTakenDB, int* fullTankDB);
    QString GetSmena(QString l, QString p);
private:
    QSqlDatabase* DB_;
    Request* Request_;
};

#endif // POSTREQUESTHANDLER_H
