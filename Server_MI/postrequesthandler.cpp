#include "postrequesthandler.h"

PostRequestHandler::PostRequestHandler(QSqlDatabase *db, Request *request):DB_(db),Request_(request)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

QString PostRequestHandler::orderHandler(QString post)
{
//    {
//        "Id": "088ccc98-73e3-4b4a-aaa9-79dddd02720a",
//        "DateCreate" :  "2019-11-20T17:28:33.994Z",
//        "OrderType":  "Litre",
//        "OrderVolume": "10",
//        "StationId":  "1234",
//        "StationExtendedId":  "1234",
//        "ColumnId":  "1",
//        "FuelId":  "a92",
//        "PriceFuel":  "42.50",
//        "Litre":  "10",
//        "Sum":  "425.00",
//        "Status":  "OrderCreated",
//        "ContractId":  "32135"
//    }

    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        //QJsonObject user = obj["Data"].toObject();

        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("SELECT TOP 1 AGZSName,AGZS,VCode,Id,Adress,Location_x,Location_y,ColumnsCount FROM [agzs].[dbo].PR_AGZSData where Id=\""+doc.object().value("StationId").toString()+"\"");
        if(query->next()){
            QSqlDatabase dbLocal = QSqlDatabase::addDatabase("QODBC3");
            QStringList setting;
            if(QFile::exists("Setting.txt")){
                QFile settings("Setting.txt");
                if (settings.open(QIODevice::ReadOnly)){
                    setting << QString::fromLocal8Bit(settings.readAll()).split("\r\n");
                } else
                    qDebug()<<"Error: setting file is already open";
            } else
                qDebug()<<"Error: setting file not found";
            dbLocal.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(query->value(x).toString(),setting[1],setting[2],setting[3]));
            if(dbLocal.open()){
                QSqlQuery *query2 = new QSqlQuery(dbLocal);
                query2->exec("SELECT TOP 1 VCode, CDate FROM Таблица order by CDate desc");
                if(query2->next()){
                    QSqlQuery *query3 = new QSqlQuery(dbLocal);
                    int fuelID=GetFuelID(doc.object().value("FuelId").toString());
                    //изменить таблицу
                    //изменить формат даты
                    query3->exec("INSERT Таблица set CDate='"+QDateTime::currentDateTime().toString()+"' ,VCode="+query2->value(0).toString()+" ,"
                                "AGZSName='"+query->value(0).toString()+"' ,AGZS="+query->value(1).toString()+" ,ID=\""+doc.object().value("Id").toString()+"\" ,"
                                "DateCreateAPI='"+doc.object().value("DateCreate").toString()+"' ,OrderType='"+doc.object().value("OrderType").toString()+"' ,"
                                "OrderVolume='"+doc.object().value("OrderVolume").toString()+"' ,StationId='"+doc.object().value("StationId").toString()+"' ,"
                                "StationExtendedId='"+doc.object().value("StationExtendedId").toString()+"' ,ColumnId="+doc.object().value("ColumnId").toString()+" ,"
                                "FuelIdAPI='"+doc.object().value("FuelId").toString()+"' , FuelId="+fuelID+" ,PriceFuel="+doc.object().value("PriceFuel").toString()+" ,"
                                "Litre="+doc.object().value("Litre").toString()+" ,Sum="+doc.object().value("Sum").toString()+" ,"
                                "Status='"+doc.object().value("Status").toString()+"' ,ContractId=\""+doc.object().value("ContractId").toString()+"\"");
                    qDebug()<<"lastError="+query3->lastError().text();
                    if(query3->lastError().text()=="")
                        qDebug()<<"Insert complete on "+query->value(0).toString();
                        //запустить функцию отслеживающую выполнение заправки
                    else {
                        qDebug()<<"Insert error on"+query->value(0).toString();
                    }
                    delete query3;
                } else {
                    qDebug()<<"Get last VCode error on"+query->value(0).toString();
                }
                delete query2;
            }
            else {
                qDebug()<<"db on "+query->value(0).toString()+" not open";
            }
        }
        delete query;
    }
    else return QString("NO");
}

int PostRequestHandler::GetFuelID(QString a_fuelIdAPI){
    //дописать
    if(a_fuelIdAPI=="")
        return 0;
}

QString PostRequestHandler::newIngredientHandler(QString post)
{
    // {"title":"name","needonstock":"23","unit":"20"}
    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();

        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("SELECT `ingredients`.`title_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` =\""+obj["title"].toString()+"\"");
        if(query->next()){
            return QString("Ingredients exist");
        }
        else {
            int id = 1;
            bool f =1;
            while (f) {
                query->exec("SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `id_ingredient` = " + QString::number(id));
                if(query->next()){
                    id++;
                }
                else {
                    f=0;
                }
            }

            query->exec("INSERT INTO `ingredients`(`id_ingredient`, `title_ingredient`, `needonstock`, `unit`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"','"+obj["needonstock"].toString()+"' ,'"+obj["unit"].toString()+"')");
            return QString("YES");
        }
    }
    else return QString("NO");
}

QString PostRequestHandler::newStockHandler(QString post)
{
    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();

        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("SELECT `stock`.`title_stock` FROM `stock` WHERE `stock`.`title_stock` =\""+obj["title"].toString()+"\"");
        if(query->next()){
            return QString("Stock exist");
        }
        else {
            int id = 1;
            bool f =1;
            while (f) {
                query->exec("SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`id_stock` = " + QString::number(id));
                if(query->next()){
                    id++;
                }
                else {
                    f=0;
                }
            }

            query->exec("INSERT INTO `stock`(`id_stock`, `title_stock`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"')");
            return QString("YES");
        }
    }
    else return QString("NO");
}

QString PostRequestHandler::newDishHandler(QString post)
{
    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();

        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("SELECT `dish`.`title_dish` FROM `dish` WHERE `dish`.`title_dish` =\""+obj["title"].toString()+"\"");
        if(query->next()){
            return QString("Dish exist");
        }
        else {
            int id = 1;
            bool f =1;
            while (f) {
                query->exec("SELECT `dish`.`id_dish` FROM `dish` WHERE `dish`.`id_dish` = " + QString::number(id));
                if(query->next()){
                    id++;
                }
                else {
                    f=0;
                }
            }

            query->exec("INSERT INTO `dish`(`id_dish`, `title_dish`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"')");

            QJsonArray arr = obj["ingredients"].toArray();
            for (int i=0; i<arr.size();i++){
                query->exec("INSERT INTO `ingredients_dish`(`id_dish`, `id_ingredient`, `amount_ingredient`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+arr[i].toObject().value("title").toString()+"\" ),'"+arr[i].toObject().value("amount").toString()+"')");
            }


            return QString("YES");
        }
    }
    else return QString("NO");
}

QString PostRequestHandler::newGuestHandler(QString post)
{
    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();

        QSqlQuery *query = new QSqlQuery(*DB_);
        int id = 1;
        bool f =1;
        while (f) {
            query->exec("SELECT `guests`.`id_guest` FROM `guests` WHERE `guests`.`id_guest` = " + QString::number(id));
            if(query->next()){
                id++;
            }
            else {
                f=0;
            }
        }


        QString tamplate = "INSERT INTO `guests`(`id_guest`, `pasport`, `second_name`, `first_name`, `patronymic`, `telephone`, `settlement_date`, `eviction_date`) VALUES ('%1','%2','%3','%4','%5','%6','%7','%8')";
        query->exec(tamplate.arg(QString::number(id)).arg(obj["pasport"].toString()).arg(obj["second_name"].toString()).arg(obj["first_name"].toString()).arg(obj["patronymic"].toString()).arg(obj["telephone"].toString()).arg(obj["settlement_date"].toString()).arg(obj["eviction_date"].toString()));



        return QString("YES");
    }

    else return QString("NO");
}

QString PostRequestHandler::updateGuestHandler(QString post)
{
    if(post != ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("SELECT `guests`.`id_guest` FROM `guests` WHERE `guests`.`id_guest` =\""+obj["id"].toString()+"\"");
        if(query->next()){
            QString tamplate ="UPDATE `guests` SET`pasport`='%1',`second_name`='%2',`first_name`='%3',`patronymic`='%4',`telephone`='%5',`settlement_date`='%6',`eviction_date`='%7' WHERE `guests`.`id_guest` = \"%8\"";
            query->exec(tamplate.arg(obj["pasport"].toString()).arg(obj["second_name"].toString()).arg(obj["first_name"].toString()).arg(obj["patronymic"].toString()).arg(obj["telephone"].toString()).arg(obj["settlement_date"].toString()).arg(obj["eviction_date"].toString()).arg(obj["id"].toString()));
            return QString("YES");
        }
        else {
            return QString("Pasport not found");
        }

    }
    else return QString("NO");
}

QString PostRequestHandler::addMenuHandler(QString post)
{
    //{"date":"2019-02-10","amount":"30","type":"dinner","dishs":["dish1","dish2"]}

    if(post!= ""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        int id = 1;
        bool f =1;
        while (f) {
            query->exec("SELECT `menu`.`id_menu` FROM `menu` WHERE `menu`.`id_menu` = " + QString::number(id));
            if(query->next()){
                id++;
            }
            else {
                f=0;
            }
        }
        query->exec("INSERT INTO `menu`(`id_menu`, `date_menu`, `id_type`, `amount_portion`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"',(SELECT `type_menu`.`id_type` FROM `type_menu` WHERE `type_menu`.`title_type` = \""+obj["type"].toString()+"\" ),"+QString::number(obj["amount"].toInt())+")");
        QJsonArray dishs = doc["dishs"].toArray();

        for(int i=0; i< dishs.size();i++){
            query->exec("INSERT INTO `menu_dish`(`id_menu`, `id_dish`) VALUES ('"+QString::number(id)+"',(SELECT `dish`.`id_dish` FROM `dish` WHERE `dish`.`title_dish` = \""+dishs[i].toString()+"\"))");

            query->exec("SELECT `ingredients`.`title_ingredient` , `ingredients_dish`.`amount_ingredient` FROM `ingredients_dish` INNER JOIN `ingredients` ON `ingredients_dish`.`id_ingredient` = `ingredients`.`id_ingredient` INNER JOIN `dish` ON `ingredients_dish`.`id_dish` = `dish`.`id_dish` WHERE `dish`.`title_dish` = \""+dishs[i].toString()+"\"");
            while (query->next()) {
                QString ing_title = query->value(0).toString(); // title
                int count = query->value(1).toInt()*obj["amount"].toInt();
                QSqlQuery* qry = new QSqlQuery(*DB_);
                qry->exec("SELECT `ingredients_stock`.`id_stock`, `ingredients`.`title_ingredient`, `ingredients_stock`.`amount_ingredient` FROM `ingredients_stock` INNER JOIN `ingredients` ON `ingredients_stock`.`id_ingredient` = `ingredients`.`id_ingredient` WHERE `ingredients`.`title_ingredient` = \""+query->value(0).toString()+"\"");
                while(qry->next()){
                    while (count!=0) {
                        int ing_am = qry->value(2).toInt();
                        if(count >= ing_am){
                            QString stock = qry->value(0).toString(); // id

                            //{"date":"2019-02-10","amount":"30","type":"dinner","dishs":["dish1","dish2"]}
                            //инсерт в invoice и invoice_stock
                            QSqlQuery* addinvoice = new QSqlQuery(*DB_);
                            id = 1;
                            f =1;
                            while (f) {
                                addinvoice->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = " + QString::number(id));
                                if(addinvoice->next()){
                                    id++;
                                }
                                else {
                                    f=0;
                                }
                            }
                            addinvoice->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"')");

                            addinvoice->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(-count)+"','"+stock+"')");

                            QSqlQuery *upd = new QSqlQuery(*DB_);
                            upd->exec("UPDATE `ingredients_stock` SET `amount_ingredient`= 0  WHERE `ingredients_stock`.`id_stock` = "+stock);
                            count -= ing_am;

                        }
                        else {
                            int sub = ing_am - count;
                            QString stock = qry->value(0).toString(); // id
                            QSqlQuery* addinvoice = new QSqlQuery(*DB_);
                            id = 1;
                            f =1;
                            while (f) {
                                addinvoice->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = " + QString::number(id));
                                if(addinvoice->next()){
                                    id++;
                                }
                                else {
                                    f=0;
                                }
                            }
                            addinvoice->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"')");

                            addinvoice->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(-count)+"','"+stock+"')");

                            QSqlQuery *upd = new QSqlQuery(*DB_);
                            upd->exec("UPDATE `ingredients_stock` SET `amount_ingredient`= "+QString::number(sub)+"  WHERE `ingredients_stock`.`id_stock` = "+qry->value(0).toString());
                            count = 0;
                        }
                    }
                }
            }

        }


        return QString ("YES");
    }
    else {
        return QString ("NO");
    }

}

QString PostRequestHandler::addInvoiceHandler(QString post)
{
    //{"date":"2019-02-13","stock":"Подвал","id":"982", "ingredients": [{"title":"Морковь","amount":20},...]}

    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QString stock = obj["stock"].toString();
        query->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = "+obj["id"].toString());
        if(query->next()){
            return QString("Invoice exsist");
        }
        else {

            query->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+obj["id"].toString()+"','"+obj["date"].toString()+"')");

            QJsonArray ingredients = obj["ingredients"].toArray();
            for (int i=0;i<ingredients.size();i++) {
                QString ing_title = ingredients[i].toObject().value("title").toString();
                int amount = ingredients[i].toObject().value("amount").toInt();
                query->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+obj["id"].toString()+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(amount)+"',(SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock`= \""+stock+"\"))");

                QSqlQuery* qry = new QSqlQuery(*DB_);
                qry->exec("SELECT * FROM `ingredients_stock` WHERE `ingredients_stock`.`id_ingredient` = (SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient`= \""+ing_title+"\" ) AND `ingredients_stock`.`id_stock` = (SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock` = \""+stock+"\")");
                if(qry->next()){
                    while (qry->next()) {
                        QString id_ing = qry->record().value(1).toString();
                        QString id_stock = qry->record().value(0).toString();
                        int count = qry->value(2).toInt() + amount;
                        qDebug() << id_ing << " " << id_stock << " " << count;
                        qry->exec("UPDATE `ingredients_stock` SET `amount_ingredient` = "+QString::number(count)+" WHERE (`ingredients_stock`.`id_stock` = \""+id_stock+"\")  AND (`ingredients_stock`.`id_ingredient` = \""+id_ing+"\") ");
                    }
                }
                else {
                    qry->exec("INSERT INTO `ingredients_stock`(`id_stock`, `id_ingredient`, `amount_ingredient`) VALUES ((SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock` = \""+stock+"\"),(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient`= \""+ing_title+"\"),'"+QString::number(amount)+"')");
                }
            }
            return QString("YES");
        }
    }
    else {
        return QString("NO");
    }


}

QString PostRequestHandler::deleteTableHandler(QString post)
{
    //{"table":"name"}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE 0");
        return QString ("YES");
    }
    else {
        return QString ("NO");
    }
}

QString PostRequestHandler::deleteByIdHandler(QString post)
{
    //{"table":"name,"id":"1"}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QString field;
        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
        field = query->record().fieldName(0);
        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE `"+field+"` = "+obj["id"].toString());
        return QString("YES");
    }
    else {
        return QString("NO");
    }
}

QString PostRequestHandler::deleteByWhereHandler(QString post)
{
    //{"table":"name","where1":" ","where2":" "}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QString field1;
        QString field2;
        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
        field1 = query->record().fieldName(0);
        field2 = query->record().fieldName(1);
        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE `"+field1+"` = "+obj["where1"].toString()+" AND `"+field2+"` = "+obj["where2"].toString());
        return QString("YES");
    }
    else {
        return QString("NO");
    }
}

QString PostRequestHandler::insertHandler(QString post)
{
    //{"table":"name","val":[1,2,3...]}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QJsonArray val = obj["values"].toArray();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QStringList list;
        query->exec("pragma table_info("+obj["table"].toString()+")");
        while (query->next()) {
            list.append(query->value(1).toString());
        }
        QString tamplate = "INSERT INTO `"+obj["table"].toString()+"`(";
        for (int i=0;i<list.size();i++) {
            if (i!=list.size()-1)
                tamplate+="`"+list.at(i)+"`,";
            else {
                tamplate+="`"+list.at(i)+"`) VALUES (";
            }
        }
        for (int i=0;i<val.size();i++) {
            if(i!=val.size()-1){
                tamplate+="'"+val[i].toString()+"',";
            }
            else {
                tamplate+="'"+val[i].toString()+"')";
            }
        }
        query->exec(tamplate);
        return QString("YES");
    }
    else {
        return QString("NO");
    }
}

QString PostRequestHandler::updateByIdHandler(QString post)
{
    //{"table":"name","val":[1,2,3...],"id":"1"}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QJsonArray val = obj["values"].toArray();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QStringList list;
        query->exec("pragma table_info("+obj["table"].toString()+")");
        while (query->next()) {
            list.append(query->value(1).toString());
        }
        QString tamplate = "UPDATE `"+ obj["table"].toString()+"` SET ";
        for (int i=0;i<list.size();i++) {
            if (i!=list.size()-1)
                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"',";
            else {
                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"' WHERE `"+list.at(0)+"` = "+obj["id"].toString();
            }
        }
        query->exec(tamplate);
        return QString("YES");
    }
    else {
        return QString("NO");
    }
}

QString PostRequestHandler::updateByWhereHandler(QString post)
{
    //{"table":"name","val":[1,2,3...],"where1":"1","where2":"1"}
    if(post!=""){
        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
        QJsonObject obj = doc.object();
        QJsonArray val = obj["values"].toArray();
        QSqlQuery *query = new QSqlQuery(*DB_);
        QStringList list;
        query->exec("pragma table_info("+obj["table"].toString()+")");
        while (query->next()) {
            list.append(query->value(1).toString());
        }
        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
        QString field1 = query->record().fieldName(0);
        QString field2 = query->record().fieldName(1);
        QString tamplate = "UPDATE `"+ obj["table"].toString()+"` SET ";
        for (int i=0;i<list.size();i++) {
            if (i!=list.size()-1)
                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"',";
            else {
                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"' WHERE `"+field1+"` = "+obj["where1"].toString()+" AND `"+field2+"` = "+obj["where2"].toString();
            }
        }
        query->exec(tamplate);
        return QString("YES");
    }
    else {
        return QString("NO");
    }
}
