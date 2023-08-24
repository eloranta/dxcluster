#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qsomodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QsoModel qsoModel;
    QString spotterContinentFilter;
    QString modeFilter;
    QString bandFilter;
private slots:
    void refresh();
    void continentChecked(int state);
    void modeChecked(int state);
    void bandChecked(int);
    void UpdateFilter();
    void clearClicked();
};
#endif
