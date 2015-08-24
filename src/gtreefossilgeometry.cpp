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




#include "gtreefossilgeometry.h"
#include <QDebug>
GTreeFossilGeometry::GTreeFossilGeometry(TypeTree *t, TypeFossilIntFeature *f, double min, double max)
{
	tree = t;
	time = NULL;
	setMinMaxTime(min, max);
	fos = f;
	if(fos != NULL)
        line = new QLineF[fos->sizeFossil];
	else
		line = NULL;
	if(tree!=NULL) {
		computeTime();
		start = new QPointF[tree->size];
	} else
		start = NULL;
}

GTreeFossilGeometry::~GTreeFossilGeometry()
{
  delete[] line;
}

void GTreeFossilGeometry::fillBounds(int n, double tmin, double tmax, double *min, double *max, int *dmax) {
	int c;
	if(tree->time != NULL && tree->time[n] != NO_TIME) {
		min[n] = tree->time[n];
	} else {
        if(fos->status[n] == unknownNodeStatus) {
            min[n] = fos->endTimeTable[fos->endTime[n]].sup;
        } else {
            if(fos && fos->fossilInt[n] != -1) {
                int f;
                min[n] = fos->fossilIntList[fos->fossilInt[n]].fossilInt.sup;
                for(f=fos->fossilIntList[fos->fossilInt[n]].prec; f!=NOSUCH; f=fos->fossilIntList[f].prec)
                    if(fos->fossilIntList[f].fossilInt.sup>min[n])
                        min[n] = fos->fossilIntList[f].fossilInt.sup;
            } else
                min[n] = tmin;
        }
	}
    for(c=tree->node[n].child; c!=NOSUCH; c=tree->node[c].sibling)
		fillBounds(c, min[n], tmax, min, max, dmax);
	if(tree->time != NULL && tree->time[n] != NO_TIME) {
		max[n] = tree->time[n];
		dmax[n] = 0;
	} else {
        if(tree->node[n].child==NOSUCH) {
			max[n] = tmax;
			dmax[n] = 0;
		} else {
			max[n] = tmax+1;
			dmax[n] = 0;
            for(c=tree->node[n].child; c!=NOSUCH; c=tree->node[c].sibling) {
                 if(fos && (fos->fossilInt[c] != NOSUCH || fos->status[c] == unknownNodeStatus)) {
					int f;
                    if(fos->status[c] == unknownNodeStatus && fos->endTimeTable[fos->endTime[c]].inf<max[n])
                        max[n] = fos->endTimeTable[fos->endTime[c]].inf;
                    for(f=fos->fossilInt[c]; f!=NOSUCH; f=fos->fossilIntList[f].prec)
						if(fos->fossilIntList[f].fossilInt.inf<max[n]) {
							max[n] = fos->fossilIntList[f].fossilInt.inf;
							dmax[n] = 0;
						}
				} else {
					if((max[c])<(max[n])) {
						max[n] = max[c];
						dmax[n] = dmax[c]+1;
					}
				}
			}
		}
	}
}

void GTreeFossilGeometry::fillUnknownTimes() {
	GTreeGeometry::fillUnknownTimes();
	for(int n=0; n<tree->size; n++)
        if(tree->node[n].child == NOSUCH) {
            switch(fos->status[n]) {
                case contempNodeStatus:
                time[n] = maxTime;
                break;
                case extinctNodeStatus:
                if(fos->fossilInt[n] != NOSUCH) {
                    time[n] = fos->fossilIntList[fos->fossilInt[n]].fossilInt.sup;
                    for(int f=fos->fossilIntList[fos->fossilInt[n]].prec; f!=NOSUCH; f=fos->fossilIntList[f].prec)
                        if(fos->fossilIntList[f].fossilInt.sup>time[n])
                            time[n] = fos->fossilIntList[f].fossilInt.sup;
                } else
                    time[n] = maxTime;
                break;
                case unknownNodeStatus:
                if(fos->endTime[n] != NOSUCH)
                    time[n] = fos->endTimeTable[fos->endTime[n]].sup;
                else
                    time[n] = maxTime;
                break;
            default:
                time[n] = maxTime;
            }
        }
}

double GTreeFossilGeometry::fillTreeGeometry(int x0, int y0, int w, int l)
{
	double y =  GTreeGeometry::fillTreeGeometry(x0, y0, w, l);
	if(fos != NULL) {
		int f;
		for(f=fos->fossilInt[tree->root]; f>=0; f=fos->fossilIntList[f].prec)
			line[f] = QLineF((fos->fossilIntList[f].fossilInt.inf-minTime)*scale+xoffset,
							   yoffset+y,
							   (fos->fossilIntList[f].fossilInt.sup-minTime)*scale+xoffset,
							   yoffset+y);
	}
	return y;
}
#define LINE_EPSILON 0.05
double GTreeFossilGeometry::fillNodeGeometry(int n, int parent)
{
	double y =  GTreeGeometry::fillNodeGeometry(n, parent);
	if(fos != NULL) {
		int f;
		for(f=fos->fossilInt[n]; f != NOSUCH; f=fos->fossilIntList[f].prec) {
			 line[f] = QLineF((fos->fossilIntList[f].fossilInt.inf-minTime)*scale+xoffset-LINE_EPSILON,
								yoffset+y,
								(fos->fossilIntList[f].fossilInt.sup-minTime)*scale+xoffset+LINE_EPSILON,
								yoffset+y);
		}
	}
	return y;
}

QLineF GTreeFossilGeometry::lineFossil(int f)
{
	return line[f];
}
