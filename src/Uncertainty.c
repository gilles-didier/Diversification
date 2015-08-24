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




#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "Utils.h"
#include "TreeExtras.h"
#include "Uncertainty.h"
#include <gsl/gsl_sf_gamma.h>

static double logBinomial(unsigned int k, unsigned int n);

double binomial(unsigned int k, unsigned int n) {
    return exp(logBinomial(k,n));
}

double logBinomial(unsigned int k, unsigned int n) {
    return gsl_sf_lnfact(n)-(gsl_sf_lnfact(k)+gsl_sf_lnfact(n-k));
}

typedef enum UNCERTAINTY_LIKE_TYPE {
    TypeUncertaintyLikeUnknown=0,
    TypeUncertaintyLikeBaseA,
    TypeUncertaintyLikeBaseB,
    TypeUncertaintyLikeBaseC,
    TypeUncertaintyLikeStandardB,
    TypeUncertaintyLikeStandardC
} TypeUncertaintyLikeType;

typedef struct UNCERTAINTY_LIKE_INFO_STANDARD {
    int min, size, nLeafMin;
    double stopTime, *like;
} TypeUncertaintyLikeDataStandard;

typedef struct UNCERTAINTY_LIKE_INFO_BASE_A {
    int numberTaxa;
    double probTreeShape;
} TypeUncertaintyLikeDataBaseA;

typedef union UNCERTAINTY_LIKE_DATA {
    double stopTime;
    TypeUncertaintyLikeDataBaseA baseA;
    TypeUncertaintyLikeDataStandard std;
} TypeUncertaintyLikeData;

typedef struct UNCERTAINTY_LIKE_SPECIAL {
    int *nLeaves, size;
    double *logRank, *logLike;
} TypeUncertaintyLikeSpecial;

typedef struct UNCERTAINTY_LIKE_STANDARD {
    int *nLeaves, **leaf, size;
    double *logRank;
} TypeUncertaintyLikeStandard;

typedef struct UNCERTAINTY_LIKE_INFO {
    TypeUncertaintyLikeType type;
    TypeUncertaintyLikeStandard std;
    TypeUncertaintyLikeSpecial min;
    TypeUncertaintyLikeData data;
} TypeUncertaintyLikeInfo;

typedef struct UNCERTAINTY_LIKE_PARAMETERS {
    double birth, death, fossil;
    double alpha, beta, omega;
    double bma, bmab, amab;
} TypeUncertaintyLikeParameters;

static double getLogLikelihoodSplitted(TypeTree *tree, TypeUncertaintyLikeParameters *param);
static int fillSplitTreeFossil(TypeTree *tcur, int n, TypeTree *tree, TypeFossilTab *ftab, TypeTree **treeList, int *size);
static void fillUncertaintyLikeInfo(int n, TypeTree *tree, int *indexMin, TypeNodeStatus *status, TypeUncertaintyLikeInfo *info, TypeUncertaintyLikeParameters *param);
static double getLogLikelihoodNode(double t, int n, TypeTree *tree, int *indexMin, TypeNodeStatus *status, TypeUncertaintyLikeInfo *info, TypeUncertaintyLikeParameters *param);
static void fillIndexMin(int n, TypeTree *tree, int *indexMin, TypeNodeStatus *status);
static double getTreeShapeLogLikelihood(double rank, unsigned int nleaf);
static TypeUncertaintyLikeParameters getUncertaintyLikeParameters(TypeModelParam *param);
static double getLogProbTypeA(double startTime, double maxTime, int k,  TypeUncertaintyLikeParameters *param);
static double getLogProbTypeB(double startTime, double endTime, double maxTime, int k, TypeUncertaintyLikeParameters *param);
static double getLogProbTypeC(double startTime, double endTime, double maxTime, int k, int l, TypeUncertaintyLikeParameters *param);
static double getLogProbNotObservable(double t, double maxTime,  TypeUncertaintyLikeParameters *param);
static double getLogProbObservable(double t, double maxTime, TypeUncertaintyLikeParameters *param);
static void fillTableTreeIter(int n, TypeTree *tree, int *indexMin, double *K, double *T);

TypeUncertaintyLikeParameters getUncertaintyLikeParameters(TypeModelParam *param) {
    TypeUncertaintyLikeParameters res;
    res.birth = param->birth;
    res.death = param->death;
    res.fossil = param->fossil;
    res.alpha = (res.birth+res.death+res.fossil-sqrt(pow(res.birth+res.death+res.fossil, 2.)-4.*res.birth*res.death))/(2*res.birth);
    res.beta = (res.birth+res.death+res.fossil+sqrt(pow(res.birth+res.death+res.fossil, 2.)-4.*res.birth*res.death))/(2*res.birth);
    res.bma = res.beta-res.alpha;
    res.omega = -res.birth*res.bma;
    res.bmab = res.beta-res.alpha*res.beta;
    res.amab = res.alpha-res.alpha*res.beta;
    return res;
}

int fillSplitTreeFossil(TypeTree *tcur, int n, TypeTree *tree, TypeFossilTab *ftab, TypeTree **treeList, int *size) {
    int curInd = tcur->size++;
    if(ftab[n].size>0) {
        tcur->time[curInd] = ftab[n].time[0];
        tcur->node[curInd].child = -1;
        ((TypeNodeStatus*)tcur->info)[curInd]=extinctNodeStatus;
        if(ftab[n].size == 1 && tree->node[n].child == NOSUCH && (tree->time[n] == ftab[n].time[0] || tree->time[n]==NO_TIME)) { /*empty tree*/
            treeList[(*size)] = newTree(0);
            treeList[(*size)]->root = 0;
            treeList[(*size)]->minTime = ftab[n].time[0];
            treeList[(*size)]->maxTime = tree->maxTime;
            (*size)++;
            ftab[n].size--;
            ftab[n].time++;
        } else  { /*tree not empty*/
            treeList[(*size)] = newTree(tree->size);
            treeList[(*size)]->root = 0;
            treeList[(*size)]->minTime = tcur->time[curInd];
            treeList[(*size)]->maxTime = tree->maxTime;
            treeList[(*size)]->info = (TypeNodeStatus*) malloc(tree->size*sizeof(TypeNodeStatus));
            ((TypeNodeStatus*)treeList[(*size)]->info)[0]=noneNodeStatus;
            (*size)++;
            ftab[n].size--;
            ftab[n].time++;
            fillSplitTreeFossil(treeList[(*size)-1], n, tree, ftab, treeList, size);
        }
    } else {
        tcur->time[curInd] = tree->time[n];
        if(tree->node[n].child!=NOSUCH) {
            int c, prec;
            ((TypeNodeStatus*)tcur->info)[curInd]=noneNodeStatus;
            tcur->node[curInd].child = fillSplitTreeFossil(tcur, tree->node[n].child, tree, ftab, treeList, size);
            prec = tcur->node[curInd].child;
            for(c=tree->node[tree->node[n].child].sibling; c!=NOSUCH; c=tree->node[c].sibling) {
                tcur->node[prec].sibling = fillSplitTreeFossil(tcur, c, tree, ftab, treeList, size);
                prec = tcur->node[prec].sibling;
            }
            tcur->node[prec].sibling = -1;
        } else {
            if(tree->time[n]==tree->maxTime)
                ((TypeNodeStatus*)tcur->info)[curInd]=contempNodeStatus;
            else
                ((TypeNodeStatus*)tcur->info)[curInd]=unknownNodeStatus;
            tcur->node[curInd].child = -1;
        }
    }
    return curInd;
}


/*split the tree "tree" each time a fossill occurs*/
void splitTreeFossil(TypeTree *tree, TypeFossilFeature *fos, TypeTree ***treeList, int *size) {
    int i;
    TypeFossilTab *ftab, *ftmp;
    ftab = listToFossilTab(fos, tree->size);
    ftmp = (TypeFossilTab*) malloc(tree->size*sizeof(TypeFossilTab));
    for(i=0; i<tree->size; i++) {
        ftmp[i].size = ftab[i].size;
        ftmp[i].time = ftab[i].time;
    }
    *treeList = (TypeTree **) malloc((fos->size+1)*sizeof(TypeTree*));
    (*treeList)[0] = newTree(tree->size);
    (*treeList)[0]->root = 0;
    (*treeList)[0]->node[0].sibling = NOSUCH;
    (*treeList)[0]->minTime = tree->minTime;
    (*treeList)[0]->maxTime = tree->maxTime;
    (*treeList)[0]->info = (void*) malloc(tree->size*sizeof(TypeNodeStatus));
    ((TypeNodeStatus*)(*treeList)[0]->info)[0]=noneNodeStatus;
    *size = 1;
    fillSplitTreeFossil((*treeList)[0], tree->root, tree, ftmp, *treeList, size);
    for(i=0; i<tree->size; i++)
        if(ftab[i].time != NULL)
            free((void*)ftab[i].time);
    free((void*)ftab);
    free((void*)ftmp);
    for(i=0; i<*size; i++)
        reallocTree((*treeList)[i]->size, (*treeList)[i]);
}



/*get the whole likelihood*/
/* by convention if a leaf l is such that
 * - tree->time[l] == NO_TIME => it is extinct (it must have fossil(s)),
 * - tree->time[l] == tree->maxTime => it is contemporary,
 * - otherwise => it is unknown after but observable until tree->time[l]
 */
double getLogLikelihoodTreeFossil(TypeTree *tree, TypeFossilFeature *fos, TypeModelParam *param) {
    TypeTree **treeList;
    int l, size;
    double res = 0.;
    TypeUncertaintyLikeParameters pu = getUncertaintyLikeParameters(param);
    splitTreeFossil(tree, fos, &treeList, &size);
    for(l=0; l<size; l++) {
/*
int i;
fprintTreeX(stdout, treeList[l]);
for(i=0; i<treeList[l]->size; i++)
    if(treeList[l]->node[i].child == NOSUCH)
        fprintf(stdout, "l %d : %d\n", i, ((TypeNodeStatus*)treeList[l]->info)[i]);
fflush(stdout);
*/
        res += getLogLikelihoodSplitted(treeList[l], &pu);
        if(treeList[l]->info!=NULL)
            free((void*)treeList[l]->info);
        freeTree(treeList[l]);
    }
    free((void*)treeList);
    return res;
}


/*get the likelihood of a tree ending at each fossil or contemporary time*/
double getLogLikelihoodSplitted(TypeTree *tree, TypeUncertaintyLikeParameters *param) {
    int  *indexMin, i;
    TypeNodeStatus *statusMin;
    TypeUncertaintyLikeInfo *info;
    double res;
    if(tree == NULL || tree->size == 0) {
        if(param->death>0)
            return getLogProbNotObservable(tree->minTime, tree->maxTime, param);
        else
            return NEG_INFTY;
    }
    info = (TypeUncertaintyLikeInfo*) malloc(tree->size*sizeof(TypeUncertaintyLikeInfo));
    for(i=0; i<tree->size; i++) {
        info[i].type = TypeUncertaintyLikeUnknown;
        info[i].std.logRank = NULL;
        info[i].std.nLeaves = NULL;
        info[i].std.leaf = NULL;
        info[i].min.logLike = NULL;
        info[i].min.logRank = NULL;
        info[i].min.nLeaves = NULL;
    }
    indexMin = (int*) malloc(tree->size*sizeof(int));
    statusMin = (TypeNodeStatus*) malloc(tree->size*sizeof(TypeNodeStatus));
    fillIndexMin(tree->root, tree, indexMin, statusMin);
    if(tree->parent == NULL)
        setParent(tree);
    fillUncertaintyLikeInfo(tree->root, tree, indexMin, statusMin, info, param);
    res = getLogLikelihoodNode(tree->minTime, tree->root, tree, indexMin, statusMin, info, param);
    free((void*)indexMin);
    free((void*)statusMin);
    for(i=0; i<tree->size; i++) {
        int j;
        if(info[i].std.leaf != NULL) {
            for(j=0; j<info[i].std.size; j++)
                if(info[i].std.leaf[j] != NULL)
                    free((void*)info[i].std.leaf[j]);
            free((void*)info[i].std.leaf);
        }
        if(info[i].std.nLeaves != NULL)
            free((void*)info[i].std.nLeaves);
        if(info[i].std.logRank != NULL)
            free((void*)info[i].std.logRank);
        if((info[i].type == TypeUncertaintyLikeStandardB || info[i].type == TypeUncertaintyLikeStandardC) && info[i].data.std.like != NULL)
            free((void*)info[i].data.std.like);
         if(info[i].min.logLike != NULL)
            free((void*)info[i].min.logLike);
        if(info[i].min.logRank != NULL)
            free((void*)info[i].min.logRank);
        if(info[i].min.nLeaves != NULL)
            free((void*)info[i].min.nLeaves);
    }
    free((void*)info);
    return res;
}

double getTreeShapeLogLikelihood(double rank, unsigned int nleaf) {
    return rank-2*gsl_sf_lnfact(nleaf-1)-log(nleaf);
}


void fillUncertaintyLikeInfo(int n, TypeTree *tree, int *indexMin, TypeNodeStatus *statusMin, TypeUncertaintyLikeInfo *info, TypeUncertaintyLikeParameters *param) {
    if(info[n].type != TypeUncertaintyLikeUnknown)
        return;
    if(tree->node[n].child != NOSUCH) {
        int i, ind, conf[2], child[2];
        child[0] = tree->node[n].child;
        child[1] = tree->node[child[0]].sibling;
        if(child[1] == NOSUCH) {
            fprintf(stderr, "Execution error: node %d with a single child\n", n);
            exit(1);
        }
        if(tree->node[child[1]].sibling != NOSUCH) {
            fprintf(stderr, "Execution error: node %d with more than 2 children\n", n);
            exit(1);
        }
        if(info[child[0]].type == TypeUncertaintyLikeUnknown)
            fillUncertaintyLikeInfo(child[0], tree, indexMin, statusMin, info, param);
         if(info[child[1]].type == TypeUncertaintyLikeUnknown)
            fillUncertaintyLikeInfo(child[1], tree, indexMin, statusMin, info, param);
         if(n != tree->root) { /*it is useless to do it for the root*/
            info[n].std.size = info[child[0]].std.size*info[child[1]].std.size+1;
            info[n].std.nLeaves = (int*) malloc(info[n].std.size*sizeof(int));
            info[n].std.logRank = (double*) malloc(info[n].std.size*sizeof(double));
            info[n].std.leaf = (int**) malloc(info[n].std.size*sizeof(int*));
            conf[0] = 0; conf[1] = 0;
            ind = 1;
            while(conf[1]<info[child[1]].std.size) {
                int cont, itmp;
                info[n].std.nLeaves[ind] = info[child[0]].std.nLeaves[conf[0]]+info[child[1]].std.nLeaves[conf[1]];
                info[n].std.leaf[ind] = (int*) malloc(info[n].std.nLeaves[ind]*sizeof(int));
                info[n].std.logRank[ind] = info[child[0]].std.logRank[conf[0]]+info[child[1]].std.logRank[conf[1]]+log(2)+logBinomial((unsigned int) (info[child[0]].std.nLeaves[conf[0]]-1), (unsigned int) (info[n].std.nLeaves[ind]-2));
                itmp = 0;
                for(i=0; i<2; i++) {
                    int j;
                    for(j=0; j<info[child[i]].std.nLeaves[conf[i]]; j++)
                        info[n].std.leaf[ind][itmp++] = info[child[i]].std.leaf[conf[i]][j];
                }
                i=0;
                do {
                    conf[i]++;
                    if(i<1 && conf[i] == info[child[i]].std.size) {
                        conf[i] = 0;
                        i++;
                        cont = 1;
                    } else
                        cont = 0;
                } while(cont);
                ind++;
            }
            info[n].std.nLeaves[0] = 1;
            info[n].std.logRank[0] = 0.;
            info[n].std.leaf[0] = (int*) malloc(info[n].std.nLeaves[0]*sizeof(int));
            info[n].std.leaf[0][0] = n;
        } else {
            info[n].std.size = 1;
            info[n].std.nLeaves = (int*) malloc(sizeof(int));
            info[n].std.logRank = (double*) malloc(sizeof(double));
            info[n].std.leaf = (int**) malloc(sizeof(int*));
            info[n].std.nLeaves[0] = info[child[0]].std.nLeaves[info[child[0]].std.size-1]+info[child[1]].std.nLeaves[info[child[1]].std.size-1];
            info[n].std.logRank[0] = info[child[0]].std.logRank[info[child[0]].std.size-1]+info[child[1]].std.logRank[info[child[1]].std.size-1]+log(2)+logBinomial((unsigned int) (info[child[0]].std.nLeaves[info[child[0]].std.size-1]-1), (unsigned int) (info[n].std.nLeaves[0]-2));
            info[n].std.leaf[0] = (int*) malloc(sizeof(int));
            info[n].std.leaf[0][0] = n;
        }
    } else {
            info[n].std.size = 1;
            info[n].std.nLeaves = (int*) malloc(sizeof(int));
            info[n].std.logRank = (double*) malloc(sizeof(double));
            info[n].std.leaf = (int**) malloc(sizeof(int*));
            info[n].std.nLeaves[0] = 1;
            info[n].std.logRank[0] = 0.;
            info[n].std.leaf[0] = (int*) malloc(info[n].std.nLeaves[0]*sizeof(int));
            info[n].std.leaf[0][0] = n;
    }
    if(tree->time[indexMin[n]] < tree->maxTime) { /*meaning it can happen something after tree->time[indexMin[n]], so we have to deal with it, i.e. it contains a fossil*/
        if(tree->node[n].child != NOSUCH) {
            int special, i, indSpecial, child[2], conf[2], sizeConf[2], leafMin, leafMax, *(nLeaves[2]);
            double *(logRank[2]), *(logLike[2]), *offset;
            special = tree->parent[n] != NOSUCH && tree->time[indexMin[tree->parent[n]]] == tree->time[indexMin[n]];
            if(statusMin[n] == extinctNodeStatus)
                info[n].type = TypeUncertaintyLikeStandardB;
            else
                info[n].type = TypeUncertaintyLikeStandardC;
            info[n].data.std.stopTime = tree->time[indexMin[n]];
            child[0] = tree->node[n].child;
            child[1] = tree->node[child[0]].sibling;
            info[n].data.std.nLeafMin = 0;
            leafMin = 0;
            leafMax = 0;
            if(tree->time[indexMin[child[0]]] == info[n].data.std.stopTime) {
                leafMin += info[child[0]].min.nLeaves[0];
                leafMax += info[child[0]].min.nLeaves[info[child[0]].min.size-1];
                nLeaves[0] = info[child[0]].min.nLeaves;
                logRank[0] = info[child[0]].min.logRank;
                logLike[0] = info[child[0]].min.logLike;
                info[n].data.std.nLeafMin += info[child[0]].data.std.nLeafMin;
                sizeConf[0] = info[child[0]].min.size;
            } else {
                int j;
                double *tableTmp = (double*) malloc(tree->size*sizeof(double));
                leafMin += info[child[0]].std.nLeaves[0];
                leafMax += info[child[0]].std.nLeaves[info[child[0]].std.size-1];
                nLeaves[0] = info[child[0]].std.nLeaves;
                logRank[0] = info[child[0]].std.logRank;
                sizeConf[0] = info[child[0]].std.size;
                for(i=0; i<tree->size; i++)
                    tableTmp[i] = sqrt(-1);
                logLike[0] = (double*) malloc(info[child[0]].std.size*sizeof(double));
                for(j=0; j<info[child[0]].std.size; j++) {
                    int k;
                    logLike[0][j] = 0.;
                    for(k=0; k<info[child[0]].std.nLeaves[j]; k++) {
                        if(isnan(tableTmp[info[child[0]].std.leaf[j][k]]))
                            tableTmp[info[child[0]].std.leaf[j][k]] = getLogLikelihoodNode(info[n].data.std.stopTime, info[child[0]].std.leaf[j][k], tree, indexMin, statusMin, info, param);
                        logLike[0][j] += tableTmp[info[child[0]].std.leaf[j][k]];
                    }
                }
                free((void*)tableTmp);
            }
            if(tree->time[indexMin[child[1]]] == info[n].data.std.stopTime) {
                leafMin += info[child[1]].min.nLeaves[0];
                leafMax += info[child[1]].min.nLeaves[info[child[1]].min.size-1];
                nLeaves[1] = info[child[1]].min.nLeaves;
                logRank[1] = info[child[1]].min.logRank;
                logLike[1] = info[child[1]].min.logLike;
                info[n].data.std.nLeafMin += info[child[1]].data.std.nLeafMin;
                sizeConf[1] = info[child[1]].min.size;
            } else {
                int j;
                double *tableTmp = (double*) malloc(tree->size*sizeof(double));
                leafMin += info[child[1]].std.nLeaves[0];
                leafMax += info[child[1]].std.nLeaves[info[child[1]].std.size-1];
                nLeaves[1] = info[child[1]].std.nLeaves;
                logRank[1] = info[child[1]].std.logRank;
                sizeConf[1] = info[child[1]].std.size;
                for(i=0; i<tree->size; i++)
                    tableTmp[i] = sqrt(-1);
                logLike[1] = (double*) malloc(info[child[1]].std.size*sizeof(double));
                for(j=0; j<info[child[1]].std.size; j++) {
                    int k;
                    logLike[1][j] = 0.;
                    for(k=0; k<info[child[1]].std.nLeaves[j]; k++) {
                        if(isnan(tableTmp[info[child[1]].std.leaf[j][k]]))
                            tableTmp[info[child[1]].std.leaf[j][k]] = getLogLikelihoodNode(info[n].data.std.stopTime, info[child[1]].std.leaf[j][k], tree, indexMin, statusMin, info, param);
                        logLike[1][j] += tableTmp[info[child[1]].std.leaf[j][k]];
                    }
                }
                free((void*)tableTmp);
            }
            info[n].data.std.min = leafMin;
            info[n].data.std.size = leafMax-leafMin+1;
            info[n].data.std.like = (double*) malloc(info[n].data.std.size*sizeof(double));
            for(i=0; i<info[n].data.std.size; i++)
                info[n].data.std.like[i] = 0.;
            conf[0] = 0;
            conf[1] = 0;
            offset = (double*) malloc(info[n].data.std.size*sizeof(double));
            for(i=0; i<info[n].data.std.size; i++)
                offset[i] = NEG_INFTY;
            if(special) {
                info[n].min.size = sizeConf[0]*sizeConf[1];
                info[n].min.nLeaves = (int*) malloc(info[n].min.size*sizeof(int));
                info[n].min.logLike = (double*) malloc(info[n].min.size*sizeof(double));
                info[n].min.logRank = (double*) malloc(info[n].min.size*sizeof(double));
                indSpecial = 0;
            }
            while(conf[1]<sizeConf[1]) {
                int nl, cont;
                double ll, lr;
                nl = nLeaves[0][conf[0]]+nLeaves[1][conf[1]];
                lr = logRank[0][conf[0]]+logRank[1][conf[1]]+log(2)+logBinomial((unsigned int) (nLeaves[0][conf[0]]-1), (unsigned int) (nl-2));
                ll = logLike[0][conf[0]]+logLike[1][conf[1]]+getTreeShapeLogLikelihood(lr, (unsigned int) nl); /*the probability of the tree shape of the current configuration*/
                if(offset[nl-info[n].data.std.min] == NEG_INFTY) {
                    offset[nl-info[n].data.std.min] = ll;
                    info[n].data.std.like[nl-info[n].data.std.min] = 1.;
                } else {
                    if(ll>offset[nl-info[n].data.std.min]) { /*compute max in offset just to avoid numerical precision issues*/
                        info[n].data.std.like[nl-info[n].data.std.min] *= exp(offset[nl-info[n].data.std.min]-ll);
                        offset[nl-info[n].data.std.min] = ll;
                    }
                    info[n].data.std.like[nl-info[n].data.std.min] += exp(ll-offset[nl-info[n].data.std.min]);
                }
                if(special) {
                    info[n].min.nLeaves[indSpecial] = nl;
                    info[n].min.logRank[indSpecial] = lr;
                    info[n].min.logLike[indSpecial] = logLike[0][conf[0]]+logLike[1][conf[1]];
                    indSpecial++;
                }
                i=0;
                do {
                    conf[i]++;
                    if(i<1 && conf[i] == sizeConf[i]) {
                        conf[i] = 0;
                        i++;
                        cont = 1;
                    } else
                        cont = 0;
                } while(cont);
            }
            for(i=0; i<info[n].data.std.size; i++)
                if(info[n].data.std.like[i]>0)
                    info[n].data.std.like[i] = log(info[n].data.std.like[i])+offset[i];
                else
                    info[n].data.std.like[i] = 0.;
            if(tree->time[indexMin[child[0]]] != info[n].data.std.stopTime)
                free((void*)logLike[0]);
            if(tree->time[indexMin[child[1]]] != info[n].data.std.stopTime)
                free((void*)logLike[1]);
            free((void*)offset);
        } else {
            info[n].min.size = 1;
            info[n].min.nLeaves = (int*) malloc(info[n].min.size*sizeof(int));
            info[n].min.logLike = (double*) malloc(info[n].min.size*sizeof(double));
            info[n].min.logRank = (double*) malloc(info[n].min.size*sizeof(double));
            info[n].min.nLeaves[0] = 1;
            info[n].min.logRank[0] = 0;
            info[n].min.logLike[0] = 0;
            info[n].data.std.nLeafMin = 1;
            if(statusMin[n] == extinctNodeStatus)
                info[n].type = TypeUncertaintyLikeBaseB;
            else
                info[n].type = TypeUncertaintyLikeBaseC;
            info[n].data.stopTime = tree->time[indexMin[n]];
        }
    } else {
        info[n].type = TypeUncertaintyLikeBaseA;
        info[n].data.baseA.numberTaxa = info[n].std.nLeaves[info[n].std.size-1];
        info[n].data.baseA.probTreeShape = getTreeShapeLogLikelihood(info[n].std.logRank[info[n].std.size-1], (unsigned int) info[n].std.nLeaves[info[n].std.size-1]);
    }
}

double getLogLikelihoodNode(double t, int n, TypeTree *tree, int *indexMin, TypeNodeStatus *statusMin, TypeUncertaintyLikeInfo *info, TypeUncertaintyLikeParameters *param) {
    if(info[n].type == TypeUncertaintyLikeUnknown)
        fillUncertaintyLikeInfo(n, tree, indexMin, statusMin, info, param);
    switch(info[n].type) {
        case TypeUncertaintyLikeStandardB:
        {
            int l;
            double like = 0., offset = NEG_INFTY;
            if(info[n].data.std.nLeafMin>1) {
                fprintf(stderr, "Execution error: two simultaneous fossils at time %.2lf (subtree %d)\n", info[n].data.std.stopTime, n);
                exit(1);
            }
            for(l=0; l<info[n].data.std.size; l++) {
                double logLike;
                logLike = getLogProbTypeB(t, info[n].data.std.stopTime, tree->maxTime, l+info[n].data.std.min-1, param)+info[n].data.std.like[l];
                if(isinf(logLike) || isnan(logLike)) {
                    like += 0.;
                } else {
                    if(offset == NEG_INFTY) {
                        offset = logLike;
                        like = 1.;
                    } else {
                        if(logLike>offset) { /*compute max in offset just to avoid numerical precision issues*/
                            like *= exp(offset-logLike);
                            offset = logLike;
                        }
                        like += exp(logLike-offset);
                    }
                }
            }
            if(like>0.)
                return log(like)+offset;
            else
                return NEG_INFTY;
        }
        case TypeUncertaintyLikeStandardC:
        {
            int l;
            double like = 0., offset = NEG_INFTY;
            for(l=0; l<info[n].data.std.size; l++) {
                double logLike;
                logLike = getLogProbTypeC(t, info[n].data.std.stopTime, tree->maxTime, l+info[n].data.std.min, info[n].data.std.nLeafMin, param)+info[n].data.std.like[l];
                if(isinf(logLike) || isnan(logLike)) {
                    like += 0.;
                } else {
                    if(offset == NEG_INFTY) {
                        offset = logLike;
                        like = 1.;
                    } else {
                        if(logLike>offset) { /*compute max in offset just to avoid numerical precision issues*/
                            like *= exp(offset-logLike);
                            offset = logLike;
                        }
                        like += exp(logLike-offset);
                    }
                }
            }
            if(like>0.)
                return log(like)+offset;
            else
                return 0.;
        }
        case TypeUncertaintyLikeBaseA:
            return getLogProbTypeA(t, tree->maxTime, info[n].data.baseA.numberTaxa, param)+info[n].data.baseA.probTreeShape;
        case TypeUncertaintyLikeBaseB:
            return getLogProbTypeB(t, info[n].data.stopTime, tree->maxTime, 0, param);
        case TypeUncertaintyLikeBaseC:
            return getLogProbTypeB(t, info[n].data.stopTime, tree->maxTime, 0, param);
        default:
            fprintf(stderr, "Execution error: No type while computing the likelihood of %d\n", n);
            exit(1);
    }
}

/*return p(k, startTime)*/
double getLogProbTypeA(double startTime, double maxTime, int k,  TypeUncertaintyLikeParameters *param) {
    double time = maxTime-startTime;
    return
        2.*log(param->bma)
        +param->omega*time
        +((double)k-1.)*log(1.-exp(param->omega*time))
        -((double)k+1.)*log(param->beta-param->alpha*exp(param->omega*time));
}

/* return P_x(k, startTime, endTime)/(P^\star_o)^k*/
double getLogProbTypeB(double startTime, double endTime, double maxTime, int k,  TypeUncertaintyLikeParameters *param) {
    return
        log((double)k+1.)
        + log(param->fossil)
        +param->omega*(endTime-startTime)
        +((double)k)*log(1-exp(param->omega*(endTime-startTime)))+
        +((double)k+2.)*log(param->beta-param->alpha*exp(param->omega*(maxTime-endTime)))
        -((double)k+2.)*log(param->beta-param->alpha*exp(param->omega*(maxTime-startTime)))
        -((double)k)*log(param->bma);
}

/* return P_y(k, startTime, endTime)/(P^\star_o)^(k-l)*/
double getLogProbTypeC(double startTime, double endTime, double maxTime, int k, int l, TypeUncertaintyLikeParameters *param) {
    return
        param->omega*(endTime-startTime)
        +((double)k-1.)*(log(1-exp(param->omega*(endTime-startTime)))-log(param->bma))
        -((double)k+1.)*log(param->beta-param->alpha*exp(param->omega*(maxTime-startTime)))
        +((double)l)*log(param->bmab-param->amab*exp(param->omega*(maxTime-endTime)))
        +((double)k-l+1.)*log(param->beta-param->alpha*exp(param->omega*(maxTime-endTime)));
}

double getLogProbNotObservable(double t, double maxTime, TypeUncertaintyLikeParameters *param) {
    return log(param->alpha)+log(param->beta)+log((1.-exp(param->omega*(maxTime-t))))-log(param->beta-param->alpha*exp(param->omega*(maxTime-t)));
}

double getLogProbObservable(double t, double maxTime, TypeUncertaintyLikeParameters *param) {
    if(t==maxTime)
        return 0.;
    return log(1.-exp(getLogProbNotObservable(t, maxTime, param)));
}

/*indexMin[n] = leaf with the smallest time in subtree n*/
void fillIndexMin(int n, TypeTree *tree, int *indexMin, TypeNodeStatus *statusMin) {
    if(tree->node[n].child == NOSUCH) {
        indexMin[n] = n;
        statusMin[n] = ((TypeNodeStatus*)tree->info)[n];
    } else {
        int c;
        fillIndexMin(tree->node[n].child, tree, indexMin, statusMin);
        indexMin[n] = indexMin[tree->node[n].child];
        statusMin[n] = statusMin[tree->node[n].child];
        for(c=tree->node[tree->node[n].child].sibling; c != NOSUCH; c=tree->node[c].sibling) {
            fillIndexMin(c, tree, indexMin, statusMin);
            if(tree->time[indexMin[c]]<tree->time[indexMin[n]]) {
                indexMin[n] = indexMin[c];
                statusMin[n] = statusMin[c];
            }
        }
    }
}


double getLogLikelihoodEvent(TypeTree *tree, TypeFossilFeature *fos, TypeModelParam *param) {
    int i, f;
    double res = 0;
    for(i=0; i<tree->size; i++)
        if(fos != NULL) {
            for(f=fos->fossil[i]; f!=-1; f=fos->fossilList[f].prec)
                if(fos->fossilList[f].time<tree->time[i])
                    res += getFossilProbLog(fos->fossilList[f].time, param->birth, param->death, param->fossil, getAlpha(param->birth, param->death, param->fossil), getBeta(param->birth, param->death, param->fossil));
                else
                    res += getDeathProbLog(fos->fossilList[f].time, param->birth, param->death, param->fossil, getAlpha(param->birth, param->death, param->fossil), getBeta(param->birth, param->death, param->fossil));
        }
    return res;
}




void fillTableTreeIter(int n, TypeTree *tree, int *indexMin, double *K, double *T) {
    if(tree->node[n].child != NOSUCH) {
        int child[2];
        child[0] = tree->node[n].child;
        child[1] = tree->node[child[0]].sibling;
        if(child[1] == NOSUCH) {
            fprintf(stderr, "Execution error: node %d with a single child\n", n);
            exit(1);
        }
        if(tree->node[child[1]].sibling != NOSUCH) {
            fprintf(stderr, "Execution error: node %d with more than 2 children\n", n);
            exit(1);
        }
        fillTableTreeIter(child[0], tree, indexMin, K, T);
        fillTableTreeIter(child[1], tree, indexMin, K, T);
        if(tree->time[indexMin[n]]<tree->maxTime) {
			double size[2];
			if(tree->time[indexMin[n]] == tree->time[indexMin[child[0]]])
				size[0] = K[child[0]];
			else
				size[0] = T[child[0]];
			if(tree->time[indexMin[n]] == tree->time[indexMin[child[1]]])
				size[1] = K[child[1]];
			else
				size[1] = T[child[1]];
			K[n] = size[0]*size[1];
		} else
			K[n] = 1;
        T[n] = T[child[0]]*T[child[1]]+1.;
    } else {
        if(tree->time[indexMin[n]]<tree->maxTime)
			K[n] = 1.;
		else
			K[n] = 0.;
        T[n] = 1.;
    }
}


double getItemNumber(TypeTree *tree, TypeFossilFeature *fos) {
    TypeTree **treeList;
    int l, size;
    double res = 0.;
    splitTreeFossil(tree, fos, &treeList, &size);
    for(l=0; l<size; l++) {
        res += getItemNumberSplitted(treeList[l]);
        if(treeList[l]->info!=NULL)
            free((void*)treeList[l]->info);
        freeTree(treeList[l]);
    }
    free((void*)treeList);
   return res;
}

double getItemNumberSplitted(TypeTree *tree) {
    int  *indexMin, n;
    TypeNodeStatus *statusMin;
    double sum = 0., *K, *T;
    if(tree == NULL || tree->size == 0)
            return 1.;
    indexMin = (int*) malloc(tree->size*sizeof(int));
    statusMin = (TypeNodeStatus*) malloc(tree->size*sizeof(TypeNodeStatus));
    fillIndexMin(tree->root, tree, indexMin, statusMin);
    K = (double*) malloc(tree->size*sizeof(double));
    T = (double*) malloc(tree->size*sizeof(double));
    fillTableTreeIter(tree->root, tree, indexMin, K, T);
    for(n=0; n<tree->size; n++)
        sum += K[n]+T[n];
    free((void*)indexMin);
    free((void*)statusMin);
    free((void*)K);
    free((void*)T);
    return sum;
}

