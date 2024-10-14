#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QTableView>

class MyTableView : public QTableView
{
    Q_OBJECT
public:
    explicit MyTableView(QWidget *parent = nullptr);

signals:

};

#endif // MYTABLEVIEW_H
