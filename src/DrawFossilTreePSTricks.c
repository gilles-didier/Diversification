#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include "Utils.h"
#include "DrawFossilTreePSTricks.h"

#define NB_MIN 6
#define OFFSET 10
#define TICK_LENGTH 4
#define FONT_NAME "Helvetica"
//#define LABEL_SEP 10.
#define MAX_STRING_SIZE 200
//#define STANDARD_WIDTH 500
#define CHAR_WIDTH 10
#define LABEL_SEP 10.
#define FONT_SIZE 9.
#define STANDARD_WIDTH 500

typedef struct PARAM_DRAW_TREE_PST {
	double scale, scaleX, scaleY, leafSep, leafCur, labelSep, ycenter, ydec, radius, xoffset, yoffset;
	double tmin, tmax;
} TypeParamDrawTreePst;

static double drawNodeFossilPst(FILE *fo, int n, int parent, TypeTree *tree,  TypeFossilIntFeature *fos, TypeParamDrawTreePst *param);
static void drawTreeFossilPst(FILE *fo, TypeTree *tree,  TypeFossilIntFeature *fos, TypeParamDrawTreePst *param);
static double getMaxLeafLabelWidth(TypeTree *tree);
static double getBestWidth(double max, char *dest, char *src);
static void drawScalePst(FILE *fo, double x, double y, TypeParamDrawTreePst *param);
static void fprintText(FILE *fo, double x0, double y0, char *text, char *mod);
static void fprintLine(FILE *fo, double x0, double y0, double x1, double y1);
static void fprintDot(FILE *fo, double x0, double y0);
static void fprintLineDot(FILE *fo, double x0, double x1, double y0);

void fprintText(FILE *fo, double x0, double y0, char *text, char *mod) {
    int i;
    char *tmp = strdpl(text);
    for(i=0; tmp[i]!='\0'; i++)
        if(tmp[i] == '_')
            tmp[i] = ' ';
    fprintf(fo, "\\rput[%s](%.2lf, %.2lf){\\small %s}\n", mod, x0, y0, tmp);
    free((void*)tmp);
}
void fprintLine(FILE *fo, double x0, double y0, double x1, double y1) {
    fprintf(fo, "\\psline[linewidth=0.5pt,linecolor=black](%.2lf, %.2lf)(%.2lf, %.2lf)\n", x0, y0, x1, y1);
}

void fprintDot(FILE *fo, double x0, double y0) {
	fprintf(fo, "\\psdot(%.2lf, %.2lf)\n", x0, y0);
}

void fprintLineDot(FILE *fo, double x0, double x1, double y0) {
    fprintf(fo, "\\psline[linewidth=3pt,linecolor=red,opacity=0.3,]{cc-cc}(%.2lf, %.2lf)(%.2lf, %.2lf)\n", x0, y0, x1, y0);
}


double getBestWidth(double max, char *dest, char *src) {
    unsigned int ind;
	double width;
	dest[0] = src[0];
	width = strlen(src);
	if(width<max || strlen(src)<3) {
		strcpy(dest, src);
		return width;
	}
	ind = 1;
	dest[0] = src[0];
	dest[1] = '.';
	dest[2] = '.';
	do {
		ind++;
		strcpy(&(dest[3]), &(src[ind]));
		width = strlen(dest);
	} while(ind<strlen(src) && width>max);
	return width;
}

void drawScalePst(FILE *fo, double x, double y, TypeParamDrawTreePst *param) {
    int flag = 0;
    double start, end, step, cur;
    fprintLine(fo, x, y, x+(param->tmax-param->tmin)*param->scale, y);
    if((param->tmax-param->tmin) <= 0.)
		return;
    step = pow(10., floor(log10(param->tmax-param->tmin)));
    if((param->tmax-param->tmin)/step<NB_MIN) {
		step /= 2.;
		flag = 1;
	}
    start = step*ceil(param->tmin/step);
    end = step*floor(param->tmax/step);
	for(cur = start; cur<=end; cur += step) {
		char tmp[500];
        fprintLine(fo, x+(cur-param->tmin)*param->scale, y, x+(cur-param->tmin)*param->scale, y+TICK_LENGTH);
		if(flag)
			sprintf(tmp, "%.1lf", cur);
		else
			sprintf(tmp, "%.0lf", cur);
        fprintText(fo, x+(cur-param->tmin)*param->scale, y+2*TICK_LENGTH, tmp, "b");
	}
}


double getMaxLeafLabelWidth(TypeTree *tree) {
	int n;
	double max = 0.;
    if(tree->name)
		for(n=0; n<tree->size; n++)
            if(tree->node[n].child<0 && tree->name[n])
                if(strlen(tree->name[n])>max)
                    max = strlen(tree->name[n]);
	return max;
}


void drawTreeFossilFilePst(FILE *fo, TypeTree *tree,  TypeFossilIntFeature *fos) {
    double width, height;
	TypeParamDrawTreePst param;

	param.scale = 100.;
	param.xoffset = OFFSET;
	param.yoffset = 0;
	param.leafSep = OFFSET;
	param.labelSep = 3.;
	param.ycenter = 3.;
	param.ydec = 2.;
	param.radius = 3.;
	param.leafCur = param.leafSep;
    param.tmin = tree->minTime;
    param.tmax = tree->maxTime;
	
	height = param.leafSep*(countLeaves(tree))+3*OFFSET+3*LABEL_SEP+FONT_SIZE;
	width = STANDARD_WIDTH;
	if(param.tmin == param.tmax)
		param.tmax++;
    param.scale = (width-(2*param.xoffset+param.labelSep+getMaxLeafLabelWidth(tree)*CHAR_WIDTH))/((param.tmax-param.tmin));
	fprintf(fo, "\\begin{pspicture}(0,0)(%.2lf, %.2lf)\n", width, height);
    drawTreeFossilPst(fo, tree, fos, &param);
    drawScalePst(fo, param.xoffset, height-(OFFSET+LABEL_SEP+FONT_SIZE), &param);
	fprintf(fo, "\\end{pspicture}\n");
}



void drawTreeFossilPst(FILE *fo, TypeTree *tree,  TypeFossilIntFeature *fos, TypeParamDrawTreePst *param) {
	int tmp, f;
	double min, max, y;
	if(tree->size<=0)
		return;
	if((tmp = tree->node[tree->root].child) >= 0) {
        min = drawNodeFossilPst(fo, tmp, tree->root, tree, fos, param);
		max = min;
		for(tmp = tree->node[tmp].sibling; tmp >= 0; tmp = tree->node[tmp].sibling) {
            max = drawNodeFossilPst(fo, tmp, tree->root, tree, fos, param);
		}
	} else {
		max = param->leafCur;
		min = param->leafCur;
	}
	fprintLine(fo, (tree->time[tree->root]-param->tmin)*param->scale+param->xoffset, min, (tree->time[tree->root]-param->tmin)*param->scale+param->xoffset, max);
	y = (min+max)/2;
	fprintLine(fo, (tree->time[tree->root]-param->tmin)*param->scale+param->xoffset, param->yoffset+y, (tree->minTime-param->tmin)*param->scale+param->xoffset, param->yoffset+y);
    if(tree->name && tree->name[tree->root])
        fprintText(fo, (tree->time[tree->root]-param->tmin)*param->scale+param->xoffset+param->labelSep, y+param->yoffset, tree->name[tree->root], "l");
	if(fos)
        for(f=fos->fossilInt[tree->root]; f>=0; f=fos->fossilIntList[f].prec)
            fprintLineDot(fo, (fos->fossilIntList[f].fossilInt.inf-param->tmin)*param->scale+param->xoffset, (fos->fossilIntList[f].fossilInt.sup-param->tmin)*param->scale+param->xoffset, param->yoffset+y);
}

double drawNodeFossilPst(FILE *fo, int n, int parent, TypeTree *tree,  TypeFossilIntFeature *fos, TypeParamDrawTreePst *param) {
	double min, max, y;
	int f;
	if(tree->node[n].child >= 0) {
		int tmp = tree->node[n].child;
        min = drawNodeFossilPst(fo, tmp, n, tree, fos, param);
		max = min;
		for(tmp = tree->node[tmp].sibling; tmp >= 0; tmp = tree->node[tmp].sibling)
            max = drawNodeFossilPst(fo, tmp, n, tree, fos, param);
		y = (min+max)/2;
		fprintLine(fo, (tree->time[n]-param->tmin)*param->scale+param->xoffset, param->yoffset+min, (tree->time[n]-param->tmin)*param->scale+param->xoffset, param->yoffset+max);
        if(tree->name && tree->name[n]) {
            fprintText(fo, (tree->time[n]-param->tmin)*param->scale+param->xoffset+param->labelSep, y+param->yoffset, tree->name[n], "l");
		}
	} else {
		param->leafCur += param->leafSep;
		y = param->leafCur;
        if(tree->name && tree->name[n])
            fprintText(fo, (tree->time[n]-param->tmin)*param->scale+param->xoffset+param->labelSep, param->yoffset+y, tree->name[n], "l");
	}
	fprintLine(fo,(tree->time[n]-param->tmin)*param->scale+param->xoffset, param->yoffset+y, (tree->time[parent]-param->tmin)*param->scale+param->xoffset, param->yoffset+y);
	if(fos) {
        for(f=fos->fossilInt[n]; f>=0; f=fos->fossilIntList[f].prec)
            fprintLineDot(fo, (fos->fossilIntList[f].fossilInt.inf-param->tmin)*param->scale+param->xoffset, (fos->fossilIntList[f].fossilInt.sup-param->tmin)*param->scale+param->xoffset, param->yoffset+y);
	}
	return y;
}
















