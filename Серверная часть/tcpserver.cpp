#include "tcpserver.h"

//Конструктор для инициализации элементов класса
TcpServer::TcpServer()
{

 db = QSqlDatabase::addDatabase("QMYSQL", "mydb");
 db.setHostName("localhost");
 db.setDatabaseName("steper");
 db.setUserName("root");
 db.setPassword("Mustang2002");
 bool ok = db.open();
 qDebug()<<"@TcpServer::TcpServer() DB Connetct:"<<ok; // Выводим в дебаг статус подключения
}

//Деструктор
TcpServer::~TcpServer()
{

}

//Процедура запуска сервера
void TcpServer::StartServer()
{
   if(this->listen(QHostAddress::Any,3390)) //запуск сервера Принимать с любого адреса Порт 3390
   {
       qDebug()<<"@StartServer OK";          //Вывод удачного старта
   }
   else
   {
       qDebug()<<"@StartServer FAILD";      //Вывод неудачного запуска
   }
}

//Процедура Подключения клиента
void TcpServer::incomingConnection (qintptr SocketDescriptor)
{

    QTcpSocket* SocketClient=new QTcpSocket(this);
    SocketClient->setSocketDescriptor(SocketDescriptor);

    SocketClients* mSockket=new SocketClients(SocketClient);

    qDebug()<<"@IncomingConnection Socket client nums:"<<SocketDescriptor;

}

//Процедура Получения сообщения
void SocketClients::SocketRead()
{
    Data=SocketClient->readAll(); 

    qDebug()<<"@SocketClients::SocketRead:"<<SocketDescriptor<<"\r\n"<<Data<<"\r\n";    
    QJsonParseError errJS;
    QJsonDocument Doc=QJsonDocument::fromJson(Data,&errJS);                             
    if(errJS.errorString().toInt()==QJsonParseError::NoError)
    {

        QString Action=Doc.object().value("action").toString();

         //Пакет авторизации
        if(Action=="auth")
        {
            QSqlQuery query(db);
            query.prepare("SELECT users.login, users.KEY FROM users WHERE login=:login AND pssword=:pssword");

            query.bindValue(":login", Doc.object().value("users").toString());
            query.bindValue(":pssword", Doc.object().value("pwd").toString());

           QString req;
            if( !query.exec() )
            {
                QString err=query.lastError().text();
                req="{\"action\":\"auth\",\"auth\":\"->FAILD\",\"err\":\""+err+"\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("pwd").toString()+"\"}";
            }
            else
            {
              if (query.next())
              {
                  QString login = query.value(0).toString();
                  QString key = query.value(1).toString();
                  req="{\"action\":\"auth\",\"auth\":\"->OK\",\"KEY\":\""+key+"\",\"users\":\""+login+"\"}";
              }
              else
              {
                  req="{\"action\":\"auth\",\"auth\":\"->FAILD\",\"err\":\"NO USERS\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("pwd").toString()+"\"}";
              }

            }
            SocketClient->write(req.toStdString().c_str());
            qDebug()<<"@SocketClients::SocketRead()"<<SocketDescriptor<< "SEND\r\n"<<req<<"\r\n";
            return;
        }

        //Пакет регистрации
        if(Action=="registration")
        {
            QString usr=Doc.object().value("users").toString();

            QSqlQuery query(db);
            query.prepare("SELECT login FROM users WHERE login=:login");
            query.bindValue(":login", usr);
            query.exec();
            if (query.next())
                {
                  QString req="{\"action\":\"registration\",\"registration\":\"->FAILD\",\"err\":\"duplicates\"}";
                  SocketClient->write(req.toStdString().c_str());
                  qDebug()<<"@registration SEND PCK:"<<req;
                  qDebug()<<"@registration :->FAILD user:  "<<usr<<" duplicates";
                  return;
                }

           int height=Doc.object().value("height").toInt();
           double weight= Doc.object().value("weight").toDouble();
           query.prepare("INSERT INTO users VALUE(:login,:pssword,:height,:weight,:YearOfBirth,:KEY)");

           //Устанавливаем  параметры
           query.bindValue(":login", usr);
           query.bindValue(":pssword", Doc.object().value("pwd").toString());
           query.bindValue(":height",height );
           query.bindValue(":weight",weight);
           query.bindValue(":YearOfBirth", Doc.object().value("YearOfBirth").toString());

           QUuid u=QUuid::createUuid();
           QString UUID=u.toString();
           query.bindValue(":KEY",UUID);


           if( !query.exec() )//проверяем запрос на ошибку
                {
                    QString err=query.lastError().text();
                    qDebug() << "> Query exec() error." << err;
                    QString req="{\"action\":\"registration\",\"registration\":\"->FAILD\",\"err\":\""+err+"\"}";
                    SocketClient->write(req.toStdString().c_str());
                    qDebug()<<"@registration SEND PCK:"<<req;
                    return;
                }
           else
                {
                    qDebug() << ">Query exec() success.";
                    QString req="{\"action\":\"registration\",\"registration\":\"->OK\",,\"err\":\"OK\"}";
                    SocketClient->write(req.toStdString().c_str());
                    qDebug()<<"@registration SEND PCK:"<<req;
                    qDebug()<<"@registration :"<<usr<<" API KEY "<<UUID;
                }


        }

        //Пакет получения формулы
        if(Action=="CalculateFormula")
        {

            QSqlQuery query(db);
            query.prepare("SELECT users.login, users.KEY, users.height, users.weight, users.YearOfBirth FROM users WHERE users.login=:login AND users.KEY=:KEY");
            query.bindValue(":login", Doc.object().value("users").toString());
            query.bindValue(":KEY", Doc.object().value("KEY").toString());

           QString req;
            if( !query.exec() )
            {
                QString err=query.lastError().text();
                req="{\"action\":\"CalculateFormula\",\"CalculateFormula\":\"->FAILD\",\"err\":\""+err+"\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
            }
            else
            {
              if (query.next())
              {
                  double reqFrm=((query.value("weight").toInt()/((query.value("height").toDouble()/100)*(query.value("height").toDouble()/100)))-9);

                  QString login = query.value(0).toString();
                  QString key = query.value(1).toString();
                  req="{\"action\":\"CalculateFormula\",\"CalculateFormula\":\"->OK\",\"KEY\":\""+key+"\",\"users\":\""+login+"\""
                          +",\"height\":"       +query.value(2).toString()+
                          +",\"weight\":"       +query.value(3).toString()+
                          +",\"YearOfBirth\":\""  +query.value(4).toString()+"\""+
                          +",\"answer\":"+QString::number(reqFrm)+
                          "}";
              }
              else
              {
                  req="{\"action\":\"CalculateFormula\",\"CalculateFormula\":\"->FAILD\",\"err\":\"NO USERS\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
              }

            }

            SocketClient->write(req.toStdString().c_str());
            qDebug()<<"@SocketClients::SocketRead()"<<SocketDescriptor<< "SEND\r\n"<<req<<"\r\n";
            return;
        }

        //Пакет обновления профиля
        if(Action=="UpdateProfile")
        {
            QSqlQuery query(db);
            query.prepare("SELECT users.login, users.KEY, users.height, users.weight, users.YearOfBirth FROM users WHERE users.login=:login AND users.KEY=:KEY"); 
            query.bindValue(":login", Doc.object().value("users").toString());
            query.bindValue(":KEY", Doc.object().value("KEY").toString());
           QString req;
           if( !query.exec() )
            {
                QString err=query.lastError().text();
                req="{\"action\":\"UpdateProfile\",\"UpdateProfile\":\"->FAILD\",\"err\":\""+err+"\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
            }
            else
            {
              if (query.next())
              {
                    query.prepare("UPDATE users SET height=:height,weight=:weight,YearOfBirth=:YearOfBirth   WHERE users.login=:login AND users.KEY=:KEY");

                    query.bindValue(":height",      Doc.object().value("height").toInt());
                    query.bindValue(":weight",      Doc.object().value("weight").toDouble());
                    query.bindValue(":YearOfBirth", Doc.object().value("YearOfBirth").toString());
                    query.bindValue(":pssword",     Doc.object().value("pwd").toString());

                    query.bindValue(":login",       Doc.object().value("users").toString());
                    query.bindValue(":KEY",         Doc.object().value("KEY").toString());

                    if( !query.exec() )//проверяем запрос на ошибку
                        {
                           QString err=query.lastError().text();
                           req="{\"action\":\"UpdateProfile\",\"UpdateProfile\":\"->FAILD\",\"err\":\""+err+"\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
                        }
                     else
                        {
                            req="{\"action\":\"UpdateProfile\",\"UpdateProfile\":\"->OK\",\"err\":\"OK\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
                        }
              }
              else
              {
                  req="{\"action\":\"UpdateProfile\",\"UpdateProfile\":\"->FAILD\",\"err\":\"NO USERS\",\"users\":\""+Doc.object().value("users").toString()+"@"+Doc.object().value("KEY").toString()+"\"}";
              }

            }

            SocketClient->write(req.toStdString().c_str());
            qDebug()<<"@SocketClients::SocketRead()"<<SocketDescriptor<< "SEND\r\n"<<req<<"\r\n";
            return;
        }
    }
}

//Процедура отключения клиента
void SocketClients::SocketDisconect()
{
    qDebug()<<"@SocketClients::SocketDisconect:"<<SocketDescriptor;
    SocketClient->deleteLater();

}

SocketClients::SocketClients(QTcpSocket* mSocketClient)
{
  bool con1 = connect(mSocketClient,SIGNAL(readyRead()),this,SLOT(SocketRead()));
  qDebug()<<"@SocketClients::SocketClients SIGNAL (readyRead()) "<<con1;
   con1 = connect(mSocketClient,SIGNAL(disconnected()),this,SLOT(SocketDisconect()));
   qDebug()<<"@SocketClients::SocketClients SIGNAL (disconnected()) "<<con1;
   SocketClient=mSocketClient;   
   qDebug()<<"@SocketClients::SocketClients Socket client send: CONNECTED"<<SocketClient->socketDescriptor();
   SocketDescriptor=SocketClient->socketDescriptor();

   db = QSqlDatabase::addDatabase("QMYSQL", "mydb"+QString::number(SocketDescriptor));
   db.setHostName("localhost");
   db.setDatabaseName("steper");
   db.setUserName("root");
   db.setPassword("Mustang2002");
   bool ok = db.open();
   qDebug()<<"@SocketClients::SocketClients() DB Connetct:"<<ok;
}

SocketClients::~SocketClients()
{

}
