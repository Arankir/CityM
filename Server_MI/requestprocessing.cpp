#include "requestprocessing.h"

RequestProcessing::RequestProcessing(qintptr a_socket_id, QSqlDatabase*  a_db, QString a_apiKey, QString a_bazeUrl): _socket_id(a_socket_id),_db(a_db),_apiKey(a_apiKey),_bazeUrl(a_bazeUrl){
    qDebug() << "client "+ QString::number(a_socket_id) +" connected";
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

void RequestProcessing::run(){
    _socket = new QTcpSocket;
    _socket->setSocketDescriptor(_socket_id);
    _socket->waitForReadyRead(500);
    Responce();
    _socket->waitForBytesWritten(500);
    //_socket->disconnectFromHost();
    _socket->close();
    delete _socket;
}


void RequestProcessing::Responce(){
    QString ask(_socket->readAll());
    if(ask.isEmpty()) return;
    //qDebug() << ask;
    _request=new Request(ask);
    if(_request->GetType()=="GET"){
        GetRequest();
    }
    if(_request->GetType()=="POST"){
        PostRequest();
    }
}

void RequestProcessing::GetRequest(){
    GetRequestHandler handler(_db,_request);
    QString response;
    qDebug() << _request->GetPath() << " client " << _socket_id<<"\n"<<_request->GetPath().indexOf("/fuelup/v2/ping?apikey="+_apiKey);
    if(_request->GetPath()=="/fuelup/v2/station?apikey="+_apiKey)
        response=handler.stationHandler();
    if(_request->GetPath()=="/fuelup/v2/price?apikey="+_apiKey)
        response=handler.priceHandler();
    if(_request->GetPath().indexOf("/fuelup/v2/ping?apikey="+_apiKey)>-1)
        response=handler.pingHandler(_request->GetPath().mid(_request->GetPath().indexOf("&stationId=")+11,_request->GetPath().indexOf("&stationId=")+11-_request->GetPath().indexOf("&columnId=")),
                                     _request->GetPath().mid(_request->GetPath().indexOf("&columnId=")));
    // /fuelup/v2/station?apikey={apikey}
    // /fuelup/v2/price?apikey={apikey}
    // /fuelup/v2/ping?apikey={apikey}&stationId={stationId}&columnId={columnId}
    if(response.isEmpty()){
        _socket->write("HTTP/1.1 404 \r\n\r\nBad request");
    }
    else {
        response ="HTTP/1.1 200 OK \r\nContent-Type: application/json\r\n\r\n" + response;
        _socket->write(response.toLocal8Bit());
    }

}

void RequestProcessing::PostRequest(){
    PostRequestHandler handler(_db,_request);
    QString response;
    qDebug() << _request->GetPath() << " client " << _socket_id;
    if(_request->GetPath()=="/fuelup/v2/order?apikey="+_apiKey)
        response=handler.orderHandler(_request->GetPost());
    // /fuelup/v2/order?apikey={apikey}
    if(response.isEmpty()){
        _socket->write("HTTP/1.1 404 \r\n\r\nBad request");
    }
    else {
        _socket->write(response.toLocal8Bit());
    }
}

RequestProcessing::~RequestProcessing(){

}
