/*
    'Diversification' and 'estimate' estimate diversification and fossilization rates frm tree shapes and fossil dates / 
    'sample' simulates random trees and fossils finds and saves them in Newick format / 
    'test' simulates random multiplex to test community detection approaches
	'complexity' simulates random trees and fossils finds, computes their complexity index and return a CSV file with lines
		<complexity index>	<likelihood computation time>	<tree size>	<fossil number>
	'assess' simulates random trees and fossils finds, estimates speciation and extinction rates and returns the mean absolute error

    Copyright (C) 2015  Gilles DIDIER

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




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
