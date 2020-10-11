/*
 * Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#include "dlgconnhidwifiwpa.h"
#include "ui_dlgconnhidwifiwpa.h"
#include "kylinheadfile.h"
#include "src/backthread.h"
#include "src/mainwindow.h"

#include <unistd.h>
#include <stdlib.h>

#include <QDir>

DlgConnHidWifiWpa::DlgConnHidWifiWpa(int type, MainWindow *mainWindow, QWidget *parent) :
    isUsed(type),
    QDialog(parent),
    ui(new Ui::DlgConnHidWifiWpa)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowIcon(QIcon::fromTheme("kylin-network", QIcon(":/res/x/setup.png")) );
    //需要添加 void paintEvent(QPaintEvent *event) 函数

    QPainterPath path;
    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRoundedRect(rect, 6, 6);
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));

//    KylinDBus mkylindbus;
//    double trans = mkylindbus.getTransparentData();
//    QString strTrans;
//    strTrans =  QString::number(trans, 10, 2);
//    QString sty = "QWidget{border-radius:6px;background-color:rgba(19,19,20," + strTrans + ");border:1px solid rgba(255, 255, 255, 0.05);}";
//    this->setStyleSheet(sty);
    //this->setStyleSheet("QWidget{border-radius:6px;background-color:rgba(19,19,20,0.7);border:1px solid rgba(255, 255, 255, 0.05);}");

    MyQss objQss;

//    ui->lbBoder->setStyleSheet("QLabel{border-radius:6px;background-color:rgba(19,19,20,0.95);border:1px solid rgba(255, 255, 255, 0.05);}");
    ui->lbBoder->hide();
//    ui->lbLeftupTitle->setStyleSheet("QLabel{border:0px;font-size:20px;color:rgba(255,255,255,0.97);background-color:transparent;}");
//    ui->lbConn->setStyleSheet(objQss.labelQss);
//    ui->lbNetName->setStyleSheet(objQss.labelQss);
//    ui->lbSecurity->setStyleSheet(objQss.labelQss);
//    ui->lbPassword->setStyleSheet(objQss.labelQss);

//    ui->cbxConn->setStyleSheet(objQss.cbxQss);
//    ui->cbxConn->setView(new  QListView());
//    ui->leNetName->setStyleSheet(objQss.leQss);
//    ui->lePassword->setStyleSheet(objQss.leQss);
//    ui->cbxSecurity->setStyleSheet(objQss.cbxQss);
//    ui->cbxSecurity->setView(new  QListView());
    ui->checkBoxPwd->setStyleSheet(objQss.checkBoxQss);

//    ui->btnCancel->setStyleSheet(objQss.btnCancelQss);
//    ui->btnConnect->setStyleSheet(objQss.btnConnQss);
    ui->lineUp->setStyleSheet(objQss.lineQss);
    ui->lineDown->setStyleSheet(objQss.lineQss);
    ui->btnCancel->setFocusPolicy(Qt::NoFocus);
    ui->checkBoxPwd->setFocusPolicy(Qt::NoFocus);


    ui->lbLeftupTitle->setText(tr("Add Hidden Wi-Fi")); //加入隐藏Wi-Fi
    ui->lbConn->setText(tr("Connection")); //连接设置:
    ui->lbNetName->setText(tr("Wi-Fi name")); //网络名称:
    ui->lbSecurity->setText(tr("Wi-Fi security")); //Wi-Fi 安全性:
    ui->lbPassword->setText(tr("Password")); //密码:
    ui->btnCancel->setText(tr("Cancel")); //取消
    ui->btnConnect->setText(tr("Connect")); //连接

    ui->cbxConn->addItem(tr("C_reate…")); //新建...
    QString tmpPath = "/tmp/kylin-nm-connshow-" + QDir::home().dirName();
    QString cmd = "nmcli connection show > " + tmpPath;
    int status = system(cmd.toUtf8().data());
    if (status != 0) {
        syslog(LOG_ERR, "execute 'nmcli connection show' in function 'DlgConnHidWifiWpa' failed");
    }
    QFile file(tmpPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Can't open the file!";
    }
    QString txt = file.readAll();
    QStringList txtLine = txt.split("\n");
    file.close();
    foreach (QString line, txtLine) {
        if (line.indexOf("wifi") != -1 || line.indexOf("802-11-wireless") != -1) {
            QStringList subLine = line.split(" ");
            ui->cbxConn->addItem(subLine[0]);
        }
    }
    ui->cbxConn->setCurrentIndex(0);
    connect(ui->cbxConn,SIGNAL(currentIndexChanged(QString)),this,SLOT(changeWindow()));

    ui->cbxSecurity->addItem(tr("None")); //无
    ui->cbxSecurity->addItem(tr("WPA & WPA2 Personal")); //WPA 及 WPA2 个人
    //ui->cbxSecurity->addItem(tr("WEP 40/128-bit Key (Hex or ASCII)")); //WEP 40/128 位密钥(十六进制或ASCII)
    //ui->cbxSecurity->addItem(tr("WEP 128-bit Passphrase")); //WEP 128 位密码句
    //ui->cbxSecurity->addItem("LEAP");
    //ui->cbxSecurity->addItem(tr("Dynamic WEP (802.1X)")); //动态 WEP (802.1x)
    //ui->cbxSecurity->addItem(tr("WPA & WPA2 Enterprise")); //WPA 及 WPA2 企业
    ui->cbxSecurity->setCurrentIndex(1);
    connect(ui->cbxSecurity,SIGNAL(currentIndexChanged(QString)),this,SLOT(changeDialog()));

    if (isUsed == 0){
        ui->btnConnect->setEnabled(false);
    } else {
        ui->cbxConn->setCurrentIndex(isUsed);
        ui->leNetName->setText(ui->cbxConn->currentText());
        ui->leNetName->setEnabled(false);
        ui->lbNetName->setEnabled(false);
        ui->lbSecurity->setEnabled(false);
        ui->cbxSecurity->setEnabled(false);
        ui->lbPassword->setEnabled(false);
        ui->lePassword->setText("<Hidden>");
        ui->lePassword->setEnabled(false);
        ui->btnConnect->setEnabled(true);
        ui->checkBoxPwd->setEnabled(false);
    }

    this->setFixedSize(432,397);

    this->mw = mainWindow;

    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));
}

DlgConnHidWifiWpa::~DlgConnHidWifiWpa()
{
    delete ui;
}

void DlgConnHidWifiWpa::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        this->isPress = true;
        this->winPos = this->pos();
        this->dragPos = event->globalPos();
        event->accept();
    }
}
void DlgConnHidWifiWpa::mouseReleaseEvent(QMouseEvent *event){
    this->isPress = false;
}
void DlgConnHidWifiWpa::mouseMoveEvent(QMouseEvent *event){
    if(this->isPress){
        this->move(this->winPos - (this->dragPos - event->globalPos()));
        event->accept();
    }
}

//切换到其他Wi-Fi安全类型
void DlgConnHidWifiWpa::changeDialog()
{
    if(ui->cbxSecurity->currentIndex()==0){
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifi *connHidWifi = new DlgConnHidWifi(0,mw);
        connHidWifi->show();
        connect(connHidWifi, SIGNAL(reSetWifiList() ), mw, SLOT(on_btnWifiList_clicked()) );
    } else if(ui->cbxSecurity->currentIndex()==1) {
        qDebug()<<"it's not need to change dialog";
    } else if(ui->cbxSecurity->currentIndex()==2) {
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiWep *connHidWifiWep = new DlgConnHidWifiWep(0);
        connHidWifiWep->show();
    } else if(ui->cbxSecurity->currentIndex()==3) {
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiWep *connHidWifiWep = new DlgConnHidWifiWep(1);
        connHidWifiWep->show();
    } else if(ui->cbxSecurity->currentIndex()==4) {
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiLeap *connHidWifiLeap = new DlgConnHidWifiLeap();
        connHidWifiLeap->show();
    } else if(ui->cbxSecurity->currentIndex()==5) {
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecTls *connHidWifiSecTls = new DlgConnHidWifiSecTls(0);
        connHidWifiSecTls->show();
    } else {
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifiSecTls *connHidWifiSecTls = new DlgConnHidWifiSecTls(1);
        connHidWifiSecTls->show();
    }
}

//同一 Wi-Fi安全类型的窗口变换
void DlgConnHidWifiWpa::changeWindow()
{
    if (ui->cbxConn->currentIndex() == 0){
        QApplication::setQuitOnLastWindowClosed(false);
        this->hide();
        DlgConnHidWifi *connHidWifi = new DlgConnHidWifi(0, mw);
        connHidWifi->show();
        connect(connHidWifi, SIGNAL(reSetWifiList() ), mw, SLOT(on_btnWifiList_clicked()) );
    }else if (ui->cbxConn->currentIndex() >= 1){
        QString tmpPath = "/tmp/kylin-nm-connshow-" + QDir::home().dirName();
        QString currStr = "nmcli connection show " + ui->cbxConn->currentText() + " > " + tmpPath;

        int status = system(currStr.toUtf8().data());
        if(status != 0){
            syslog(LOG_ERR, "execute 'nmcli connection show' in function 'changeWindow' failed");
        }
        QFile file(tmpPath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug()<<"Can't open the file!";
        }
        QString txt = file.readAll();
        file.close();
        if (txt.indexOf("802-11-wireless-security.key-mgmt:") != -1){
            isUsed = ui->cbxConn->currentIndex();
            ui->lbNetName->setEnabled(false);
            ui->leNetName->setText(ui->cbxConn->currentText());
            ui->leNetName->setEnabled(false);
            ui->lbSecurity->setEnabled(false);
            ui->cbxSecurity->setEnabled(false);
            ui->lePassword->setText("<Hidden>");
            ui->lbPassword->setEnabled(false);
            ui->lePassword->setEnabled(false);
            ui->btnConnect->setEnabled(true);
        }else {
            QApplication::setQuitOnLastWindowClosed(false);
            this->hide();
            DlgConnHidWifi *connHidWifi = new DlgConnHidWifi(ui->cbxConn->currentIndex(), mw);
            connHidWifi->show();
            connect(connHidWifi, SIGNAL(reSetWifiList() ), mw, SLOT(on_btnWifiList_clicked()) );
        }
    }
}

void DlgConnHidWifiWpa::on_btnCancel_clicked()
{
    this->close();
}

void DlgConnHidWifiWpa::on_btnConnect_clicked()
{
    QThread *t = new QThread();
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(t, SIGNAL(started()), this, SLOT(slotStartLoading()));
    connect(this, SIGNAL(stopSignal()), t, SLOT(quit()));
    t->start();

    QString wifiName = ui->leNetName->text();
    QString wifiPassword = ui->lePassword->text();
    BackThread *bt = new BackThread();
    strWifiname = wifiName;
    strWifiPassword = wifiPassword;
    if (isUsed == 0){
        int x = 0;
        do{
            sleep(1);
            QString tmpPath = "/tmp/kylin-nm-btoutput-" + QDir::home().dirName();
            QString cmd = "nmcli device wifi connect " + wifiName + " password " + wifiPassword + " hidden yes > " + tmpPath;

            int status = system(cmd.toUtf8().data());
            if (status != 0) {
                syslog(LOG_ERR, "execute 'nmcli device wifi connect' in function 'on_btnConnect_clicked' failed");
            }

            QFile file(tmpPath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug()<<"Can't open the file!"<<endl;
            }
            QString text = file.readAll();
            file.close();
            if(text.indexOf("Scanning not allowed") != -1){x = 1;} else { x = 0;}
        } while(x == 1);

        connect(this, SIGNAL(execSecConn()), this,SLOT(on_execSecConn() ));
        QTimer::singleShot(5*1000, this, SLOT(on_execSecConn() ));
    } else {
        bt->execConnWifi(wifiName);
        connect(this, SIGNAL(sendMessage()), this,SLOT(emitSignal() ));
        QTimer::singleShot(4*1000, this, SLOT(emitSignal() ));
    }
    this->close();
}

void DlgConnHidWifiWpa::on_checkBoxPwd_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->lePassword ->setEchoMode(QLineEdit::Password);
    } else {
        ui->lePassword->setEchoMode(QLineEdit::Normal);
    }
}

void DlgConnHidWifiWpa::on_leNetName_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == "" || ui->lePassword->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        ui->btnConnect->setEnabled(true);
    }
}

void DlgConnHidWifiWpa::on_lePassword_textEdited(const QString &arg1)
{
    if (ui->leNetName->text() == "" || ui->lePassword->text() == ""){
        ui->btnConnect->setEnabled(false);
    } else {
        ui->btnConnect->setEnabled(true);
    }
}

void DlgConnHidWifiWpa::slotStartLoading()
{
    mw->startLoading();
}

void DlgConnHidWifiWpa::on_execSecConn()
{
    QString str = "nmcli device wifi connect " + strWifiname + " password " + strWifiPassword;
    int status = system(str.toUtf8().data());
    if (status != 0){ syslog(LOG_ERR, "execute 'nmcli device wifi connect' in function 'on_execSecConn' failed");}
    connect(this, SIGNAL(sendMessage()), this,SLOT(emitSignal() ));
    QTimer::singleShot(3*1000, this, SLOT(emitSignal() ));
}

void DlgConnHidWifiWpa::emitSignal()
{
    emit reSetWifiList();
    mw->stopLoading();
    emit this->stopSignal();
}

void DlgConnHidWifiWpa::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QRect rect = this->rect();
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(opt.palette.color(QPalette::Base));
    //p.setOpacity(1);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, 6, 6);
    QWidget::paintEvent(event);
}
