#include "server.h"

Server::Server(QObject* parent): QTcpServer (parent), ThreadPool(new QThreadPool(this))
{
    ThreadPool->setMaxThreadCount(1);
    //qDebug() << ThreadPool->maxThreadCount();
    _db = QSqlDatabase::addDatabase("QODBC3");
    QStringList setting;
    if(QFile::exists("Setting.txt")){
        QFile settings("Setting.txt");
        if (settings.open(QIODevice::ReadOnly)){
            while(!settings.atEnd()){
                setting << QString::fromLocal8Bit(settings.readLine()).remove("\r\n").remove("\n");
            }
        } else
            qDebug()<<"Error: setting file is already open";
    } else
        qDebug()<<"Error: setting file not found";
    _db.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;Persist Security Info=true;uid=%3;pwd=%4").arg(setting[0],setting[1],setting[2],setting[3]));
    //DB_.setHostName("127.0.0.1");
    //DB_.setUserName("root");
    //DB_.setPassword("1423");
    if(_db.open()){
        _manager = new QNetworkAccessManager;
        qDebug() << "db open";
        _timer.setInterval(60000);
        QObject::connect(&_timer,SIGNAL(timeout()),this,SLOT(SendFuel()));
        _timer.start();
    }
    else {
        qDebug() << "db close";
    }
}

Server::~Server(){

}

void Server::StartServer(){
    if(this->listen(QHostAddress::Any,0)){
        qDebug() << "listen";
    } else {
        qDebug() << "not listen";
    }
}

void Server::SendFuel(){
    QStringList VCodes;
    if(QFile::exists("Requests.txt")){
        QFile requests("Requests.txt");
        if (requests.open(QIODevice::ReadOnly)){
            while(!requests.atEnd()){
                VCodes << QString::fromLocal8Bit(requests.readLine()).remove("\r\n").remove("\n");
            }
        } else
            qDebug()<<"Error: requests file is already open";
    } else
        qDebug()<<"Error: request file not found";
    QSqlQuery *query = new QSqlQuery(_db);
    query->exec("SELECT VCode, localState, APIId, [Price], [Litre], [Sum], [DateOpen], [DateClose], [Link] FROM [agzs].[dbo].PR_APITransaction where VCode in ('"+VCodes.join(", ")+"') order by CDate desc");
    if(query->next()){
        switch(query->value(1).toString().toInt()){
        case 1:{
            QNetworkRequest request(QUrl(_bazeUrl+"/api/order/accept"));
            _manager->post(request,QString("apikey="+_apiKey+"&orderId="+query->value(2).toString()).toLocal8Bit());
        }
        case 10:{
            QNetworkRequest request(QUrl(_bazeUrl+"/api/order/completed"));
            QString("apikey="+_apiKey+"&orderId="+query->value(2).toString()+"&litre="+query->value(4).toString().replace(",",".")+"&extendedOrderId="+id+"&extendedDate="+QDateTime::fromString(query->value(6).toString(),"yyyy-MM-dd hh:mm:ss.zzz").toString("dd.MM.yyyy HH:mm:ss")).toLocal8Bit();
            //litre – кол-во пролитых литров, указывается как double с разделителем точка
            //extendedOrderId – идентификатор заказа в АСУ сети
            //extendedDate – дата по которой АСУ строит отчет для сверки, формат dd.MM.yyyy HH:mm:ss
        }
        }
    }
}

void Server::incomingConnection(qintptr socketDescriptor){
    RequestProcessing* requestProcessing = new RequestProcessing(socketDescriptor,&_db,_apiKey,_bazeUrl);
    ThreadPool->start(requestProcessing);
}
