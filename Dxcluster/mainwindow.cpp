#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, height());
    ui->qsoView->setModel(&qsoModel);
    qsoModel.setTable("spot");
    qsoModel.setSort(0, Qt::DescendingOrder);

    modeFilter = "(false or Mode='DATA')";
    spotterContinentFilter = "(false or SpotterContinent='EU')";
    bandFilter = "(false or Band='160M' or Band='80M' or Band='30M' or Band='20M' or Band='17M' or Band='15M' or Band='12M' or Band='10M' or Band='6M' or Band='2M')";
    UpdateFilter();

    ui->qsoView->setColumnHidden(0, true);
    ui->qsoView->setColumnHidden(3, true);
    ui->qsoView->setColumnHidden(6, true);
    ui->qsoView->setColumnHidden(8, true);
    ui->qsoView->setColumnHidden(10, true);
    ui->qsoView->setColumnHidden(11, true);
    ui->qsoView->setColumnHidden(12, true);
    ui->qsoView->setColumnHidden(14, true);

    ui->qsoView->setColumnWidth(1, 40);
    ui->qsoView->setColumnWidth(2, 150);
    ui->qsoView->setColumnWidth(4, 150);
    ui->qsoView->setColumnWidth(5, 80);
    ui->qsoView->setColumnWidth(6, 40);
    ui->qsoView->setColumnWidth(7, 40);
    ui->qsoView->setColumnWidth(8, 150);

    ui->checkBoxEU->setChecked(true);
    connect(ui->checkBoxEU, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));
    connect(ui->checkBoxAF, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));
    connect(ui->checkBoxAS, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));
    connect(ui->checkBoxOC, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));
    connect(ui->checkBoxNA, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));
    connect(ui->checkBoxSA, SIGNAL(stateChanged(int)), this, SLOT(continentChecked(int)));

    ui->checkBoxDigi->setChecked(true);
    connect(ui->checkBoxCW, SIGNAL(stateChanged(int)), this, SLOT(modeChecked(int)));
    connect(ui->checkBoxPhone, SIGNAL(stateChanged(int)), this, SLOT(modeChecked(int)));
    connect(ui->checkBoxDigi, SIGNAL(stateChanged(int)), this, SLOT(modeChecked(int)));

    connect(ui->checkBox160, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox80, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox40, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox30, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox20, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox17, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox15, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox12, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox10, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox6, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));
    connect(ui->checkBox2, SIGNAL(stateChanged(int)), this, SLOT(bandChecked(int)));

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh()
{
    qsoModel.select();
}

void MainWindow::continentChecked(int)
{
    spotterContinentFilter = "(false";
    if (ui->checkBoxEU->isChecked())
        spotterContinentFilter += " or SpotterContinent='EU'";
    if (ui->checkBoxAF->isChecked())
        spotterContinentFilter += " or SpotterContinent='AF'";
    if (ui->checkBoxAS->isChecked())
        spotterContinentFilter += " or SpotterContinent='AS'";
    if (ui->checkBoxOC->isChecked())
        spotterContinentFilter += " or SpotterContinent='OC'";
    if (ui->checkBoxNA->isChecked())
        spotterContinentFilter += " or SpotterContinent='NA'";
    if (ui->checkBoxSA->isChecked())
        spotterContinentFilter += " or SpotterContinent='SA'";
    spotterContinentFilter += ")";
    UpdateFilter();
}

void MainWindow::modeChecked(int)
{
    modeFilter = "(false";
    if (ui->checkBoxCW->isChecked())
        modeFilter += " or Mode='CW'";
    if (ui->checkBoxPhone->isChecked())
        modeFilter += " or Mode='PHONE'";
    if (ui->checkBoxDigi->isChecked())
        modeFilter += " or Mode='DATA'";
    modeFilter += ")";
    UpdateFilter();
}

void MainWindow::bandChecked(int)
{
    bandFilter = "(false";
    if (ui->checkBox160->isChecked())
        bandFilter += " or Band='160M'";
    if (ui->checkBox80->isChecked())
        bandFilter += " or Band='80M'";
    if (ui->checkBox40->isChecked())
        bandFilter += " or Band='40M'";
    if (ui->checkBox30->isChecked())
        bandFilter += " or Band='30M'";
    if (ui->checkBox20->isChecked())
        bandFilter += " or Band='20M'";
    if (ui->checkBox17->isChecked())
        bandFilter += " or Band='17M'";
    if (ui->checkBox15->isChecked())
        bandFilter += " or Band='15M'";
    if (ui->checkBox12->isChecked())
        bandFilter += " or Band='12M'";
    if (ui->checkBox10->isChecked())
        bandFilter += " or Band='10M'";
    if (ui->checkBox6->isChecked())
        bandFilter += " or Band='6M'";
    if (ui->checkBox2->isChecked())
        bandFilter += " or Band='2M'";
    bandFilter += ")";
    UpdateFilter();
}

void MainWindow::UpdateFilter()
{
    QString filter = modeFilter + " and " + bandFilter + " and " + spotterContinentFilter;
    qDebug() << filter;
    qsoModel.setFilter(filter);
    qsoModel.select();
}


void MainWindow::clearClicked()
{
    qsoModel.clear();
    qsoModel.select();
}
