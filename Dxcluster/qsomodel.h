#ifndef MODEL_H
#define MODEL_H

#include <QtSql>

class QsoModel : public QSqlRelationalTableModel
{
    Q_OBJECT

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        switch(role)
        {
        case Qt::BackgroundRole:
        {
            int status = record(index.row()).value("Status").toInt();

            if (index.column() == 4 && status == 1)
            {
                return QBrush(Qt::green);
            }
            else if (index.column() == 5 && status == 2)
            {
                return QBrush(Qt::green);
            }
            else if (index.column() == 6 && status == 3)
            {
                return QBrush(Qt::green);
            }
            return QSqlRelationalTableModel::data(index, role);
        }
//        case Qt::TextAlignmentRole:
//            if (index.column() == 3)
//                return Qt::AlignRight;
//            return Qt::AlignLeft;
        default:
            return QSqlRelationalTableModel::data(index, role);
        }
    }
public:
    QsoModel() : QSqlRelationalTableModel() {}
    void initialize();
};

#endif
