#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>

#include "classes.h"

using namespace std;

// поля, используемые в GUI

// массив textEdit
QTextEdit* queueTEs[7];


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
QString mins_str = "10";

Model* mod;

void timePanelUpdate() {
    // отображаем время...

    mins += step;
    // сутки кончились
    if ((hrs ==23) && (mins > 59)) {
        hrs = 0;
        mins = 0;
    }

    if (mins >= 60) {

        hrs += mins/60;
        mins = mins%60;
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
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Бензозаправка");
    ui->comboBox->addItem("10 мин",QVariant(10));
    ui->comboBox->addItem("30 мин",QVariant(30));
    ui->comboBox->addItem("1 час",QVariant(60));

    // "шаг" неактивна

    ui->pushButton_3->setEnabled(false);


    /* reading K from GUI (...) and generating K random nums in an arr
    ui->q1->setFontPointSize(24); // move to constructor
    ui->q1->setText(QString::fromStdString(to_string(browserNum)));
    */

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

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked() // "Выход"
{
   this->close();
   cout << "Выход" << endl;
}


void MainWindow::on_pushButton_2_clicked() // "Начать"
{
   //активируем "Шаг"
   ui->pushButton_3->setEnabled(true);
   // деактивируем поля ввода параметров...
   ui->spinBox->setEnabled(false);
   ui->spinBox_2->setEnabled(false);
   ui->comboBox->setEnabled(false);
   ui->lineEdit->setEnabled(false);
   cout << "Начать" << endl;
   N = ui->spinBox->value();
   K = ui->spinBox_2->value();
   step = ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt();
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


}

void MainWindow::on_pushButton_3_clicked() // "Шаг"
{
   if (ui->comboBox->isEnabled()){
       ui->comboBox->setEnabled(false);
       ui->pushButton_2->setEnabled(false);
   }

   cout << "Шаг" << endl;


   if (mod->currTime[0]<daysInWeek-1) {
       mod->Tick();
       cout << "weekday:" << mod->currTime[0] << endl;
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
}


void MainWindow::on_comboBox_activated(int index)
{

}

