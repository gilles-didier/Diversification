#ifndef GUTIL_H
#define GUTIL_H
#include <stdio.h>
#include <QAbstractTableModel>

class GUtil
{
public:
    GUtil();
    ~GUtil();
    static void writeTableCSV(FILE *f, QAbstractTableModel *table);
    static void writeTableLatex(FILE *f, QAbstractTableModel *table);
};

#endif // GUTIL_H
