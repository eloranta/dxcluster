#include "mainwindow.h"
#include <QApplication>
#include <QTcpSocket>
#include <QJsonArray>
#include <QtSql>
#include <map>

struct Qso
{
    QString country;
    bool mixed;
    bool phone;
    bool cw;
    bool data;
    bool sat;
    bool b160;
    bool b80;
    bool b40;
    bool b30;
    bool b20;
    bool b17;
    bool b15;
    bool b12;
    bool b10;
    bool b6;
    bool b2;
    Qso()
    {
        mixed = false;
        phone = false;
        cw = false;
        data = false;
        sat = false;
        b160 = false;
        b80 = false;
        b40 = false;
        b30 = false;
        b20 = false;
        b17 = false;
        b15 = false;
        b12 = false;
        b10 = false;
        b6 = false;
        b2 = false;
    }
    void setValues(const QString& country, const QString& mode, const QString& band)
    {
        this->country = country;
        mixed = true;
        if (mode == "PHONE")
            phone = true;
        else if (mode == "CW")
            cw = true;
        else if (mode == "DATA")
            data = true;

        if (band == "160M")
            b160 = true;
        else if (band == "80M")
            b80 = true;
        else if (band == "40M")
            b40 = true;
        else if (band == "30M")
            b30 = true;
        else if (band == "20M")
            b20 = true;
        else if (band == "17M")
            b17 = true;
        else if (band == "15M")
            b15 = true;
        else if (band == "12M")
            b12 = true;
        else if (band == "10M")
            b10 = true;
        else if (band == "6M")
            b6 = true;
        else if (band == "2M")
            b2 = true;
    }
};

struct Location
{
    QString call;
    QString dxcc;
    QString country;
    QString continent;
    QString locator;
    void GetCountryInfo(const QJsonArray& array)
    {
        foreach (const QJsonValue & value, array)
        {
            QRegularExpression rx(value.toObject().value("prefixRegex").toString());
            QRegularExpressionMatch match = rx.match(call);
            if (match.hasMatch())
            {
                dxcc = QString::number(value.toObject().value("entityCode").toInt());
                country = value.toObject().value("name").toString();
                continent = value.toObject().value("continent").toArray()[0].toString();
                break;
            }
        }
    }
};

struct Spot
{
    Location dx;
    Location spotter;
    QString freq;
    QString info;
    QString time;
    QString mode;
    int status;
};

class Task : public QObject
{
    Q_OBJECT
    QTcpSocket *socket;
    QJsonArray array;
    QJsonArray worked;
    std::map<QString, Qso> map;
    void ReadDxccJson();
    void ReadAdif();
    void ReadWorkedJson();
    bool bandWorked(const QString& dxcc, const QString& freq);
    QString Mode(const QString& freq);
    QString Band(const QString& freq);
public Q_SLOTS:
    void connected()
    {
        qDebug() << "connected...";
        socket->write("og3z\r\n");
    }
    void readyRead()
    {
        QString rawMessage = socket->readAll();
        // qDebug() << rawMessage;
        QStringList lines = rawMessage.split("\n");
        for (QString line : lines)
        {
            if (line.startsWith("DX de"))
            {
                Parse(array, line);
            }
        }
    }

    void bytesWritten(qint64 bytes)
    {
        Q_UNUSED(bytes)
    }
    void Parse(const QJsonArray array, QString& message)
    {
        QRegularExpression re {R"(DX de ([A-Z0-9\/]+):?\s+(\d+.\d)\s+([A-Z0-9\/]+)\s+(.*)(\d\d\d\d)Z\s?([A-R][A-R]\d\d)?)"};
        QRegularExpressionMatch match = re.match(message);
        if (!match.hasMatch())
        {
           // qDebug() << message;
            //qDebug() << "no match";
            return;
        }

        Spot spot;
        spot.spotter.call = match.captured(1);
        spot.freq         = match.captured(2);
        spot.dx.call      = match.captured(3);
        spot.info         = match.captured(4).trimmed();
        spot.time         = match.captured(5);
        spot.mode         = Mode(spot.freq);
        spot.dx.locator   = match.captured(6);

        //qDebug() << spot.time << spot.dx.call << spot.freq << spot.dx.locator << spot.spotter.call << spot.info;

        spot.dx.GetCountryInfo(array);
        spot.spotter.GetCountryInfo(array);
        spot.status = 0;

        if(Band(spot.freq) == "6M")
        {
            qDebug() << message << "spot is on 6 meters";
            //return;
        }

        if(spot.dx.dxcc == "0")
        {
            qDebug() << message << "spot is pirate";
            return;
        }

        bool dxcc_in_worked = false;
        foreach (const QJsonValue & value, worked)
        {
            if (spot.dx.dxcc == QString::number(value.toObject().value("entity").toInt()))
            {
                dxcc_in_worked = true;
                qDebug() << "DXCC " << spot.dx.country << " in worked.json";
            }
            if (spot.dx.dxcc == QString::number(value.toObject().value("entity").toInt()))
            {
                qDebug() << Band(spot.freq) << value.toObject().value("band").toString();

                if (Band(spot.freq) == value.toObject().value("band").toString())
                {
                    qDebug() << Mode(spot.freq) << value.toObject().value("mode").toString();
                    if (Mode(spot.freq) == value.toObject().value("mode").toString())
                    {
                        qDebug() << message << "spot in worked.json";
                        return;
                    }
                }
            }
        }

        if(map.find(spot.dx.dxcc) == map.end() && !dxcc_in_worked) // country not worked
        {
            spot.status = 1;
            qDebug() << message << "country not worked";
        }
        else if (!bandWorked(spot.dx.dxcc, spot.freq))
        {
            spot.status = 2;
            qDebug() << message << "band not worked";
         }
        else if ((spot.mode == "CW" && !map[spot.dx.dxcc].cw) ||
                 (spot.mode == "PHONE" && !map[spot.dx.dxcc].phone) ||
                 (spot.mode == "DATA" && !map[spot.dx.dxcc].data))
        {
            spot.status = 3;
            qDebug() << message << "mode not worked";
        }
        else
        {
            qDebug() << message << "worked";
            return;
        }

        QString params;
        params = "insert into spot (Time, Call, Dxcc, Country, Freq, Band, Mode, Continent, Spotter, SpotterDxcc, SpotterCountry, SpotterContinent, Info, Status)"
             "values('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12', '%13', '%14')";
        params = params.arg(spot.time).arg(spot.dx.call).arg(spot.dx.dxcc).arg(spot.dx.country).arg(spot.freq).arg(Band(spot.freq)).arg(spot.mode).arg(spot.dx.continent).arg(spot.spotter.call).
                arg(spot.spotter.dxcc).arg(spot.spotter.country).arg(spot.spotter.continent).arg(spot.info).arg(spot.status);
        //qDebug() << "params";

        QSqlQuery query;
        query.exec(params);

        query.exec("delete from spot where id not in (select id from spot order by id desc limit 100)");

        emit refresh();
    }
public:
    Task(QObject *parent = nullptr) : QObject(parent)
    {
        ReadDxccJson();
        ReadAdif();
        ReadWorkedJson();

        QSqlQuery query;
        query.exec(QString("create table if not exists spot ("
                   "Id integer primary key autoincrement,"
                   "Time text,"
                   "Call text,"
                   "Dxcc text,"
                   "Country text,"
                   "Freq text,"
                   "Band text,"
                   "Mode text,"
                   "Continent text,"
                   "Spotter text,"
                   "SpotterDxcc text,"
                   "SpotterCountry text,"
                   "SpotterContinent text,"
                   "Info text,"
                   "Status integer)"));

        socket = new QTcpSocket(this);

        connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
public slots:
    void makeConnection()
    {
        qDebug() << "connecting...";

        socket->disconnectFromHost();
        socket->connectToHost("ham.connect.fi", 7300);
        if(!socket->waitForConnected(5000))
        {
            qDebug() << "Error: " << socket->errorString();
            emit finished();
        }
    }
signals:
    void finished();
    void refresh();
};

#include "main.moc"

bool openDatabase(const QString& name)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(name);
    return db.open();
}

void Task::ReadDxccJson()
{
    QString jsonFile = qApp->applicationDirPath() + "/dxcc.json";
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    //qDebug() << text;

    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject object = doc.object();
    QJsonValue value = object.value("dxcc");
    array = value.toArray();
}

void Task::ReadAdif()
{
    QString jsonFile = qApp->applicationDirPath() + "/QSL.adi";
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    const long long MAX_LEN = 500;
    char data[MAX_LEN];
    long long len;
    QString mode;
    QString country;
    QString band;
    QString dxcc;
    while ((len = file.readLine(data, MAX_LEN)) > 0)
    {
        data[len] = 0;
        QString line(data);
        QRegularExpression rx("<APP_LoTW_MODEGROUP:\\d+:S>(.+)\\n");
        QRegularExpressionMatch match = rx.match(line);
        if (match.hasMatch())
        {
            mode = match.captured(1);
//            qDebug() << mode;
        }
        rx.setPattern("<BAND:\\d+>(.+)\\n");
        match = rx.match(line);
        if (match.hasMatch())
        {
            band = match.captured(1);
//            qDebug() << band;
        }
        rx.setPattern("<DXCC:\\d+>(.+)\\n");
        match = rx.match(line);
        if (match.hasMatch())
        {
            dxcc = match.captured(1);
        }

        rx.setPattern("<COUNTRY:\\d+>(.+)\\n");
        match = rx.match(line);
        if (match.hasMatch())
        {
            country = match.captured(1);
//            qDebug() << dxcc << country;

            Qso qso;
            if(map.find(dxcc) != map.end())
                qso = map[dxcc];

            qso.setValues(country, mode, band);
            map[dxcc] = qso;
        }
    }
    file.close();
}

bool Task::bandWorked(const QString& dxcc, const QString& freq)
{
    double f = freq.toDouble();
    if (f >= 1810.0 && f <= 2000.0)
        return map[dxcc].b160;
    else if (f >= 3500.0 && f <= 3800.0)
        return map[dxcc].b80;
    else if (f >= 7000.0 && f <= 7200.0)
        return map[dxcc].b40;
    else if (f >= 10100.0 && f <= 10150.0)
        return map[dxcc].b30;
    else if (f >= 14000.0 && f <= 14350.0)
        return map[dxcc].b20;
    else if (f >= 18068.0 && f <= 18168.0)
        return map[dxcc].b17;
    else if (f >= 21000.0 && f <= 21450.0)
        return map[dxcc].b15;
    else if (f >= 24890.0 && f <= 24990.0)
        return map[dxcc].b12;
    else if (f >= 28000.0 && f <= 29700.0)
        return map[dxcc].b10;
    else if (f >= 50000.0 && f <= 52000.0)
        return map[dxcc].b6;
    else if (f >= 144000.0 && f <= 146000.0)
        return map[dxcc].b2;
    else
        return false;
}

QString Task::Mode(const QString& freq)
{
    double f = freq.toDouble();

    if (f >= 1810 && f <= 1838)
        return "CW";
    else if (f >= 1838 && f <= 1843)
        return "DATA";
    else if (f >= 1843 && f <= 2000)
        return "PHONE";
    else if (f >= 3500 && f <= 3570)
        return "CW";
    else if (f >= 3570 && f <= 3600)
        return "DATA";
    else if (f >= 3600 && f <= 3800)
        return "PHONE";
    else if (f >= 7000 && f <= 7040)
        return "CW";
    else if (f >= 7040 && f <= 7080)
        return "DATA";
    else if (f >= 7080 && f <= 7200)
        return "PHONE";
    else if (f >= 10100 && f <= 10130)
        return "CW";
    else if (f >= 10130 && f <= 10150)
        return "DATA";
    else if (f >= 14000 && f <= 14070)
        return "CW";
    else if (f >= 14070 && f <= 14112)
        return "DATA";
    else if (f >= 14112 && f <= 14350)
        return "PHONE";
    else if (f >= 18068 && f <= 18095)
        return "CW";
    else if (f >= 18095 && f <= 18111)
        return "DATA";
    else if (f >= 18111 && f <= 18168)
        return "PHONE";
    else if (f >= 21000 && f <= 21070)
        return "CW";
    else if (f >= 21070 && f <= 21151)
        return "DATA";
    else if (f >= 21151 && f <= 21450)
        return "PHONE";
    else if (f >= 24890 && f < 24915)
        return "CW";
    else if (f >= 24915 && f <= 24919)
        return "DATA";
    else if (f >= 28000 && f <= 28070)
        return "CW";
    else if (f >= 28070 && f <= 28180)
        return "DATA";
    else if (f > 28180 && f <= 29700)
        return "PHONE";
    else if (f >= 50000 && f <= 50100)
        return "CW";
    else if (f >= 50313 && f <= 50313)
        return "DATA";
    else if (f >= 50100 && f <= 50500)
        return "PHONE";
    else
        return "";
}

void Task::ReadWorkedJson()
{
    QString jsonFile = qApp->applicationDirPath() + "/worked.json";
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    // qDebug() << text;

    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (doc.isNull())
    {
        qDebug() << "Error in parsing worked.json";
            return;
    }
    QJsonObject object = doc.object();
    worked = object.value("worked").toArray();
    // qDebug() << worked;
}

QString Task::Band(const QString& freq)
{
    double f = freq.toDouble();
    if (f >= 1810.0 && f <= 2000.0)
        return "160M";
    else if (f >= 3500.0 && f <= 3800.0)
        return "80M";
    else if (f >= 7000.0 && f <= 7200.0)
        return "40M";
     else if (f >= 10100.0 && f <= 10150.0)
        return "30M";
    else if (f >= 14000.0 && f <= 14350.0)
        return "20M";
    else if (f >= 18068.0 && f <= 18168.0)
        return "17M";
    else if (f >= 21000.0 && f <= 21450.0)
        return "15M";
    else if (f >= 24890.0 && f <= 24990.0)
        return "12M";
    else if (f >= 28000.0 && f <= 29700.0)
        return "10M";
    else if (f >= 50000.0 && f <= 52000.0)
        return "6M";
    else if (f >= 144000.0 && f <= 146000.0)
        return "2M";
    else
        return "";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!openDatabase("Spots.sqlite"))
    {
        qDebug() << "Cannot open database";
        return -1;
    }
    Task task(&app);
    QObject::connect(&task, SIGNAL(finished()), &app, SLOT(quit()));

    QTimer::singleShot(0, &task, SLOT(makeConnection()));

    QTimer watchDog;
    QObject::connect(&watchDog, &QTimer::timeout, &task, QOverload<>::of(&Task::makeConnection));
    watchDog.start(10*60*1000); // reconnect every 10 min

    MainWindow window;
    QObject::connect(&task, SIGNAL(refresh()), &window, SLOT(refresh()));
    window.show();
    return app.exec();
}

