#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SocketClient=new QTcpSocket;
    connect(SocketClient,SIGNAL(readyRead()),this,SLOT(SocketRead()));
    connect(SocketClient,SIGNAL(disconnected()),this,SLOT(SocketDisconect()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SocketRead()
{
    if(SocketClient->waitForConnected(500))
    {
        SocketClient->waitForReadyRead(500);
        Data=SocketClient->readAll();
        qDebug()<<"@SocketRead\r\n"<<Data<<"\r\n";

        QJsonParseError errJS;
        QJsonDocument Doc=QJsonDocument::fromJson(Data,&errJS);
        if(errJS.errorString().toInt()==QJsonParseError::NoError)
        {
            QString Action=Doc.object().value("action").toString();
            //Пакет авторизации
           if(Action=="auth")
           {
               if(Doc.object().value("auth").toString()=="->OK")
               {
                   ui->KEYline->setText(Doc.object().value("KEY").toString());
                   ui->pushButton_2->setEnabled(false);
               }
           }

           if(Action=="CalculateFormula")
           {
               if(Doc.object().value("CalculateFormula").toString()=="->OK")
               {
                   ui->KEYline->setText(Doc.object().value("KEY").toString());
                   ui->lineFrm->setText(QString::number(Doc.object().value("answer").toDouble()));
                   ui->height->setValue(Doc.object().value("height").toInt());
                   ui->weight->setValue(Doc.object().value("weight").toDouble());
                   ui->YearOfBirth->setDate(QDate::fromString(Doc.object().value("YearOfBirth").toString(),"yyyy-MM-dd"));
                   ui->pushButton_2->setEnabled(false);
               }
           }
        }

    }
}

void MainWindow::SocketDisconect()
{
    qDebug()<<"@SocketDisconect";
    ui->pushButton_2->setEnabled(true);

}
void MainWindow::on_pushButton_clicked()
{
    SocketClient->connectToHost("127.0.0.1",3390);
    if (SocketClient->waitForConnected(1000))
    qDebug("Connected!");

    QString STR="{\"action\":\"UpdateProfile\",\"users\":\""+ui->username->text()+"\" ,\"KEY\":\""+ui->KEYline->text()+"\",\"pwd\":\""+ui->password->text()+
                "\",\"height\":"+ui->height->text()+",\"weight\":"+QString::number(ui->weight->value())+
                ",\"YearOfBirth\":\""+ui->YearOfBirth->date().toString("yyyy-MM-dd")+"\"}";
    qDebug()<<"@SEND registration"<<STR;
    SocketClient->write(STR.toStdString().c_str());
}

void MainWindow::on_pushButton_2_clicked()
{
    SocketClient->connectToHost("127.0.0.1",3390);
    if (SocketClient->waitForConnected(1000))
    qDebug("Connected!");

    QString STR="{\"action\":\"auth\",\"users\":\""+ui->username->text()+"\",\"pwd\":\""+ui->password->text()+"\"}";
    qDebug()<<"@SEND AUTH"<<STR;
    SocketClient->write(STR.toStdString().c_str());

}

void MainWindow::on_pushButton_3_clicked()
{
    SocketClient->connectToHost("127.0.0.1",3390);
    if (SocketClient->waitForConnected(1000))
    qDebug("Connected!");

    QString STR="{\"action\":\"registration\",\"users\":\""+ui->username->text()+"\",\"pwd\":\""+ui->password->text()+
                "\",\"height\":"+ui->height->text()+",\"weight\":"+QString::number(ui->weight->value())+
                ",\"YearOfBirth\":\""+ui->YearOfBirth->date().toString("yyyy-MM-dd")+"\"}";
    qDebug()<<"@SEND registration"<<STR;

    SocketClient->write(STR.toStdString().c_str());

}

void MainWindow::on_pushButton_4_clicked()
{
    SocketClient->connectToHost("127.0.0.1",3390);
    if (SocketClient->waitForConnected(1000))
    qDebug("Connected!");

    QString STR="{\"action\":\"CalculateFormula\",\"users\":\""+ui->username->text()+"\",\"KEY\":\""+ui->KEYline->text()+"\"}";
    qDebug()<<"@SEND CalculateFormula"<<STR;
    SocketClient->write(STR.toStdString().c_str());
}
