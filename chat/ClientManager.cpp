#include "ClientManager.h"

//jak chcesz przez siec to odkomentuj i zakomentuj ta nizej
//ClientManager::ClientManager(const QHostAddress &ip, ushort port, QObject *parent) : QObject{parent}, _ip(ip), _port(port)
ClientManager::ClientManager(QHostAddress ip, ushort port, QObject *parent) : QObject{parent}, _ip(ip), _port(port)
{
    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, this, &ClientManager::connected);
    connect(_socket, &QTcpSocket::disconnected, this, &ClientManager::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &ClientManager::readyRead);
}

ClientManager::ClientManager(QTcpSocket *client, QObject *parent)
    :QObject{parent},
    _socket(client)
{
    connect(_socket, &QTcpSocket::connected, this, &ClientManager::connected);
    connect(_socket, &QTcpSocket::disconnected, this, &ClientManager::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &ClientManager::readyRead);
}

void ClientManager::connectToServer(){
    _socket->connectToHost(_ip, _port);
}

void ClientManager::disconnectFromHost(){
    _socket->disconnectFromHost();
}

void ClientManager::readyRead(){
    auto data = _socket->readAll();
    _protocol.loadData(data);
        switch (_protocol.type()){
    case ChatProtocol::Text:
            emit textMessageReceived(_protocol.message(), _protocol.receiver());
        break;
    case ChatProtocol::SetName:{
        auto prevName = _socket->property("clientname").toString();
        _socket->setProperty("clientName", name());
        emit nameChanged(prevName, name());
        break;
    }
    case ChatProtocol::SetStatus:
        emit statusChanged(_protocol.status());
        break;
    case ChatProtocol::IsTyping:
        emit isTyping();
        break;
    default:
        break;
    }
}

void ClientManager::sendMessage(QString message){
    _socket->write(_protocol.textMessage(message, name()));
}

void ClientManager::sendName(QString name)
{
    _socket->write(_protocol.setNameMessage(name));
}

void ClientManager::sendStatus(ChatProtocol::Status status)
{
    _socket->write(_protocol.setStatusMessage(status));
}

QString ClientManager::name() const
{
    auto id = _socket->property("id").toInt();
    auto name = _protocol.name().length() > 0 ? _protocol.name() : QString("Client %1").arg(id);

    return name;
}

void ClientManager::sendIsTyping()
{
    _socket->write(_protocol.isTypingMessage());
}
