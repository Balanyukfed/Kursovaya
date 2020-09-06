#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUuid>
#include <QSqlError>


//Основной класс Сервера
class TcpServer :public QTcpServer
{
Q_OBJECT
public:
    TcpServer();//Конструктор класа
    ~TcpServer();//Деструктор класса


    void StartServer();//Метод запуска сервера
    QSqlDatabase db; //Объект кработы с базой данных
public slots:
   void incomingConnection (qintptr SocketDescriptor);  //Метод открытия соединения
};

class SocketClients :public QObject
{
    Q_OBJECT
    public:
      SocketClients(QTcpSocket* mSocketClient);//Конструктор класа
      ~SocketClients();//Деструктор класса
    QTcpSocket* SocketClient;//Объявляем наш сокет для клиента

    QByteArray Data; //Байтовый массив для передачи данных

    QSqlDatabase db; //Объект работы с базой данных
    int SocketDescriptor;
public slots:
    void SocketRead(); //Готовность сокета читать данные
    void SocketDisconect();//Событие отключение клиента
};

#endif // TCPSERVER_H
