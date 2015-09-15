#ifndef GSAVEOPTION_H
#define GSAVEOPTION_H
#include <QString>
#include <QList>


class GSaveType
{
public:
    GSaveType(QString n,  QList<QString> *f) {name = n; format = f;};
    ~GSaveType(){};
    QString getName() const {return name;};
    QList<QString> *getFormat() const {return format;};
private:
    QString name;
    QList<QString> *format;
};




class GSaveOption
{
public:
    GSaveOption(QList<GSaveType> t) {type = t;};
    ~GSaveOption(){};
    static QList<QString> imageFormat, tableFormat, textFormat, treeFormat;
    QList<GSaveType> getType() {
        return type;
    }
private:
    QList<GSaveType> type;
};

#endif // GSAVEOPTION_H
