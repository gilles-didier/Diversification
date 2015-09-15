#include "gutil.h"

GUtil::GUtil()
{

}

GUtil::~GUtil()
{

}

void GUtil::writeTableCSV(FILE *f, QAbstractTableModel *table)
{
    char sep = '\t';
    for(int c=0; c<table->columnCount(); c++)
        fprintf(f, "%c%s", sep, qPrintable(table->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString()));
    fprintf(f, "\n");
    for(int l=0; l<table->rowCount(); l++) {
        fprintf(f, "%s", qPrintable(table->headerData(l, Qt::Vertical, Qt::DisplayRole).toString()));
        for(int c=0; c<table->columnCount(); c++)
            fprintf(f, "%c%s", sep, qPrintable(table->data(table->index(l,c), Qt::DisplayRole).toString()));
        fprintf(f, "\n");
    }

}

void GUtil::writeTableLatex(FILE *f, QAbstractTableModel *table) {
    fprintf(f, "%%\\usepackage{adjustbox}\n%%\\usepackage{array}\n\n\\newcolumntype{R}[2]{>{\\adjustbox{angle=#1,lap=\\width-(#2)}\\bgroup}l<{\\egroup}}\n\\newcommand*\\rot{\\multicolumn{1}{R{45}{1em}}}\n");
    fprintf(f, "\\begin{tabular}{l");
    for(int c=0; c<table->columnCount(); c++)
        fprintf(f, "r");
    fprintf(f, "}\n");
    for(int c=0; c<table->columnCount(); c++)
        fprintf(f, "& \\rot{%s}", qPrintable(table->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString()));
    fprintf(f, "\\\\\n\\hline\n");
    for(int l=0; l<table->rowCount(); l++) {
        fprintf(f, "%s", qPrintable(table->headerData(l, Qt::Vertical, Qt::DisplayRole).toString()));
        for(int c=0; c<table->columnCount(); c++)
            fprintf(f, "& %s", qPrintable(table->data(table->index(l,c), Qt::DisplayRole).toString()));
        fprintf(f, "\\\\\n");
    }
    fprintf(f, "\\hline\n\\end{tabular}\n");
}
