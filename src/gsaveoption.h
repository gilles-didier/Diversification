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
