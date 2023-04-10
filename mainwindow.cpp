#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>

#include "classes.h"

using namespace std;

// TODO
// убрать отладочную печать


// поля, используемые в GUI

// массив textEdit
QTextEdit* queueTEs[7];

string weekDays[] = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"};

// здесь все те же поля, что в Model
int margin;
int browserNum=0;

// цены??
int N;
int K;

int step = 30;
int currT = 0;
int hrs = 0;
int mins = 0;
QString hrs_str = "00";
QString mins_str = "00";


Model* mod;

void timePanelUpdate() {
    // отображаем время...

    currT = mod->currTime[1];
    cout << "mins: " << currT << endl;
    // сутки кончились

    hrs = currT / 60;
    mins =  currT % 60;

    if (hrs > 23) {
        hrs = 0;
    }

    if (mins < 10) {
        mins_str = QString::fromStdString("0" + to_string(mins));
    }
    else {
        mins_str = QString::fromStdString(to_string(mins));
    }

    if (hrs < 10) {
        hrs_str = QString::fromStdString("0" + to_string(hrs));
    }
    else {
        hrs_str = QString::fromStdString(to_string(hrs));
    }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    // окно появляется

    ui->setupUi(this);
    setWindowTitle("Бензозаправка");
    ui->comboBox->addItem("10 мин",QVariant(10));
    ui->comboBox->addItem("30 мин",QVariant(30));
    ui->comboBox->addItem("1 час",QVariant(60));

    // disabling Step and "Till the end" button
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);

    queueTEs[0] = ui->q1;
    queueTEs[1] = ui->q2;
    queueTEs[2] = ui->q3;
    queueTEs[3] = ui->q4;
    queueTEs[4] = ui->q5;
    queueTEs[5] = ui->q6;
    queueTEs[6] = ui->q7;

    for (int i=0; i<7; i++) {
        queueTEs[i]->setEnabled(false);
        queueTEs[i]->setVisible(false);
        queueTEs[i]->setFontPointSize(24);
    }

    // не отображаем время и день недели
    ui->label->setEnabled(false);
    ui->weekDayLabel->setText(QString::fromStdString(weekDays[0]));
    ui->weekDayLabel->setEnabled(false);

}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_pushButton_clicked() // "Выход"
{
   this->close();
   cout << "Выход" << endl;
}


void MainWindow::on_pushButton_2_clicked() // "Начать"
{



   //enabling Step and "Till the end" buttons
   ui->pushButton_3->setEnabled(true);
   ui->pushButton_4->setEnabled(true);

   ui->pushButton_2->setEnabled(false); // disabling Start button

   // деактивируем поля ввода параметров...
   ui->spinBox->setEnabled(false);
   ui->spinBox_2->setEnabled(false);
   ui->comboBox->setEnabled(false);
   ui->lineEdit->setEnabled(false);

   cout << "Начать" << endl;

   // Initialize()...
   N = ui->spinBox->value();
   K = ui->spinBox_2->value();
   step = ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt();
   //step = 59;
   margin = ui->lineEdit->text().toInt();
   cout << "N = " << N;
   cout << "K = " << K;
   cout << "step = " << step << endl;
   cout << "margin = " << margin << endl;


   for (int i=0; i<K; i++) {

       queueTEs[i]->setEnabled(true);
       queueTEs[i]->setVisible(true);
       queueTEs[i]->setText(QString::fromStdString(to_string(0)));
   }

   //mod = new Model(60,4,5,13);
   mod = new Model(step,N,K,3);
   mins = mod->currTime[1];
   cout << "MINS at start: " << mins << endl;
   // ...Initialize()



   // отображаем время...
   ui->label->setEnabled(true);

   // ...и день недели
   ui->weekDayLabel->setEnabled(true);
   ui->weekDayLabel->setText(QString::fromStdString(weekDays[mod->currTime[0]]));

}

void MainWindow::on_pushButton_3_clicked() // "Шаг"
{
   if (ui->comboBox->isEnabled()){
       ui->comboBox->setEnabled(false);
   }

   cout << "Шаг" << endl;


   if (mod->currTime[0]<daysInWeek-1) {
       mod->Tick();

       // отображаем день недели
       ui->weekDayLabel->setText(QString::fromStdString(weekDays[mod->currTime[0]]));

       timePanelUpdate();
       ui->label->setText(hrs_str+":"+mins_str); // TODO move to timePanelUpdate
   }

    // updating the queue numbers in GUI
    for (int i=0; i<K; i++) {
        queueTEs[i]->setText(QString::fromStdString(to_string(mod->gasStation.cols[i].queueNum())));
    }
}


void MainWindow::on_pushButton_4_clicked() // "До конца"
{
   cout << "До конца" << endl;
   // проходим до конца
   int stepsToEnd = (minsInDay*7 - (mod->currTime[0]*minsInDay+mod->currTime[1]))/step;
   for (int i = 0; i < stepsToEnd; i++) {
       mod->Tick();
       timePanelUpdate();
       ui->label->setText(hrs_str+":"+mins_str);
   }

   // updating the queue numbers in GUI
   for (int i=0; i<K; i++) {
       queueTEs[i]->setText(QString::fromStdString(to_string(mod->gasStation.cols[i].queueNum())));
   }

   // отображаем день недели
   ui->weekDayLabel->setText(QString::fromStdString(weekDays[mod->currTime[0]]));

   mod->GetStats();
   ui->profitText->setText(QString::fromStdString(to_string(mod->overallProfit)));

   ui->va11->setText(QString::fromStdString(to_string(mod->avgVolume[0])));
   ui->va12->setText(QString::fromStdString(to_string(mod->avgVolume[1])));
   ui->vb11->setText(QString::fromStdString(to_string(mod->avgVolume[2])));
   ui->vb12->setText(QString::fromStdString(to_string(mod->avgVolume[3])));

   ui->textEdit_2->setText(QString::fromStdString(to_string(mod->numAccepted)));
   ui->textEdit_3->setText(QString::fromStdString(to_string(mod->numDeclined)));
}


void MainWindow::on_comboBox_activated(int index)
{


}


void MainWindow::on_pushButton_5_clicked() // "Заново"
{
    cout << "Заново" << endl;


    // disabling Step and "Till the end" button
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);

    // убираем с экрана очереди
    for (int i=0; i<7; i++) {
        queueTEs[i]->setEnabled(false);
        queueTEs[i]->setVisible(false);
        queueTEs[i]->setFontPointSize(24);
    }

    // не отображаем время и день недели
    ui->label->setEnabled(false);
    ui->weekDayLabel->setText(QString::fromStdString("ПН"));
    ui->weekDayLabel->setEnabled(false);

    ui->pushButton_2->setEnabled(true); // enabling Start button

    // активируем поля ввода параметров...
    ui->spinBox->setEnabled(true);
    ui->spinBox_2->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->lineEdit->setEnabled(true);

}