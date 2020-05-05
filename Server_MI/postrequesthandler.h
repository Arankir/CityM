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
    QString newIngredientHandler(QString post);
    QString newStockHandler(QString post);
    QString newDishHandler(QString post);
    QString newGuestHandler(QString post);
    QString updateGuestHandler(QString post);
    QString addMenuHandler(QString post);
    QString addInvoiceHandler(QString post);

    QString deleteTableHandler(QString post);
    QString deleteByIdHandler(QString post);
    QString deleteByWhereHandler(QString post);
    QString insertHandler(QString post);
    QString updateByIdHandler(QString post);
    QString updateByWhereHandler(QString post);
private slots:
    int GetFuelID(QString a_fuelIdAPI);
    int ConformityPrice(QString fuel, QString price, QString price1, QString price2, QString price3, QString price4, QString price5, QString price6, QString price7, QString price8, QString price9,
                        QString price10, QString price11, QString price12, QString price13);
    void SmashData(QString data, int* sideAdress, int* nozzle, int* trkFuelCode, int* trkVCode);
    void SmashMoneyData(QJsonObject data, int* requestTotalPriceDB, int* requestVolumeDB, int* requestUnitPriceDB, int* moneyTakenDB, int* fullTankDB);
private:
    QSqlDatabase* DB_;
    Request* Request_;
};

#endif // POSTREQUESTHANDLER_H
