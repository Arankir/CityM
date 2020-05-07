#include "postrequesthandler.h"

PostRequestHandler::PostRequestHandler(QSqlDatabase *db, Request *request):DB_(db),Request_(request){
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

QString PostRequestHandler::orderHandler(QString post){
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
        QSqlQuery *qServer_AGZSData = new QSqlQuery(*DB_);
        qServer_AGZSData->exec("SELECT TOP 1 AGZSName, AGZS, VCode, Id, Adress, Location_x, Location_y, ColumnsCount, AGZSL, AGZSP "
                    ",[diesel_price] ,[diesel_premium_price] ,[a80_price] ,[a92_price] ,[a92_premium_price] ,[a95_price] "
                    ",[a95_premium_price] ,[a98_price] ,[a98_premium_price] ,[a100_price] ,[a100_premium_price] ,[propane_price] "
                    ",[metan_price] FROM [agzs].[dbo].PR_AGZSData where Id='"+doc.object().value("StationId").toString()+"' order by CDate desc");
        if(qServer_AGZSData->next()){
            if(ConformityPrice(doc.object().value("FuelId").toString(),doc.object().value("FriceFuel").toString(),
                               qServer_AGZSData->value(10).toString(),qServer_AGZSData->value(11).toString(),
                               qServer_AGZSData->value(12).toString(),qServer_AGZSData->value(13).toString(),
                               qServer_AGZSData->value(14).toString(),qServer_AGZSData->value(15).toString(),
                               qServer_AGZSData->value(16).toString(),qServer_AGZSData->value(17).toString(),
                               qServer_AGZSData->value(18).toString(),qServer_AGZSData->value(19).toString(),
                               qServer_AGZSData->value(20).toString(),qServer_AGZSData->value(21).toString(),
                               qServer_AGZSData->value(22).toString())==-1){
                delete qServer_AGZSData;
                qDebug()<<"price error on "+qServer_AGZSData->value(0).toString();
                return QString("HTTP/1.1 402 \r\n\r\nBad request");
            }
            int fuelID=GetFuelID(doc.object().value("FuelId").toString());
            int sideAdress=-1, nozzle=-1, trkFuelCode=-1, trkVCode=-1;
            SmashData(doc.object().value("ColumnId").toString(),&sideAdress, &nozzle, &trkFuelCode, &trkVCode);
            QSqlQuery *qServer_AGZSColumnsData = new QSqlQuery(*DB_);
            if(sideAdress>0 && nozzle>0 && trkFuelCode>0 && trkVCode>0){
                qServer_AGZSColumnsData->exec("SELECT TOP 1 [AGZSName],[AGZS],[CDate],[VCode],[Link],[TrkType],[DeviceName],[Serial],[FuelName]"
                                              ",[FuelShortName],[Side],[SideAdress],[Nozzle],[TrkFuelCode],[TrkVCode] FROM [agzs].[dbo].[PR_AGZSColumnsData] "
                                              "where SideAdress="+QString::number(sideAdress)+" and Nozzle="+QString::number(nozzle)+" and "
                                              "TrkFuelCode="+QString::number(trkFuelCode)+" and TRKVCode="+QString::number(trkVCode)+" and Link="+qServer_AGZSData->value(2).toString());
                if(qServer_AGZSColumnsData->next()){
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
                    QSqlDatabase dbAGZS = QSqlDatabase::addDatabase("QODBC3");
                    dbAGZS.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(qServer_AGZSData->value(8).toString(),setting[1],setting[2],qServer_AGZSData->value(9).toString()));
                    if(dbAGZS.open()){
                        QSqlQuery *qAGZS_Transaction = new QSqlQuery(dbAGZS);
                        qAGZS_Transaction->exec("SELECT TOP 1 Value FROM [agzs].[dbo].[LxKeysOfCodes] where Key='ADAST_TRKTransaction'");
                        int transVCode=-1;
                        if(qAGZS_Transaction->next()){
                            int requestTotalPriceDB=-1,	requestVolumeDB=-1,	requestUnitPriceDB=-1, moneyTakenDB=-1, fullTankDB=-1;
                            SmashMoneyData(doc.object(), &requestTotalPriceDB, &requestVolumeDB, &requestUnitPriceDB, &moneyTakenDB, &fullTankDB);
                            if(requestTotalPriceDB>-1 && requestVolumeDB>-1 && requestUnitPriceDB>-1 && moneyTakenDB>-1 && fullTankDB>-1){
                                transVCode=qAGZS_Transaction->value(0).toInt()+1;
                                qAGZS_Transaction->exec("Update [agzs].[dbo].[LxKeysOfCodes] set Value="+QString::number(transVCode)+" where Key='ADAST_TRKTransaction'");
                                //Занести данные в Transaction
                                //AGZSName	LocalVCode	TrkType	DeviceName	Serial	FuelName	FuelShortName	Side	SideAddress	Nozzle	TrkFuelCode	TransNum
                                //TrkTotalPriceDB	TrkVolumeDB	TrkUnitPriceDB	RequestTotalPriceDB	RequestVolumeDB	RequestUnitPriceDB	RequestField	State	iState
                                //TrkTransType	LitersCountBeforeDB	MoneyCountBeforeDB	TransCountBefore	LitersCountAfterDB	MoneyCountAfterDB	TransCountAfter	Result
                                //DateOpen	DateClose	TemperatureDB	PayOperationVCode	PayWay	PrePostPay	WUser	WDate	CUser	CDate	CHost	WHost	VCode
                                //AddedForTransVCode	AditionalTransVCode	ActiveDB	MassDB	Smena	TrkVcode	CapacityVcode	PumpPlace	MoneyTakenDB	iPayWay	AutoCheckDB	Closed
                                //FullTankDB	AGZS	FuelVCode	rowguid	Propan
                                ///Смена!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                qAGZS_Transaction->exec("INSERT INTO [agzs].[dbo].[ADAST_TRKTransaction] VALUES("
                                             +qServer_AGZSData->value(0).toString()+", "+QString::number(transVCode)+", "+qServer_AGZSColumnsData->value(5).toString()+", "
                                             +qServer_AGZSColumnsData->value(6).toString()+", "+qServer_AGZSColumnsData->value(7).toString()+", "
                                             +qServer_AGZSColumnsData->value(8).toString()+", "+qServer_AGZSColumnsData->value(9).toString()+", "
                                             +qServer_AGZSColumnsData->value(10).toString()+", "+qServer_AGZSColumnsData->value(11).toString()+", "
                                             +qServer_AGZSColumnsData->value(12).toString()+", "+qServer_AGZSColumnsData->value(13).toString()+", "+", '', 0, 0, 0, "
                                             +QString::number(requestTotalPriceDB)+", "+QString::number(requestVolumeDB)+", "+QString::number(requestUnitPriceDB)+", V, "
                                             "'Завершение выдачи', 8, '', 0, 0, 0, 0, 0, 0, 'Выдача завершена: 1', '"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")+"', "
                                             "'19000101', -100, 0, 'СИТИМОБИЛ', 0, 'Server', '"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")+"', 'Server', "
                                             "'"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")+"', 'Server', 'Server', "+qServer_AGZSData->value(0).toString()+", "
                                             "0, 0, 0, 0, "+GetSmena(qServer_AGZSData->value(8).toString(),qServer_AGZSData->value(9).toString())+", "+QString::number(trkVCode)+", 0, "+QString::number(sideAdress)+", "+QString::number(moneyTakenDB)+", "
                                             "10, 0, 0, "+QString::number(fullTankDB)+", "+qServer_AGZSColumnsData->value(1).toString()+", "+QString::number(fuelID)+", DEFAULT, 0"
                                             ")");
                                QSqlQuery *qAPITransaction = new QSqlQuery(*DB_);
                                qAPITransaction->exec("SELECT TOP 1 VCode FROM PR_APITransaction Order by CDate desc");
                                int apiVCode=qAPITransaction->value(0).toInt()+1;
                                qAPITransaction->exec("INSERT PR_APITransaction set AGZSName='"+qServer_AGZSData->value(0).toString()+"' ,AGZS="+qServer_AGZSData->value(1).toString()+" ,"
                                            "CDate='"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")+"' ,VCode="+QString::number(apiVCode)+" ,APIID=\""+doc.object().value("Id").toString()+"\" ,"
                                            "APIDateCreateAPI='"+doc.object().value("DateCreate").toString()+"' ,APIOrderType='"+doc.object().value("OrderType").toString()+"' ,"
                                            "APIOrderVolume='"+doc.object().value("OrderVolume").toString()+"' ,APIStationId='"+doc.object().value("StationId").toString()+"' ,"
                                            "APIStationExtendedId='"+doc.object().value("StationExtendedId").toString()+"' ,APIColumnId="+doc.object().value("ColumnId").toString()+" ,"
                                            "APIFuelId='"+doc.object().value("FuelId").toString()+"' , FuelId="+fuelID+" ,APIPriceFuel="+doc.object().value("PriceFuel").toString()+" ,"
                                            "APILitre="+doc.object().value("Litre").toString()+" ,APISum="+doc.object().value("Sum").toString()+" ,"
                                            "APIStatus='"+doc.object().value("Status").toString()+"' ,APIContractId='"+doc.object().value("ContractId").toString()+"', agent='CityMobile', "
                                            "localState='0'");
                                if(QFile::exists("Requests.txt")){
                                    QFile requestTransactionsVCode("Requests.txt");
                                    if (requestTransactionsVCode.open(QIODevice::Append)){
                                        QTextStream writeStream(&requestTransactionsVCode);
                                        writeStream<<QString::number(apiVCode)+"\n";
                                        requestTransactionsVCode.close();
                                    } else
                                        qDebug()<<"Error: request file is already open";
                                } else
                                    qDebug()<<"Error: request file not found";
                                delete qAPITransaction;
                            }
                            qDebug()<<"lastError="+qAGZS_Transaction->lastError().text();
                            if(qAGZS_Transaction->lastError().text()=="")
                                qDebug()<<"Insert complete on "+qServer_AGZSData->value(0).toString();
                            else
                                qDebug()<<"Insert error on "+qServer_AGZSData->value(0).toString();
                        } else
                            qDebug()<<"Get last VCode error on "+qServer_AGZSData->value(0).toString();
                        delete qAGZS_Transaction;
                    } else
                        qDebug()<<"db on "+qServer_AGZSData->value(0).toString()+" not open";
                }
            }
            delete qServer_AGZSData;
        } else {
            delete qServer_AGZSData;
            qDebug()<<qServer_AGZSData->value(0).toString()+" not found";
            return QString("HTTP/1.1 400 \r\n\r\nBad request");
        }
        qDebug()<<"Insert transaction";
        return QString("HTTP/1.1 200 OK \r\n\r\n\r\n");
    } else
        qDebug()<<"Not insert transaction, post is empty";
        return QString("HTTP/1.1 404 \r\n\r\nBad request");
}

int PostRequestHandler::GetFuelID(QString a_fuelIdAPI){
    if(a_fuelIdAPI=="diesel")
        return 32;
    if(a_fuelIdAPI=="diesel_premium")
        return 0;
    if(a_fuelIdAPI=="a80")
        return 0;
    if(a_fuelIdAPI=="a92")
        return 3;
    if(a_fuelIdAPI=="a92_premium")
        return 0;
    if(a_fuelIdAPI=="a95")
        return 8;
    if(a_fuelIdAPI=="a95_premium")
        return 0;
    if(a_fuelIdAPI=="a98")
        return 0;
    if(a_fuelIdAPI=="a98_premium")
        return 0;
    if(a_fuelIdAPI=="a100")
        return 0;
    if(a_fuelIdAPI=="a100_premium")
        return 0;
    if(a_fuelIdAPI=="propane")
        return 14;
    if(a_fuelIdAPI=="metan")
        return 0;
    return 0;
}

int PostRequestHandler::ConformityPrice(QString a_fuel, QString a_price, QString a_diesel_price, QString a_diesel_premium_price, QString a_a80_price, QString a_a92_price,
                                        QString a_a92_premium_price, QString a_a95_price, QString a_a95_premium_price, QString a_a98_price, QString a_a98_premium_price,
                                        QString a_a100_price, QString a_a100_premium_price, QString a_propane_price, QString a_metan_price){
    if((a_fuel=="diesel"&&a_price!=a_diesel_price)||
       (a_fuel=="diesel_premium"&&a_price!=a_diesel_premium_price)||
       (a_fuel=="a80"&&a_price!=a_a80_price)||
       (a_fuel=="a92"&&a_price!=a_a92_price)||
       (a_fuel=="a92_premium"&&a_price!=a_a92_premium_price)||
       (a_fuel=="a95"&&a_price!=a_a95_price)||
       (a_fuel=="a95_premium"&&a_price!=a_a95_premium_price)||
       (a_fuel=="a98"&&a_price!=a_a98_price)||
       (a_fuel=="a98_premium"&&a_price!=a_a98_premium_price)||
       (a_fuel=="a100"&&a_price!=a_a100_price)||
       (a_fuel=="a100_premium"&&a_price!=a_a100_premium_price)||
       (a_fuel=="propane"&&a_price!=a_propane_price)||
       (a_fuel=="metan"&&a_price!=a_metan_price)){
        return -1;
    }
    return 1;
}

void PostRequestHandler::SmashData(QString a_data, int *a_sideAdress, int *a_nozzle, int *a_trkFuelCode, int *a_trkVCode){
    int data=a_data.toInt();
    *a_sideAdress=data/100000; data%=100000;
    *a_nozzle=data/10000; data%=10000;
    *a_trkFuelCode=data/1000; data%=1000;
    *a_trkVCode=data;
}

void PostRequestHandler::SmashMoneyData(QJsonObject a_data, int *a_requestTotalPriceDB, int *a_requestVolumeDB, int *a_requestUnitPriceDB, int *a_moneyTakenDB, int *a_fullTankDB){
    if(a_data.value("OrderType").toString()=="Money"){
        *a_requestTotalPriceDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestVolumeDB=*a_requestTotalPriceDB / *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=0;
    } else if(a_data.value("OrderType").toString()=="Liters"){
        *a_requestVolumeDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestTotalPriceDB=*a_requestVolumeDB * *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=0;
    } else if(a_data.value("OrderType").toString()=="FullTank"){
        *a_requestTotalPriceDB=a_data.value("OrderVolume").toString().toInt();
        *a_requestUnitPriceDB=a_data.value("PriceFuel").toString().toInt();
        *a_requestVolumeDB=*a_requestTotalPriceDB / *a_requestUnitPriceDB;
        *a_moneyTakenDB=a_data.value("OrderVolume").toString().toInt();
        *a_fullTankDB=1;
    }
}

QString PostRequestHandler::GetSmena(QString a_l, QString a_p){
    QStringList setting;
    if(QFile::exists("Setting.txt")){
        QFile settings("Setting.txt");
        if (settings.open(QIODevice::ReadOnly)){
            setting << QString::fromLocal8Bit(settings.readAll()).split("\r\n");
            settings.close();
        } else
            qDebug()<<"Error: setting file is already open";
    } else
        qDebug()<<"Error: setting file not found";
    QSqlDatabase dbAGZS = QSqlDatabase::addDatabase("QODBC3");
    dbAGZS.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(a_l,setting[1],setting[2],a_p));
    if(dbAGZS.open()){
        QSqlQuery *qAGZS_Transaction = new QSqlQuery(dbAGZS);
        qAGZS_Transaction->exec("SELECT top 1 CDate, VCode, Smena FROM [agzs].[dbo].[ADAST_TRKTransaction] order by CDate desc");
        qAGZS_Transaction->next();
        QDateTime trans=QDateTime::fromString(qAGZS_Transaction->value(0).toString(),"yyyy-MM-dd hh:mm:ss.zzz");
        return QDateTime::currentDateTime().date()>trans.date()?QString::number(qAGZS_Transaction->value(2).toInt()+1):QString::number(qAGZS_Transaction->value(2).toInt());
    }
    return "";
}

#define Comment {
//QString PostRequestHandler::newIngredientHandler(QString post)
//{
//    // {"title":"name","needonstock":"23","unit":"20"}
//    if(post != ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();

//        QSqlQuery *query = new QSqlQuery(*DB_);
//        query->exec("SELECT `ingredients`.`title_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` =\""+obj["title"].toString()+"\"");
//        if(query->next()){
//            return QString("Ingredients exist");
//        }
//        else {
//            int id = 1;
//            bool f =1;
//            while (f) {
//                query->exec("SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `id_ingredient` = " + QString::number(id));
//                if(query->next()){
//                    id++;
//                }
//                else {
//                    f=0;
//                }
//            }

//            query->exec("INSERT INTO `ingredients`(`id_ingredient`, `title_ingredient`, `needonstock`, `unit`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"','"+obj["needonstock"].toString()+"' ,'"+obj["unit"].toString()+"')");
//            return QString("YES");
//        }
//    }
//    else return QString("NO");
//}

//QString PostRequestHandler::newStockHandler(QString post)
//{
//    if(post != ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();

//        QSqlQuery *query = new QSqlQuery(*DB_);
//        query->exec("SELECT `stock`.`title_stock` FROM `stock` WHERE `stock`.`title_stock` =\""+obj["title"].toString()+"\"");
//        if(query->next()){
//            return QString("Stock exist");
//        }
//        else {
//            int id = 1;
//            bool f =1;
//            while (f) {
//                query->exec("SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`id_stock` = " + QString::number(id));
//                if(query->next()){
//                    id++;
//                }
//                else {
//                    f=0;
//                }
//            }

//            query->exec("INSERT INTO `stock`(`id_stock`, `title_stock`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"')");
//            return QString("YES");
//        }
//    }
//    else return QString("NO");
//}

//QString PostRequestHandler::newDishHandler(QString post)
//{
//    if(post != ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();

//        QSqlQuery *query = new QSqlQuery(*DB_);
//        query->exec("SELECT `dish`.`title_dish` FROM `dish` WHERE `dish`.`title_dish` =\""+obj["title"].toString()+"\"");
//        if(query->next()){
//            return QString("Dish exist");
//        }
//        else {
//            int id = 1;
//            bool f =1;
//            while (f) {
//                query->exec("SELECT `dish`.`id_dish` FROM `dish` WHERE `dish`.`id_dish` = " + QString::number(id));
//                if(query->next()){
//                    id++;
//                }
//                else {
//                    f=0;
//                }
//            }

//            query->exec("INSERT INTO `dish`(`id_dish`, `title_dish`) VALUES ('"+QString::number(id)+"','"+obj["title"].toString()+"')");

//            QJsonArray arr = obj["ingredients"].toArray();
//            for (int i=0; i<arr.size();i++){
//                query->exec("INSERT INTO `ingredients_dish`(`id_dish`, `id_ingredient`, `amount_ingredient`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+arr[i].toObject().value("title").toString()+"\" ),'"+arr[i].toObject().value("amount").toString()+"')");
//            }


//            return QString("YES");
//        }
//    }
//    else return QString("NO");
//}

//QString PostRequestHandler::newGuestHandler(QString post)
//{
//    if(post != ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();

//        QSqlQuery *query = new QSqlQuery(*DB_);
//        int id = 1;
//        bool f =1;
//        while (f) {
//            query->exec("SELECT `guests`.`id_guest` FROM `guests` WHERE `guests`.`id_guest` = " + QString::number(id));
//            if(query->next()){
//                id++;
//            }
//            else {
//                f=0;
//            }
//        }


//        QString tamplate = "INSERT INTO `guests`(`id_guest`, `pasport`, `second_name`, `first_name`, `patronymic`, `telephone`, `settlement_date`, `eviction_date`) VALUES ('%1','%2','%3','%4','%5','%6','%7','%8')";
//        query->exec(tamplate.arg(QString::number(id)).arg(obj["pasport"].toString()).arg(obj["second_name"].toString()).arg(obj["first_name"].toString()).arg(obj["patronymic"].toString()).arg(obj["telephone"].toString()).arg(obj["settlement_date"].toString()).arg(obj["eviction_date"].toString()));



//        return QString("YES");
//    }

//    else return QString("NO");
//}

//QString PostRequestHandler::updateGuestHandler(QString post)
//{
//    if(post != ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        query->exec("SELECT `guests`.`id_guest` FROM `guests` WHERE `guests`.`id_guest` =\""+obj["id"].toString()+"\"");
//        if(query->next()){
//            QString tamplate ="UPDATE `guests` SET`pasport`='%1',`second_name`='%2',`first_name`='%3',`patronymic`='%4',`telephone`='%5',`settlement_date`='%6',`eviction_date`='%7' WHERE `guests`.`id_guest` = \"%8\"";
//            query->exec(tamplate.arg(obj["pasport"].toString()).arg(obj["second_name"].toString()).arg(obj["first_name"].toString()).arg(obj["patronymic"].toString()).arg(obj["telephone"].toString()).arg(obj["settlement_date"].toString()).arg(obj["eviction_date"].toString()).arg(obj["id"].toString()));
//            return QString("YES");
//        }
//        else {
//            return QString("Pasport not found");
//        }

//    }
//    else return QString("NO");
//}

//QString PostRequestHandler::addMenuHandler(QString post)
//{
//    //{"date":"2019-02-10","amount":"30","type":"dinner","dishs":["dish1","dish2"]}

//    if(post!= ""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        int id = 1;
//        bool f =1;
//        while (f) {
//            query->exec("SELECT `menu`.`id_menu` FROM `menu` WHERE `menu`.`id_menu` = " + QString::number(id));
//            if(query->next()){
//                id++;
//            }
//            else {
//                f=0;
//            }
//        }
//        query->exec("INSERT INTO `menu`(`id_menu`, `date_menu`, `id_type`, `amount_portion`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"',(SELECT `type_menu`.`id_type` FROM `type_menu` WHERE `type_menu`.`title_type` = \""+obj["type"].toString()+"\" ),"+QString::number(obj["amount"].toInt())+")");
//        QJsonArray dishs = doc["dishs"].toArray();

//        for(int i=0; i< dishs.size();i++){
//            query->exec("INSERT INTO `menu_dish`(`id_menu`, `id_dish`) VALUES ('"+QString::number(id)+"',(SELECT `dish`.`id_dish` FROM `dish` WHERE `dish`.`title_dish` = \""+dishs[i].toString()+"\"))");

//            query->exec("SELECT `ingredients`.`title_ingredient` , `ingredients_dish`.`amount_ingredient` FROM `ingredients_dish` INNER JOIN `ingredients` ON `ingredients_dish`.`id_ingredient` = `ingredients`.`id_ingredient` INNER JOIN `dish` ON `ingredients_dish`.`id_dish` = `dish`.`id_dish` WHERE `dish`.`title_dish` = \""+dishs[i].toString()+"\"");
//            while (query->next()) {
//                QString ing_title = query->value(0).toString(); // title
//                int count = query->value(1).toInt()*obj["amount"].toInt();
//                QSqlQuery* qry = new QSqlQuery(*DB_);
//                qry->exec("SELECT `ingredients_stock`.`id_stock`, `ingredients`.`title_ingredient`, `ingredients_stock`.`amount_ingredient` FROM `ingredients_stock` INNER JOIN `ingredients` ON `ingredients_stock`.`id_ingredient` = `ingredients`.`id_ingredient` WHERE `ingredients`.`title_ingredient` = \""+query->value(0).toString()+"\"");
//                while(qry->next()){
//                    while (count!=0) {
//                        int ing_am = qry->value(2).toInt();
//                        if(count >= ing_am){
//                            QString stock = qry->value(0).toString(); // id

//                            //{"date":"2019-02-10","amount":"30","type":"dinner","dishs":["dish1","dish2"]}
//                            //инсерт в invoice и invoice_stock
//                            QSqlQuery* addinvoice = new QSqlQuery(*DB_);
//                            id = 1;
//                            f =1;
//                            while (f) {
//                                addinvoice->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = " + QString::number(id));
//                                if(addinvoice->next()){
//                                    id++;
//                                }
//                                else {
//                                    f=0;
//                                }
//                            }
//                            addinvoice->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"')");

//                            addinvoice->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(-count)+"','"+stock+"')");

//                            QSqlQuery *upd = new QSqlQuery(*DB_);
//                            upd->exec("UPDATE `ingredients_stock` SET `amount_ingredient`= 0  WHERE `ingredients_stock`.`id_stock` = "+stock);
//                            count -= ing_am;

//                        }
//                        else {
//                            int sub = ing_am - count;
//                            QString stock = qry->value(0).toString(); // id
//                            QSqlQuery* addinvoice = new QSqlQuery(*DB_);
//                            id = 1;
//                            f =1;
//                            while (f) {
//                                addinvoice->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = " + QString::number(id));
//                                if(addinvoice->next()){
//                                    id++;
//                                }
//                                else {
//                                    f=0;
//                                }
//                            }
//                            addinvoice->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+QString::number(id)+"','"+obj["date"].toString()+"')");

//                            addinvoice->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+QString::number(id)+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(-count)+"','"+stock+"')");

//                            QSqlQuery *upd = new QSqlQuery(*DB_);
//                            upd->exec("UPDATE `ingredients_stock` SET `amount_ingredient`= "+QString::number(sub)+"  WHERE `ingredients_stock`.`id_stock` = "+qry->value(0).toString());
//                            count = 0;
//                        }
//                    }
//                }
//            }

//        }


//        return QString ("YES");
//    }
//    else {
//        return QString ("NO");
//    }

//}

//QString PostRequestHandler::addInvoiceHandler(QString post)
//{
//    //{"date":"2019-02-13","stock":"Подвал","id":"982", "ingredients": [{"title":"Морковь","amount":20},...]}

//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QString stock = obj["stock"].toString();
//        query->exec("SELECT `invoice`.`id_invoice` FROM `invoice` WHERE `invoice`.`id_invoice` = "+obj["id"].toString());
//        if(query->next()){
//            return QString("Invoice exsist");
//        }
//        else {

//            query->exec("INSERT INTO `invoice`(`id_invoice`, `date_invoice`) VALUES ('"+obj["id"].toString()+"','"+obj["date"].toString()+"')");

//            QJsonArray ingredients = obj["ingredients"].toArray();
//            for (int i=0;i<ingredients.size();i++) {
//                QString ing_title = ingredients[i].toObject().value("title").toString();
//                int amount = ingredients[i].toObject().value("amount").toInt();
//                query->exec("INSERT INTO `invoice_stock`(`id_invoice`, `id_ingredient`, `amount_ingredient`, `id_stock`) VALUES ('"+obj["id"].toString()+"',(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient` = \""+ing_title+"\" ),'"+QString::number(amount)+"',(SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock`= \""+stock+"\"))");

//                QSqlQuery* qry = new QSqlQuery(*DB_);
//                qry->exec("SELECT * FROM `ingredients_stock` WHERE `ingredients_stock`.`id_ingredient` = (SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient`= \""+ing_title+"\" ) AND `ingredients_stock`.`id_stock` = (SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock` = \""+stock+"\")");
//                if(qry->next()){
//                    while (qry->next()) {
//                        QString id_ing = qry->record().value(1).toString();
//                        QString id_stock = qry->record().value(0).toString();
//                        int count = qry->value(2).toInt() + amount;
//                        qDebug() << id_ing << " " << id_stock << " " << count;
//                        qry->exec("UPDATE `ingredients_stock` SET `amount_ingredient` = "+QString::number(count)+" WHERE (`ingredients_stock`.`id_stock` = \""+id_stock+"\")  AND (`ingredients_stock`.`id_ingredient` = \""+id_ing+"\") ");
//                    }
//                }
//                else {
//                    qry->exec("INSERT INTO `ingredients_stock`(`id_stock`, `id_ingredient`, `amount_ingredient`) VALUES ((SELECT `stock`.`id_stock` FROM `stock` WHERE `stock`.`title_stock` = \""+stock+"\"),(SELECT `ingredients`.`id_ingredient` FROM `ingredients` WHERE `ingredients`.`title_ingredient`= \""+ing_title+"\"),'"+QString::number(amount)+"')");
//                }
//            }
//            return QString("YES");
//        }
//    }
//    else {
//        return QString("NO");
//    }


//}

//QString PostRequestHandler::deleteTableHandler(QString post)
//{
//    //{"table":"name"}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE 0");
//        return QString ("YES");
//    }
//    else {
//        return QString ("NO");
//    }
//}

//QString PostRequestHandler::deleteByIdHandler(QString post)
//{
//    //{"table":"name,"id":"1"}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QString field;
//        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
//        field = query->record().fieldName(0);
//        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE `"+field+"` = "+obj["id"].toString());
//        return QString("YES");
//    }
//    else {
//        return QString("NO");
//    }
//}

//QString PostRequestHandler::deleteByWhereHandler(QString post)
//{
//    //{"table":"name","where1":" ","where2":" "}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QString field1;
//        QString field2;
//        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
//        field1 = query->record().fieldName(0);
//        field2 = query->record().fieldName(1);
//        query->exec("DELETE FROM `"+obj["table"].toString()+"` WHERE `"+field1+"` = "+obj["where1"].toString()+" AND `"+field2+"` = "+obj["where2"].toString());
//        return QString("YES");
//    }
//    else {
//        return QString("NO");
//    }
//}

//QString PostRequestHandler::insertHandler(QString post)
//{
//    //{"table":"name","val":[1,2,3...]}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QJsonArray val = obj["values"].toArray();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QStringList list;
//        query->exec("pragma table_info("+obj["table"].toString()+")");
//        while (query->next()) {
//            list.append(query->value(1).toString());
//        }
//        QString tamplate = "INSERT INTO `"+obj["table"].toString()+"`(";
//        for (int i=0;i<list.size();i++) {
//            if (i!=list.size()-1)
//                tamplate+="`"+list.at(i)+"`,";
//            else {
//                tamplate+="`"+list.at(i)+"`) VALUES (";
//            }
//        }
//        for (int i=0;i<val.size();i++) {
//            if(i!=val.size()-1){
//                tamplate+="'"+val[i].toString()+"',";
//            }
//            else {
//                tamplate+="'"+val[i].toString()+"')";
//            }
//        }
//        query->exec(tamplate);
//        return QString("YES");
//    }
//    else {
//        return QString("NO");
//    }
//}

//QString PostRequestHandler::updateByIdHandler(QString post)
//{
//    //{"table":"name","val":[1,2,3...],"id":"1"}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QJsonArray val = obj["values"].toArray();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QStringList list;
//        query->exec("pragma table_info("+obj["table"].toString()+")");
//        while (query->next()) {
//            list.append(query->value(1).toString());
//        }
//        QString tamplate = "UPDATE `"+ obj["table"].toString()+"` SET ";
//        for (int i=0;i<list.size();i++) {
//            if (i!=list.size()-1)
//                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"',";
//            else {
//                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"' WHERE `"+list.at(0)+"` = "+obj["id"].toString();
//            }
//        }
//        query->exec(tamplate);
//        return QString("YES");
//    }
//    else {
//        return QString("NO");
//    }
//}

//QString PostRequestHandler::updateByWhereHandler(QString post)
//{
//    //{"table":"name","val":[1,2,3...],"where1":"1","where2":"1"}
//    if(post!=""){
//        QJsonDocument doc = QJsonDocument::fromJson(post.toLocal8Bit());
//        QJsonObject obj = doc.object();
//        QJsonArray val = obj["values"].toArray();
//        QSqlQuery *query = new QSqlQuery(*DB_);
//        QStringList list;
//        query->exec("pragma table_info("+obj["table"].toString()+")");
//        while (query->next()) {
//            list.append(query->value(1).toString());
//        }
//        query->exec("SELECT * FROM `"+obj["table"].toString()+"`");
//        QString field1 = query->record().fieldName(0);
//        QString field2 = query->record().fieldName(1);
//        QString tamplate = "UPDATE `"+ obj["table"].toString()+"` SET ";
//        for (int i=0;i<list.size();i++) {
//            if (i!=list.size()-1)
//                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"',";
//            else {
//                tamplate+="`"+list.at(i)+"` = '" +val[i].toString()+"' WHERE `"+field1+"` = "+obj["where1"].toString()+" AND `"+field2+"` = "+obj["where2"].toString();
//            }
//        }
//        query->exec(tamplate);
//        return QString("YES");
//    }
//    else {
//        return QString("NO");
//    }
//}
#define CommentEnd }
