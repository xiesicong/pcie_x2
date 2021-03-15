#include "widget.h"
#include "ui_widget.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "timer.h"
#include "riffa.h"
#include <QString>
#include <QTime>
#include <QPalette>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("野火pcie测速助手");


}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_find_device_clicked()
{
    fpga_info_list info;
    int i;
    ui->textEdit->clear();
    if (fpga_list(&info) != 0) {
        ui->textEdit->setText("请安装驱动\n");
    }
    else if(info.num_fpgas==0){
        ui->textEdit->setText("未检测到pcie设备\n");
    }
    else{
        ui->textEdit->setText("Number of devices:"+QString::number(info.num_fpgas,10));
        for (i = 0; i < info.num_fpgas; i++) {

            ui->textEdit->append(QString::number(i,10)+": id:"+QString::number(info.id[i],10));
            ui->textEdit->append(QString::number(i,10)+": num_chnls:"+QString::number(info.num_chnls[i],10));
            ui->textEdit->append(QString::number(i,10)+": name:"+QString::fromUtf8(info.name[i]));
            ui->textEdit->append(QString::number(i,10)+": vendor id:"+QString::number(info.vendor_id[i],16));
            ui->textEdit->append(QString::number(i,10)+": device id:"+QString::number(info.device_id[i],16)+"\n");
         }
     }

}

void Widget::on_pushButton_2_clicked()
{
    fpga_t * fpga;
    fpga_info_list info;
    int option;
    int i;
    int id;
    int chnl;
    size_t numWords;
    int sent;
    int recvd;
    unsigned int * sendBuffer;
    unsigned int * recvBuffer;
    int read_speed;
    int wread_speed;

    if (fpga_list(&info) != 0) {
         ui->textEdit->clear();
         ui->textEdit->setText("请安装驱动\n");
        }
    else if(info.num_fpgas==0){
         ui->textEdit->clear();
         ui->textEdit->setText("未检测到pcie设备\n");
    }
    else {
        ui->textEdit->append("正在测速，请等待");
        ui->textEdit->append("数据长度1920*1080*60个32位数据");
        id = 0;
        chnl = 0;
        numWords = 1920*1080*60;
        fpga = fpga_open(id);
        sendBuffer = (unsigned int *)malloc(numWords<<2);
        recvBuffer = (unsigned int *)malloc(numWords<<2);
        if (sendBuffer == NULL) {
            goto close_error;
        }
        recvBuffer = (unsigned int *)malloc(numWords<<2);
        if (recvBuffer == NULL) {
            free(sendBuffer);
            goto close_error;
        }
        for (i = 0; i < numWords; i++) {
            sendBuffer[i] = i+1;
            recvBuffer[i] = 0;
        }

        //GET_TIME_VAL(0);
        QTime current_time =QTime::currentTime();
        int t1_second=current_time.second();
        int t1_mesc=current_time.msec();
        sent = fpga_send(fpga, chnl, sendBuffer, numWords, 0, 1, 25000);
        //GET_TIME_VAL(1);
        current_time =QTime::currentTime();
        int t2_second=current_time.second();
        int t2_mesc=current_time.msec();
        if (sent != 0) {
         // Recv the data
         recvd = fpga_recv(fpga, chnl, recvBuffer, numWords, 25000);

        }

        //GET_TIME_VAL(2);
        current_time =QTime::currentTime();
        int t3_second=current_time.second();
        int t3_mesc=current_time.msec();
        wread_speed=(sent*4.0/1024/1024/((t2_second*1000+t2_mesc - t1_second*1000-t1_mesc)/1000.0));
        read_speed=(recvd*4.0/1024/1024/((t3_second*1000+t3_mesc - t2_second*1000-t2_mesc)/1000.0));

        ui->write_speed->setText(QString::number(wread_speed,10)+"  MB/s");
        ui->read_speed->setText(QString::number(read_speed,10)+"  MB/s");
        free(sendBuffer);
        free(recvBuffer);
        fpga_close(fpga);
        ui->textEdit->append("写入耗时"+QString::number((t2_second*1000+t2_mesc - t1_second*1000-t1_mesc),10)+"  ms");
        ui->textEdit->append("读取耗时"+QString::number((t3_second*1000+t3_mesc - t2_second*1000-t2_mesc),10)+"  ms");
        ui->textEdit->append("测速结束\n");
    }
    close_error: i=0;

}
