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
private:
    QSqlDatabase* DB_;
    Request* Request_;
};

#endif // POSTREQUESTHANDLER_H
