#include "getrequesthandler.h"

GetRequestHandler::GetRequestHandler(QSqlDatabase *db, Request *request):_db(db),Request_(request){
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

QString GetRequestHandler::stationHandler(){
//[
//    {
//        "Id": "1234",
//        "Enable": true,
//        "Name": "АЗС \"На Мандариновой\" №10",
//        "Address": "г. Энск, ул. Мандариновая, 4",
//        "Location": { "Lon": 43.5432, "Lat": 34.1234 },
//        "Columns": {
//            "1": {
//                "Fuels": ["a92", "a95", "a95_premium", "diesel_premium"]
//            },
//            "2": {
//                "Fuels": ["a92", "a95", "a95_premium", "a100_premium"]
//            }
//        },
//        "FuelNames": [
//        {
//            "Fuel": "a92",
//            "FuelName": "АИ-92"
//        },
//        {
//            "Fuel": "a95",
//            "FuelName": "АИ-95"
//        },
//        {
//            "Fuel": "a95_premium",
//            "FuelName": "G-95"
//        },
//        {
//            "Fuel": "a100_premium",
//            "FuelName": "G-100"
//        },
//        {
//            "Fuel": "diesel_premium",
//            "FuelName": "ДТ Опти"
//        }
//        ]
//    }
//]

    QJsonArray AGZSs;
    QSqlQuery* q = new QSqlQuery(*_db);
    if (q->exec("SELECT AGZSName, AGZS, VCode, Id, Adress, Location_x, Location_y, ColumnsCount, AGZSL, AGZSP FROM [agzs].[dbo].PR_AGZSData")) {
        while (q->next()) {
            QJsonObject AGZS;
            AGZS["Id"] = q->value(3).toString();

            QSqlDatabase dbLocal = QSqlDatabase::addDatabase("QODBC3");
            QStringList setting;
            if(QFile::exists("Setting.txt")){
                QFile settings("Setting.txt");
                if (settings.open(QIODevice::ReadOnly)){
                    while(!settings.atEnd()){
                        setting << QString::fromLocal8Bit(settings.readLine()).remove("\r\n").remove("\n");
                    }
                    settings.close();
                } else
                    qDebug()<<"Error: setting file is already open";
            } else
                qDebug()<<"Error: setting file not found";
            dbLocal.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(q->value(8).toString(),setting[1],setting[2],q->value(9).toString()));
            if(dbLocal.open()){
                AGZS["Enable"] = "true";
            }
            else {
                AGZS["Enable"] = "false";
            }

            AGZS["Name"] = q->value(0).toString();
            AGZS["Adress"] = q->value(4).toString();
            QJsonObject Location;
            Location["Lon"] = q->value(6).toString().replace(",",".");
            Location["Lat"] = q->value(5).toString().replace(",",".");
            AGZS["Location"] = Location;
            //Columns
            QJsonObject Columns;
            //FuelNames
            QJsonArray FuelNames;
            bool FuelsInc[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
            QSqlQuery* q2 = new QSqlQuery(*_db);
            q2->exec("SELECT d.AGZSName,d.VCode,c.ColumnNumber,c.diesel,c.diesel_premium,c.a80,c.a92,c.a92_premium,c.a95,c.a95_premium,c.a98,c.a98_premium, "
                     "c.a100,c.a100_premium,c.propane,c.metan, c.SideAdress, c.Nozzle, c.TrkFuelCode, c.TrkVCode "
                     "FROM [agzs].[dbo].[PR_AGZSColumnsData] c INNER JOIN [agzs].[dbo].PR_AGZSData d ON d.VCode = c.Link WHERE (((d.VCode)="+q->value(2).toString()+")) order by c.ColumnNumber asc");
            while (q2->next()) {
                QJsonObject Column;
                QJsonArray Fuels;
                if(q2->value(3).toInt()>0){
                    Fuels.append("diesel");
                    if(!FuelsInc[0]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="diesel";
                        FuelName["FuelName"]="ДТ";
                        FuelNames.append(FuelName);
                        FuelsInc[0]=true;
                    }
                }
                if(q2->value(4).toInt()>0){
                    Fuels.append("diesel_premium");
                    if(!FuelsInc[1]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="diesel_premium";
                        FuelName["FuelName"]="ДТ Премиум";
                        FuelNames.append(FuelName);
                        FuelsInc[1]=true;
                    }
                }
                if(q2->value(5).toInt()>0){
                    Fuels.append("a80");
                    if(!FuelsInc[2]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a80";
                        FuelName["FuelName"]="АИ-80";
                        FuelNames.append(FuelName);
                        FuelsInc[2]=true;
                    }
                }
                if(q2->value(6).toInt()>0){
                    Fuels.append("a92");
                    if(!FuelsInc[3]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a92";
                        FuelName["FuelName"]="АИ-92";
                        FuelNames.append(FuelName);
                        FuelsInc[3]=true;
                    }
                }
                if(q2->value(7).toInt()>0){
                    Fuels.append("a92_premium");
                    if(!FuelsInc[4]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a92_premium";
                        FuelName["FuelName"]="АИ-92 Премиум";
                        FuelNames.append(FuelName);
                        FuelsInc[4]=true;
                    }
                }
                if(q2->value(8).toInt()>0){
                    Fuels.append("a95");
                    if(!FuelsInc[5]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a95";
                        FuelName["FuelName"]="АИ-95";
                        FuelNames.append(FuelName);
                        FuelsInc[5]=true;
                    }
                }
                if(q2->value(9).toInt()>0){
                    Fuels.append("a95_premium");
                    if(!FuelsInc[6]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a95_premium";
                        FuelName["FuelName"]="АИ-95 Премиум";
                        FuelNames.append(FuelName);
                        FuelsInc[6]=true;
                    }
                }
                if(q2->value(10).toInt()>0){
                    Fuels.append("a98");
                    if(!FuelsInc[7]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a98";
                        FuelName["FuelName"]="АИ-98";
                        FuelNames.append(FuelName);
                        FuelsInc[7]=true;
                    }
                }
                if(q2->value(11).toInt()>0){
                    Fuels.append("a98_premium");
                    if(!FuelsInc[8]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a98_premium";
                        FuelName["FuelName"]="АИ-98 Премиум";
                        FuelNames.append(FuelName);
                        FuelsInc[8]=true;
                    }
                }
                if(q2->value(12).toInt()>0){
                    Fuels.append("a100");
                    if(!FuelsInc[9]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a100";
                        FuelName["FuelName"]="АИ-100";
                        FuelNames.append(FuelName);
                        FuelsInc[9]=true;
                    }
                }
                if(q2->value(13).toInt()>0){
                    Fuels.append("a100_premium");
                    if(!FuelsInc[10]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="a100_premium";
                        FuelName["FuelName"]="АИ-100 Премиум";
                        FuelNames.append(FuelName);
                        FuelsInc[10]=true;
                    }
                }
                if(q2->value(14).toInt()>0){
                    Fuels.append("propane");
                    if(!FuelsInc[11]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="propane";
                        FuelName["FuelName"]="Сжиженный газ";
                        FuelNames.append(FuelName);
                        FuelsInc[11]=true;
                    }
                }
                if(q2->value(15).toInt()>0){
                    Fuels.append("metan");
                    if(!FuelsInc[12]){
                        QJsonObject FuelName;
                        FuelName["Fuel"]="metan";
                        FuelName["FuelName"]="Метан";
                        FuelNames.append(FuelName);
                        FuelsInc[12]=true;
                    }
                }
                Columns[QString::number(q2->value(16).toInt()*100000+q2->value(17).toInt()*10000+q2->value(18).toInt()*1000+q2->value(19).toInt())]=Column;
            }
            delete q2;
            AGZS["Columns"]=Columns;
            AGZS["FuelNames"]=FuelNames;
            AGZSs.append(AGZS);
        }
    }
    QJsonDocument doc;
    doc.setArray(AGZSs);
    delete q;
    return QString(doc.toJson());
}

QString GetRequestHandler::priceHandler(){
//[
//    { "StationId": "1234", "ProductId": "a92", "Price": 42.50 },
//    { "StationId": "1234", "ProductId": "a95", "Price": 44.90 },
//    { "StationId": "1234", "ProductId": "a95_premium", "Price": 46.20 }
//]
    QJsonArray prices;
    QSqlQuery* q = new QSqlQuery(*_db);
    if (q->exec("SELECT AGZSName,Id,VCode,ColumnsCount,[diesel_price],[diesel_premium_price],[a80_price],[a92_price],[a92_premium_price],[a95_price],[a95_premium_price]"
                ",[a98_price],[a98_premium_price],[a100_price],[a100_premium_price],[propane_price],[metan_price] FROM [agzs].[dbo].PR_AGZSData")) {
        while (q->next()) {
            if(q->value(4).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="diesel";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(4).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(5).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="diesel_premium";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(5).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(6).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a80";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(6).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(7).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a92";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(7).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(8).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a92_premium";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(8).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(9).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a95";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(9).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(10).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a95_premium";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(10).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(11).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a98";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(11).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(12).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a98_premium";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(12).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(13).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a100";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(13).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(14).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="a100_premium";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(14).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(15).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="propane";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(15).toDouble();
                prices.append(FuelPrice);
            }
            if(q->value(16).toInt()>0){
                QJsonObject FuelPrice;
                FuelPrice["ProductId"]="metan";
                FuelPrice["StationId"]=q->value(1).toString();
                FuelPrice["Price"]=q->value(16).toDouble();
                prices.append(FuelPrice);
            }
        }
    }
    QJsonDocument doc;
    doc.setArray(prices);
    delete q;
    return QString(doc.toJson());
}

QString GetRequestHandler::pingHandler(QString a_station, QString a_column){
    QSqlQuery* q = new QSqlQuery(*_db);
    if (q->exec("SELECT AGZSName, AGZS, VCode, Id, Adress, Location_x, Location_y, ColumnsCount, AGZSL, AGZSP FROM [agzs].[dbo].PR_AGZSData where Id=\""+a_station+"\"")) {
        while (q->next()) {
            QSqlDatabase dbLocal = QSqlDatabase::addDatabase("QODBC3");
            QStringList setting;
            if(QFile::exists("Setting.txt")){
                QFile settings("Setting.txt");
                if (settings.open(QIODevice::ReadOnly)){
                    while(!settings.atEnd()){
                        setting << QString::fromLocal8Bit(settings.readLine()).remove("\r\n").remove("\n");
                    }
                    settings.close();
                } else
                    qDebug()<<"Error: setting file is already open";
            } else
                qDebug()<<"Error: setting file not found";
            dbLocal.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(q->value(8).toString(),setting[1],setting[2],q->value(9).toString()));
            if(dbLocal.open()){
                return "1";
            }
            else {
                return "0";
            }
        }
    }

    QJsonArray menu_arr;
    QSqlQuery* query = new QSqlQuery(*_db);
    if(query->exec("SELECT `menu`.`id_menu`, `menu`.`date_menu`, `type_menu`.`title_type`, `menu`.`amount_portion` FROM `menu` INNER JOIN `type_menu` ON `type_menu`.`id_type`=`menu`.`id_type`"))
        while (query->next()) {
            QJsonObject menu_obj;
            menu_obj["id"]=query->value(0).toString();
            menu_obj["date"]=query->value(1).toString();
            menu_obj["type"]=query->value(2).toString();
            menu_obj["amount"]=query->value(3).toString();

            QSqlQuery* qry = new QSqlQuery(*_db);
            QJsonArray dishs;
            if(qry->exec("SELECT `dish`.`title_dish` FROM `menu_dish` INNER JOIN `dish` ON `menu_dish`.`id_dish`=`dish`.`id_dish` WHERE `menu_dish`.`id_menu`= "+menu_obj["id"].toString()))
                while (qry->next()) {
                    dishs.append(qry->value(0).toString());
                }
            delete qry;
            menu_obj["dishs"]=dishs;
            menu_arr.append(menu_obj);
        }
    QJsonObject menu;
    menu["Menu"]=menu_arr;
    QJsonDocument doc;
    doc.setObject(menu);
    delete query;

    return QString(doc.toJson());
}

QString GetRequestHandler::authHandler()
{

    QJsonArray user_arr;
    QSqlQuery* query = new QSqlQuery(*_db);
    if(query->exec("SELECT `auth`.`login`, `auth`.`password`, `role`.`role` FROM `auth` INNER JOIN `role` ON `auth`.`id_role`=`role`.`id_role`"))
        while (query->next()) {
            QJsonObject user_obj;
            user_obj["login"]=query->value(0).toString();
            user_obj["password"]=query->value(1).toString();
            user_obj["role"]=query->value(2).toString();
            user_arr.append(user_obj);
        }
    QJsonObject users;
    users["Users"]=user_arr;
    QJsonDocument doc;
    doc.setObject(users);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::invoiceHandler()
{

    QJsonArray inv_arr;
    QSqlQuery *query = new QSqlQuery(*_db);
    if(query->exec("SELECT `invoice_stock`.`id_invoice`, `invoice`.`date_invoice`, `stock`.`title_stock`, `ingredients`.`title_ingredient`, `invoice_stock`.`amount_ingredient`, `ingredients`.`unit` FROM `invoice` INNER JOIN `invoice_stock` ON `invoice`.`id_invoice` = `invoice_stock`.`id_invoice` INNER JOIN `stock` ON `invoice_stock`.`id_stock`=`stock`.`id_stock` INNER JOIN `ingredients` ON `invoice_stock`.`id_ingredient`=`ingredients`.`id_ingredient`"))
        while (query->next()) {
            QJsonObject inv_obj;
            inv_obj["id"]=query->value(0).toString();
            inv_obj["date"]=query->value(1).toString();
            inv_obj["title-stock"]=query->value(2).toString();
            inv_obj["title-ingredient"]=query->value(3).toString();
            inv_obj["amount"]=query->value(4).toString();
            inv_obj["unit"]=query->value(5).toString();
            inv_arr.append(inv_obj);
        }
    QJsonObject invoice;
    invoice["Invoice"]=inv_arr;
    QJsonArray stocks;
    if(query->exec("SELECT * FROM stock"))
        while (query->next()) {
            stocks.append(query->value(1).toString());
        }
    invoice["Stocks"]=stocks;

    QJsonArray ingredients;
    if(query->exec("SELECT * FROM ingredients"))
        while (query->next()) {
            ingredients.append(query->value(1).toString());
        }
    invoice["Ingredients"]=ingredients;
    QJsonDocument doc;
    doc.setObject(invoice);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::wkInfoHandler()
{
    QJsonObject info;
    QSqlQuery *query = new QSqlQuery(*_db);
    QJsonArray ingredients;
    query->exec("SELECT * FROM ingredients");
    while(query->next()){
        QJsonObject ingredient;
        ingredient["id"]=query->value(0).toString();
        ingredient["title"]=query->value(1).toString();
        ingredient["needstock"]=query->value(2).toString();
        ingredient["unit"]=query->value(3).toString();
        ingredients.append(ingredient);
    }
    info["Ingredients"]=ingredients;
    QJsonArray invoices;
    query->exec("SELECT * FROM invoice");
    while (query->next()) {
        QJsonObject invoice;
        invoice["id"]=query->value(0).toString();
        invoice["date"]=query->value(1).toString();
        invoices.append(invoice);
    }
    info["Invoices"]=invoices;
    QJsonArray stocks;
    query->exec("SELECT * FROM stock");
    while (query->next()) {
        stocks.append(query->value(1).toString());
    }
    info["Stocks"]=stocks;
    QJsonDocument doc;
    doc.setObject(info);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::guestsTableHandler()
{
    QJsonObject table;
    table["Name"] = "guests";
    QJsonArray guests;
    QSqlQuery* query= new QSqlQuery(*_db);
    query->exec("SELECT * FROM guests");
    while (query->next()) {
        QJsonObject guest;
        guest["id"]=query->value(0).toString();
        guest["pasport"]=query->value(1).toString();
        guest["second_name"]=query->value(2).toString();
        guest["first_name"]=query->value(3).toString();
        guest["patronymic"]=query->value(4).toString();
        guest["telephone"]=query->value(5).toString();
        guest["settlement_date"]=query->value(6).toString();
        guest["eviction_date"]=query->value(7).toString();
        guests.append(guest);
    }
    table["Data"]=guests;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}


QString GetRequestHandler::guestCountHandler()
{
    QJsonObject count;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT COUNT(`guests`.`id_guest`) FROM `guests` WHERE `guests`.`settlement_date`<= '"+Request_->GetCgi("date")+"' AND `guests`.`eviction_date` >= '"+Request_->GetCgi("date")+"'");
    while (query->next()) {
        count["count"] = query->value(0).toString();
    }
    QJsonDocument doc;
    doc.setObject(count);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::authTableHandler()
{
    QJsonObject table;
    table["Name"] = "auth";
    QJsonArray auth;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `auth`");
    while (query->next()) {
        QJsonObject user;
        user["login"]=query->value(0).toString();
        user["password"]=query->value(1).toString();
        user["id_role"]=query->value(2).toString();
        auth.append(user);
    }
    table["Data"]=auth;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::dishTableHandler()
{
    QJsonObject table;
    table["Name"] = "dish";
    QJsonArray dishs;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `dish`");
    while (query->next()) {
        QJsonObject dish;
        dish["id"]=query->value(0).toString();
        dish["title"]=query->value(1).toString();
        dishs.append(dish);
    }
    table["Data"]=dishs;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::ingredientsTableHandler()
{
    QJsonObject table;
    table["Name"] = "ingredients";
    QJsonArray ingredients;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `ingredients`");
    while(query->next()){
        QJsonObject ingredient;
        ingredient["id"]=query->value(0).toString();
        ingredient["title"]=query->value(1).toString();
        ingredient["needonstock"]=query->value(2).toString();
        ingredient["unit"]=query->value(3).toString();
        ingredients.append(ingredient);
    }
    table["Data"]=ingredients;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::ingredients_dishTableHandler()
{
    QJsonObject tab;
    tab["Name"] = "ingredients_dish";
    QJsonArray table;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `ingredients_dish`");
    while(query->next()){
        QJsonObject row;
        row["id_dish"]=query->value(0).toString();
        row["id_ingredient"]=query->value(1).toString();
        row["amount"]=query->value(2).toString();
        table.append(row);
    }
    tab["Data"]=table;
    QJsonDocument doc;
    doc.setObject(tab);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::ingredients_stockHandler()
{
    QJsonObject tab;
    tab["Name"] = "ingredients_stock";
    QJsonArray table;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `ingredients_stock`");
    while (query->next()) {
        QJsonObject row;
        row["id_stock"]=query->value(0).toString();
        row["id_ingredient"]=query->value(1).toString();
        row["amount"]=query->value(2).toString();
        table.append(row);
    }
    tab["Data"]=table;
    QJsonDocument doc;
    doc.setObject(tab);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::invoiceTableHandler()
{
    QJsonObject table;
    table["Name"] = "invoice";
    QJsonArray invoices;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `invoice`");
    while (query->next()) {
        QJsonObject invoice;
        invoice["id"]=query->value(0).toString();
        invoice["date"]=query->value(1).toString();
        invoices.append(invoice);
    }
    table["Data"]=invoices;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::invoice_stockHandler()
{
    QJsonObject tab;
    tab["Name"] = "invoice_stock";
    QJsonArray table;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `invoice_stock`");
    while (query->next()) {
        QJsonObject row;
        row["id_invoice"]=query->value(0).toString();
        row["id_ingredient"]=query->value(1).toString();
        row["amount"]=query->value(2).toString();
        row["id_stock"]=query->value(3).toString();
        table.append(row);
    }
    tab["Data"]=table;
    QJsonDocument doc;
    doc.setObject(tab);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::menuTableHandler()
{
    QJsonObject table;
    table["Name"] = "menu";
    QJsonArray menus;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `menu`");
    while (query->next()) {
        QJsonObject menu;
        menu["id"]=query->value(0).toString();
        menu["date"]=query->value(1).toString();
        menu["id_type"]=query->value(2).toString();
        menu["amount"]=query->value(3).toString();
        menus.append(menu);
    }
    table["Data"]=menus;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::menu_dishTableHandler()
{
    QJsonObject tab;
    tab["Name"] = "menu_dish";
    QJsonArray table;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `menu_dish`");
    while (query->next()) {
        QJsonObject row;
        row["id_menu"]=query->value(0).toString();
        row["id_dish"]=query->value(1).toString();
        table.append(row);
    }
    tab["Data"]=table;
    QJsonDocument doc;
    doc.setObject(tab);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::roleTableHandler()
{
    QJsonObject table;
    table["Name"] = "role";
    QJsonArray roles;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `role`");
    while (query->next()) {
        QJsonObject role;
        role["id"]=query->value(0).toString();
        role["role"]=query->value(1).toString();
        roles.append(role);
    }
    table["Data"]=roles;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::stockTableHandler()
{
    QJsonObject table;
    table["Name"] = "stock";
    QJsonArray stocks;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `stock`");
    while (query->next()) {
        QJsonObject stock;
        stock["id"]=query->value(0).toString();
        stock["title"]=query->value(1).toString();
        stocks.append(stock);
    }
    table["Data"]=stocks;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}

QString GetRequestHandler::type_menuTableHandler()
{
    QJsonObject table;
    table["Name"] = "type_menu";
    QJsonArray types;
    QSqlQuery* query = new QSqlQuery(*_db);
    query->exec("SELECT * FROM `type_menu`");
    while (query->next()) {
        QJsonObject type;
        type["id"]=query->value(0).toString();
        type["title"]=query->value(1).toString();
        types.append(type);
    }
    table["Data"]=types;
    QJsonDocument doc;
    doc.setObject(table);
    delete query;
    return QString(doc.toJson());
}
