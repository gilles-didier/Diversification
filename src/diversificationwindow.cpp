#include "diversificationwindow.h"
#include "ui_diversificationwindow.h"
#include <QtWidgets>
#include "goptiondialog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <iostream>
#include <locale.h>
#include "Utils.h"
#include "Fossil.h"
#include "Uncertainty.h"
#include "SimulFossil.h"
#include "Random.h"
#include "MinimizeNLOpt.h"
#include "qtreedrawwidget.h"
#include <QDebug>
#include <QFileInfo>
#include <QtSvg/QSvgGenerator>
#include "gsaveoptiondialog.h"
#include "gutil.h"
#include "gestimater.h"
#include "DrawFossilTreePSTricks.h"
#include "gtreefossilgeometry.h"
#include "ghelpdialog.h"
//#include <QWebView>

#define HELP_PAGE "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n<html>\n<head>\n	<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n	<title></title>\n	<meta name=\"generator\" content=\"LibreOffice 4.3.3.2 (Linux)\"/>\n	<meta name=\"created\" content=\"2015-03-31T00:00:00\"/>\n	<meta name=\"changed\" content=\"2015-04-01T11:20:48.923424128\"/>\n	<style type=\"text/css\">\n		@page { margin: 2cm }\n		p { margin-bottom: 0.25cm; line-height: 120% }\n	</style>\n</head>\n<body lang=\"fr-FR\" dir=\"ltr\" style=\"background: transparent\">\n<p align=\"center\" style=\"margin-bottom: 0cm; line-height: 100%\"><font size=\"4\" style=\"font-size: 16pt\"><b>Diversification\nHelp</b></font></p>\n<p align=\"justify\" style=\"margin-bottom: 0cm; line-height: 100%\"><br/>\n\n</p>\n<p align=\"justify\" style=\"margin-bottom: 0cm; line-height: 100%\">To\nuse <i>Diversification</i><span style=\"font-variant: normal\"><span style=\"font-style: normal\">,\nplease first open a tree file, next a file containing the\ncorresponding fossils table. </span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">You\nmay need to</span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">\nset</span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">s</span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">\n</span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">the\nadditional</span></span><span style=\"font-variant: normal\"><span style=\"font-style: normal\">\nparameters, which are&nbsp;:</span></span></p>\n<ul>\n	<li/>\n<p align=\"justify\" style=\"margin-bottom: 0cm; font-variant: normal; font-style: normal; line-height: 100%\">\n	the <i>Origin</i>, i.e. the starting time of evolution,</p>\n	<li/>\n<p align=\"justify\" style=\"margin-bottom: 0cm; font-variant: normal; font-style: normal; line-height: 100%\">\n	the <i>End</i>, i.e. the contemporary time,</p>\n	<li/>\n<p align=\"justify\" style=\"margin-bottom: 0cm; font-variant: normal; font-style: normal; line-height: 100%\">\n	the number of samplings of fossil times, which are uniformly drawn\n	from the intervals provided in the fossils file.</p>\n</ul>\n<p align=\"justify\" style=\"margin-bottom: 0cm; font-variant: normal; font-style: normal; line-height: 100%\">\n<br/>\n\n</p>\n<p align=\"justify\" style=\"margin-bottom: 0cm; font-variant: normal; font-style: normal; line-height: 100%\">\nThe estimation of the diversification and fossil find rates is\nlaunched by the item <i>Compute</i> of the <i>Diversification</i>\nmenu or alternatively the corresponding item of the toolbar. The\ncomputation may be stopped by clicking on the <i>Stop</i> button\n(results displayed afterwards will be based only on the iterations\nperformed so far). The mean rates and their standard deviations among\nthe fossil times samplings are displayed as soon as the computation\nends.</p>\n</body>\n</html>"
#define MINIMUM_TIME -100000000000.
#define MAXIMUM_TIME 100000000000.
#define SIMAL 0.01


DiversificationWindow::DiversificationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DiversificationWindow)
{
    ui->setupUi(this);
    tree = NULL;
    fossilInt = NULL;
//    helpDialog = NULL;
    helpDialog = new GHelpDialog(this);
    helpDialog->setContent(*new QUrl("qrc:/Help.html"));
    TypeTree *tree;
    tree = Tree::newTree(INC_SIZE_TREE);
    tree->size = 1;
    tree->root = 0;
    tree->node[tree->root].child = NOSUCH;
    tree->node[tree->root].sibling = NOSUCH;
    setWindowTitle("Untitled.newick");
    newed = true;
    TypeFossilIntFeature *f = newFossilIntFeature(tree->sizeBuf);
    treeView = new GTreeFossilView(tree, f, 0., 1., this);
    treeView->setModified(false);
    ui->verticalLayout->addWidget(treeView);
    ui->spinBox_origin->setMinimum(MINIMUM_TIME);
    ui->spinBox_endS->setMaximum(MAXIMUM_TIME);
    connect(ui->spinBox_origin, SIGNAL(valueChanged(double)), treeView, SLOT(setMinTimeInf(double)));
    connect(ui->spinBox_end, SIGNAL(valueChanged(double)), treeView, SLOT(setMaxTimeInf(double)));
    connect(ui->spinBox_originS, SIGNAL(valueChanged(double)), treeView, SLOT(setMinTimeSup(double)));
    connect(ui->spinBox_endS, SIGNAL(valueChanged(double)), treeView, SLOT(setMaxTimeSup(double)));
    connect(treeView, SIGNAL(mouseMoved(QString)), ui->timeDisplay, SLOT(setText(QString)));
    tree->minTimeInt.inf = ui->spinBox_origin->value();
    tree->minTimeInt.sup = ui->spinBox_originS->value();
    tree->minTime = (ui->spinBox_origin->value()+ui->spinBox_originS->value())/2.;
    tree->maxTimeInt.inf = ui->spinBox_end->value();
    tree->maxTimeInt.sup = ui->spinBox_endS->value();
    tree->maxTime = (ui->spinBox_end->value()+ui->spinBox_endS->value())/2.;
    model = new QTableResultModel(4, 2, this);
    model->setHeaderRow(0,QString("Speciation"));
    model->setHeaderRow(1, QString("Extinction"));
    model->setHeaderRow(2, QString("Fossil find"));
    model->setHeaderRow(3, QString("Log Likelihood"));
    model->setHeaderColumn(0, QString("Mean"));
    model->setHeaderColumn(1, QString("Std Dev"));
    ui->tableView->setModel(model);
    ui->tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    nloptOption.infSpe = 0.;
    nloptOption.supSpe = 1.;
    nloptOption.infExt = 0.;
    nloptOption.supExt = 1.;
    nloptOption.infFos = 0.;
    nloptOption.supFos = 1.;
    nloptOption.trials = 10;
    nloptOption.tolOptim = 0.00001;
    nloptOption.maxIter = 500;
    ui->lineEdit_comp->setText(QString("0"));
    QObject::connect(treeView, SIGNAL(change()), this, SLOT(changeTree()));
    estimater = new GEstimater(this);
    connect(estimater, &GEstimater::newEstimation, this, &DiversificationWindow::setRates);
    connect(estimater, &GEstimater::setProgress, ui->progressBar, &QProgressBar::setValue);
    connect(estimater, &GEstimater::estimationCompleted, this, &DiversificationWindow::restoreCompute);

}

DiversificationWindow::~DiversificationWindow()
{
 delete ui;
 delete model;
}

void DiversificationWindow::loadTree (QString input_file_name) {
    FILE *fi;
    char* name = strdup(qPrintable(input_file_name));
    if((fi = fopen(name, "r"))) {
        setlocale(LC_ALL, "C");
        tree = Tree::readTree(fi);
        fclose(fi);
        if(tree == NULL)
            return;
        Tree::toBinary(tree);
        Tree::reorderTree(tree->name, tree);
        TypeFossilIntFeature *f = fosToFossilInt(tree);
        double contemp = Tree::getMaximumLeafTime(tree);
        double min, max;

        min = getMinFossilIntTime((TypeFossilIntFeature*) f);
        max = getMaxFossilIntTime((TypeFossilIntFeature*) f);
        if(max == min)
               max++;
        if(contemp != NO_TIME && max<contemp)
            max = contemp;
        ui->spinBox_originS->setMaximum(min);
        if(tree->minTimeInt.inf == NO_TIME || tree->minTimeInt.sup == NO_TIME) {
            tree->minTimeInt.inf = min-(max-min)/5.;
            tree->minTimeInt.sup = min-(max-min)/10.;
        }
        ui->spinBox_origin->setValue(tree->minTimeInt.inf);
        ui->spinBox_originS->setValue(tree->minTimeInt.sup);
        ui->spinBox_origin->setMaximum(ui->spinBox_originS->value());
        ui->spinBox_originS->setMinimum(ui->spinBox_origin->value());
        if(contemp == NO_TIME || max>contemp) {
            ui->spinBox_end->setMinimum(max);
            if(tree->maxTimeInt.inf == NO_TIME || tree->maxTimeInt.sup == NO_TIME) {
                tree->maxTimeInt.inf = max+(max-min)/10.;
                tree->maxTimeInt.sup = max+(max-min)/5.;
            }
            ui->spinBox_end->setValue(tree->maxTimeInt.inf);
            ui->spinBox_endS->setValue(tree->maxTimeInt.sup);
            ui->spinBox_end->setMaximum(ui->spinBox_endS->value());
            ui->spinBox_endS->setMinimum(ui->spinBox_end->value());
        } else {
            max = contemp;
            if(max == min)
                max++;
            ui->spinBox_end->setMinimum(max);
            if(tree->maxTimeInt.inf == NO_TIME || tree->maxTimeInt.sup == NO_TIME) {
                tree->maxTimeInt.inf = max;
                tree->maxTimeInt.sup = max;
            }
            ui->spinBox_end->setValue(tree->maxTimeInt.inf);
            ui->spinBox_endS->setValue(tree->maxTimeInt.sup);
            ui->spinBox_end->setMaximum(ui->spinBox_endS->value());
            ui->spinBox_endS->setMinimum(ui->spinBox_end->value());
        }
        treeView->setMinMaxTime(tree->minTimeInt.inf, tree->maxTimeInt.sup);
        treeView->setTreeFossil(tree, f);
        setWindowTitle(input_file_name);
        newed = false;
    } else {
        warning(QString(tr("IO error")), (QString(tr("Unable to read\n"))).append(input_file_name));
    }
   free((void*)name);
 }

void DiversificationWindow::loadFossil (QString input_file_name) {
    FILE *fi;
    char* name = strdup(qPrintable(input_file_name));
    if((fi = fopen(name, "r"))) {
        setlocale(LC_ALL, "C");
        if(fossilInt != NULL)
            freeFossilIntFeature(fossilInt);
        fossilInt = getFossilIntFeature(fi, tree->name, tree->size);
        fclose(fi);
    } else {
        warning(QString(tr("IO error")), (QString(tr("Unable to read\n"))).append(input_file_name));
    }
    free((void*)name);
    negateFossilInt(fossilInt);
    double min, max;
    min = getMinFossilIntTime((TypeFossilIntFeature*) fossilInt);
    max = getMaxFossilIntTime((TypeFossilIntFeature*) fossilInt);
    if(max == min)
           max++;
    ui->spinBox_originS->setMaximum(min);
    ui->spinBox_originS->setValue(min-(max-min)/5.);
    ui->spinBox_origin->setValue(min-(max-min)/10.);
    ui->spinBox_origin->setMaximum(ui->spinBox_originS->value());
    ui->spinBox_originS->setMinimum(ui->spinBox_origin->value());
    ui->spinBox_end->setMinimum(max);
    ui->spinBox_end->setValue(max+(max-min)/10.);
    ui->spinBox_endS->setValue(max+(max-min)/5.);
    ui->spinBox_end->setMaximum(ui->spinBox_endS->value());
    ui->spinBox_endS->setMinimum(ui->spinBox_end->value());
//    treeView->setMinMaxTime(ui->spinBox_origin->value(), ui->spinBox_end->value());
    treeView->setFossil(fossilInt);
}


void DiversificationWindow::changeTree()
{
    TypeTree *tree;
    TypeFossilIntFeature *fos;
    TypeFossilFeature *ff;
    double comp;
    tree = treeView->getTree();
    fos = treeView->getFossil();
fprintTreeFossilInt(stdout, tree, fos);
    ff = sampleFossilInt(fos, tree->size);
    comp = getItemNumber(tree, ff);
    freeFossilFeature(ff);
    char buffer[50];
    if(log10(fabs(comp))<10)
        sprintf(buffer, "%.0lf", comp);
    else
        sprintf(buffer, "%.2lE", comp);
    ui->lineEdit_comp->setText(QString(buffer));
}

void DiversificationWindow::changeWidth()
{
//    setFixedWidth(ui->scrollArea->widget()->width());
}

void DiversificationWindow::changeTimeOrigin()
{
    ui->spinBox_originS->setMinimum(ui->spinBox_origin->value());
//    treeView->setMinMaxTime(ui->spinBox_origin->value(), ui->spinBox_end->value());
}




void DiversificationWindow::changeTimeOriginS()
{
/*fprintf(stdout, "\nA o %.2lf/%.2lf/%.2lf os %.2lf/%.2lf/%.2lf\n",
        ui->spinBox_origin->minimum(), ui->spinBox_origin->value(), ui->spinBox_origin->minimum(),
        ui->spinBox_originS->minimum(), ui->spinBox_originS->value(), ui->spinBox_originS->minimum());
*/
    ui->spinBox_origin->setMaximum(ui->spinBox_originS->value());
    if((ui->spinBox_originS->value()+SIMAL)>ui->spinBox_end->minimum())
        ui->spinBox_end->setMinimum(ui->spinBox_originS->value()+SIMAL);
    if((ui->spinBox_originS->value()+SIMAL)>ui->spinBox_end->value())
        ui->spinBox_end->setValue(ui->spinBox_originS->value()+SIMAL);
/*fprintf(stdout, "\nA o %.2lf/%.2lf/%.2lf os %.2lf/%.2lf/%.2lf\n",
        ui->spinBox_origin->minimum(), ui->spinBox_origin->value(), ui->spinBox_origin->minimum(),
        ui->spinBox_originS->minimum(), ui->spinBox_originS->value(), ui->spinBox_originS->minimum());
fflush(stdout);
*/
}

void DiversificationWindow::changeTimeEnd()
{
    ui->spinBox_endS->setMinimum(ui->spinBox_end->value());
    if((ui->spinBox_end->value()-SIMAL)<ui->spinBox_originS->maximum())
        ui->spinBox_originS->setMaximum(ui->spinBox_end->value()-SIMAL);
    if((ui->spinBox_end->value()-SIMAL)<ui->spinBox_originS->value())
        ui->spinBox_originS->setValue(ui->spinBox_end->value()-SIMAL);
}

void DiversificationWindow::changeTimeEndS()
{
    ui->spinBox_end->setMaximum(ui->spinBox_endS->value());
//    treeView->setMinMaxTime(ui->spinBox_origin->value(), ui->spinBox_endS->value());
}

void DiversificationWindow::openTree()
{
    if(!maybeSave())
        return;
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        loadTree (fileName);
}

void DiversificationWindow::newTree()
{
    if(!maybeSave())
        return;
    TypeTree *tree;
    tree = Tree::newTree(INC_SIZE_TREE);
    tree->size = 1;
    tree->root = 0;
    tree->node[tree->root].child = NOSUCH;
    tree->node[tree->root].sibling = NOSUCH;
    tree->minTimeInt.inf = 0;
    tree->minTimeInt.sup = 0;
    tree->minTime = (ui->spinBox_origin->value()+ui->spinBox_originS->value())/2.;
    tree->maxTimeInt.inf = 100;
    tree->maxTimeInt.sup = 100;
    tree->maxTime = (ui->spinBox_end->value()+ui->spinBox_endS->value())/2.;
    ui->spinBox_origin->setValue(tree->minTimeInt.inf);
    ui->spinBox_originS->setValue(tree->minTimeInt.sup);
    ui->spinBox_origin->setMaximum(ui->spinBox_originS->value());
    ui->spinBox_originS->setMinimum(ui->spinBox_origin->value());
    ui->spinBox_end->setValue(tree->maxTimeInt.inf);
    ui->spinBox_endS->setValue(tree->maxTimeInt.sup);
    ui->spinBox_end->setMaximum(ui->spinBox_endS->value());
    ui->spinBox_endS->setMinimum(ui->spinBox_end->value());
    setWindowTitle("Untitled.newick");
    newed = true;
    TypeFossilIntFeature *f = newFossilIntFeature(INC_SIZE_TREE);
    treeView->setTreeFossil(tree, f);
    treeView->setMinMaxTime(tree->minTimeInt.inf, tree->maxTimeInt.sup);
    treeView->setModified(false);
}

void DiversificationWindow::openFossil()
{
    if(tree != NULL) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty()) {
            loadFossil(fileName);
         }
    } else {
        warning(QString(tr("Execution")), QString(tr("Load a tree before loading fossils file")));
    }
}

bool DiversificationWindow::saveAs() {
    GSaveOptionDialog optDialog(new GSaveOption(QList<GSaveType>() << GSaveType(QString("Tree picture as"), &(GSaveOption::imageFormat))
                                            << GSaveType(QString("Tree as"), &(GSaveOption::treeFormat))
                                            << GSaveType(QString("Rates as"), &(GSaveOption::tableFormat))
                                            << GSaveType(QString("Report"), &(GSaveOption::textFormat))
                                            ));
    optDialog.setWindowTitle("Export");
    if(optDialog.exec()) {
        switch(optDialog.getType()) {
            case 0:
                return saveImage(optDialog.getFormat());
            case 1:
                switch(optDialog.getFormat()) {
                    case 0:
                        setWindowTitle(treeView->saveSubtree(treeView->getTree()->root));
                        return true;
                    case 1:
                    default:
                        (treeView->saveSubtree(treeView->getTree()->root));
                        return true;
                }
            case 2:
                return saveTable(optDialog.getFormat());
            case 3:
                return saveReport();
            default:
                return false;
        }
        return true;
    } else
        return false;

}


void DiversificationWindow::writePDF(QString filename)
{
    QPdfWriter writer(filename);
    QTreeFossilDrawer drawer(treeView->getFossil(), treeView->getTree(), ui->spinBox_origin->value(), ui->spinBox_endS->value());
    QPainter painter(&writer);
    writer.setPageSize(QPagedPaintDevice::A4);
    drawer.setMetrics(&painter, writer.height());
    drawer.setWidth(writer.width());
    drawer.paintTree(&painter, 0, drawer.drawScaleGenericUp(&painter, 0.,0.));
    painter.end();
}

void DiversificationWindow::writePNG(QString filename)
{
    int width = 800;
    QTreeFossilDrawer drawer(treeView->getFossil(), treeView->getTree(), ui->spinBox_origin->value(), ui->spinBox_endS->value());
    {
        QPixmap pixmap_tmp(width, 200);
        QPainter painter_tmp(&pixmap_tmp);
        drawer.setMetrics(&painter_tmp, INT_MAX);
    }
    drawer.setWidth(width);
    QPixmap pixmap(width, drawer.getHeight());
    QPainter painter(&pixmap);
    drawer.setMetrics(&painter, INT_MAX);
    pixmap.fill( Qt::white );
    drawer.paintTree(&painter, 0, drawer.drawScaleGenericUp(&painter, 0.,0.));
    painter.end();
    pixmap.save( filename );
}

void DiversificationWindow::writeSVG(QString filename)
{
    int width = 800;
    QTreeFossilDrawer drawer(treeView->getFossil(), treeView->getTree(), ui->spinBox_origin->value(), ui->spinBox_endS->value());
    {
        QSvgGenerator tmp;
        tmp.setFileName(filename);
        tmp.setSize(QSize(width, 1000));
        QPainter painter_tmp;
        painter_tmp.begin(&tmp);
        drawer.setMetrics(&painter_tmp, INT_MAX);
        painter_tmp.end();
    }
    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(QSize(width, drawer.getHeight()));
    QPainter painter;
    painter.begin(&generator);
    drawer.setMetrics(&painter, INT_MAX);
    drawer.paintTree(&painter, 0, drawer.drawScaleGenericUp(&painter, 0.,0.));
    painter.end();
}

void DiversificationWindow::writePSTricks(QString filename)
{
    FILE *f;
    if((f = fopen(qPrintable(filename), "w"))) {
        TypeTree tmp, *tree = treeView->getTree();
        TypeFossilIntFeature *fos = treeView->getFossil();
        tmp.node = tree->node;
        tmp.size = tree->size;
        tmp.minTime = ui->spinBox_origin->value();
        tmp.maxTime = ui->spinBox_endS->value();
        tmp.name = tree->name;
        tmp.comment = NULL;
        tmp.parent = NULL;
        tmp.root = tree->root;
        GTreeFossilGeometry geo = GTreeFossilGeometry(tree, fos, tmp.minTime, tmp.maxTime);
        geo.fillTreeGeometry(0, 0, 200, 10);
        tmp.time = geo.getTimeTable();
        drawTreeFossilFilePst(f,  &tmp, fos);
        fclose(f);
    }
}

bool DiversificationWindow::saveImage(int type) {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setWindowTitle("Save tree picture as");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec()) {
        files = dialog.selectedFiles();
        switch(type) {
            case 0:
                writePDF(dialog.selectedFiles().at(0));
                break;
            case 1:
                writePNG(dialog.selectedFiles().at(0));
                break;
            case 2:
                writeSVG(dialog.selectedFiles().at(0));
                break;
            case 3:
                writePSTricks(dialog.selectedFiles().at(0));
                break;
            default:
                writePNG(dialog.selectedFiles().at(0));
                break;
        }
        return true;
    } else
        return false;
}

bool DiversificationWindow::saveTable(int type)
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec()) {
        FILE *f;
        if((f = fopen(qPrintable(dialog.selectedFiles().at(0)), "w"))) {
            switch(type) {
                case 1:
                    GUtil::writeTableLatex(f, model);
                    break;
                case 0:
                default:
                    GUtil::writeTableCSV(f, model);
            }
            fclose(f);
            return true;
        } else
            return false;
    } else
        return false;
}

bool DiversificationWindow::saveReport()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        FILE *f;
        char* name = strdup(qPrintable(dialog.selectedFiles().at(0)));
        if((f = fopen(name, "w"))) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char buffer[50];
            fprintf(f, "Diversification execution report\n%d/%d/%d at %d:%d\n\n", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
            fprintf(f, "\nInputs\n");
            fprintf(f, "Origin time:\t%.2lf:%.2lf\n", ui->spinBox_origin->value(), ui->spinBox_originS->value());
            fprintf(f, "End time:\t%.2lf:%.2lf\n", ui->spinBox_end->value(), ui->spinBox_endS->value());
            fprintf(f, "Number of samplings:\t%d\n", ui->spinBox_samples->value());
            fprintf(f, "\nEstimations\tMean\tStandard Deviation\n");
            sprintf(buffer, "%.3lE", estimationMean.param.birth);
            fprintf(f, "Speciation rate:\t%s", buffer);
            sprintf(buffer, "%.3lE", estimationStd.param.birth);
            fprintf(f, "\t%s\n", buffer);
            sprintf(buffer, "%.3lE", estimationMean.param.death);
            fprintf(f, "Extinction rate:\t%s", buffer);
            sprintf(buffer, "%.3lE", estimationStd.param.death);
            fprintf(f, "\t%s\n", buffer);
            sprintf(buffer, "%.3lE", estimationMean.param.fossil);
            fprintf(f, "Fossil find rate:\t%s", buffer);
            sprintf(buffer, "%.3lE", estimationStd.param.fossil);
            fprintf(f, "\t%s\n", buffer);
            sprintf(buffer, "%.3lE", estimationMean.logLikelihood);
            fprintf(f, "Log-likelihood:\t%s", buffer);
            sprintf(buffer, "%.3lE", estimationStd.logLikelihood);
            fprintf(f, "\t%s\n", buffer);
            fprintf(f, "\nOptimization settings\n");
            fprintNLoptOption(f, &nloptOption);
            fclose(f);
        }
        free((void*) name);
        return true;
    } else
        return false;
}

bool DiversificationWindow::save() {
    if(newed) {
        QString name = treeView->saveSubtree(treeView->getTree()->root);
        if(name != 0) {
            setWindowTitle(name);
            newed = false;
            treeView->setModified(false);
            return true;
        } else
            return false;
    } else {
        treeView->saveTree(windowTitle());
        treeView->setModified(false);
        return true;
    }
}

bool DiversificationWindow::maybeSave()
{
    if (treeView->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Diversification"),
                     tr("The tree has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void DiversificationWindow::help()
{
    helpDialog->show();
}

void DiversificationWindow::about()
{
   QMessageBox::about(this, tr("About Diversification"),
            tr("<b>Diversification</b> estimates speciation, extinction and fossil find rates from the fossil record"));
}


//\nGilles Didier\ngilles.didier@univ-amu.fr\nInstitut de Mathématiques de Marseille\nSite Sud, Campus de Luminy, Case 907\n13288 MARSEILLE Cedex 9

void DiversificationWindow::warning(QString title, QString text)
{
    QMessageBox::warning(this, title, text );
}

void DiversificationWindow::setRates(TypeEstimation mean, TypeEstimation std) {
    estimationMean = mean;
    estimationStd = std;
    model->setEntry(model->index(0, 0), mean.param.birth, false);
    model->setEntry(model->index(1, 0), mean.param.death, false);
    model->setEntry(model->index(2, 0), mean.param.fossil, false);
    model->setEntry(model->index(3, 0), mean.logLikelihood, false);
    model->setEntry(model->index(0, 1), std.param.birth, false);
    model->setEntry(model->index(1, 1), std.param.death, false);
    model->setEntry(model->index(2, 1), std.param.fossil, false);
    model->setEntry(model->index(3, 1), std.logLikelihood, false);
    model->emit dataChanged(model->index(0, 0), model->index(3, 1));
}

void DiversificationWindow::restoreCompute() {
    ui->actionCompute->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
    ui->progressBar->setEnabled(false);
    ui->progressBar->setValue(0);
}

void DiversificationWindow::getOptimOptions()
{
    GOptionDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setOption(nloptOption);
    if (dialog.exec())
        nloptOption = dialog.getOption();
}


void DiversificationWindow::stopCompute() {
    estimater->stop();
}

void DiversificationWindow::computeThread()
{
    QThread::idealThreadCount();
/*    TypeFossilIntFeature *f;
    TypeTree *t;
    t = treeView->getTree();
    f = treeView->getFossil();
    if(t == NULL) {
        warning(QString(tr("Execution")), QString(tr("Please, load a tree !")));
        return;
    }
    if(f == NULL) {
        warning(QString(tr("Execution")), QString(tr("Please, load fossils !")));
        return;
    }
    thread = new QComputeThread(t, f, ui->spinBox_samples->value(), this);
    qRegisterMetaType<TypeModelParam>("TypeModelParam");
    connect(thread, &QComputeThread::resultReady, this, &DiversificationWindow::setRates);
    connect(thread, &QComputeThread::setProgress, ui->progressBar, &QProgressBar::setValue);
    connect(thread, &QComputeThread::finished, thread, &QObject::deleteLater);
    ui->actionCompute->setEnabled(false);
    ui->progressBar->setEnabled(true);
    ui->pushButton_stop->setEnabled(true);
    thread->start();
*/}

void DiversificationWindow::compute()
{

    ui->actionCompute->setEnabled(false);
    ui->progressBar->setEnabled(true);
    ui->pushButton_stop->setEnabled(true);
    estimater->start(getLogLikelihoodTreeFossil, treeView->getTree(), treeView->getFossil(), &nloptOption, ui->spinBox_samples->value());
return;

/*
    TypeTree *tree;
    TypeFossilIntFeature *fos;
    TypeEstimation *estimation, mean, std;
    TypeFossilFeature *ff;
    double *timeSave;
    int n;
    tree = treeView->getTree();
    fos = treeView->getFossil();
    if(tree == NULL) {
        warning(QString(tr("Execution")), QString(tr("Please, load a tree !")));
        return;
    }
    if(fos == NULL || fos->sizeFossil == 0) {
        warning(QString(tr("Execution")), QString(tr("Please, it really needs fossils !")));
        return;
    }
    int i, size = 0, iter = ui->spinBox_samples->value();
    timeSave = tree->time;
    tree->time = (double*) malloc(tree->size*sizeof(double));
    fprintf(stdout, "maxTime %.2le\n", tree->maxTime);
    tree->maxTime = ui->spinBox_end->value();
    for(int n=0; n<tree->size; n++)
        if(tree->name[n]!=NULL)
            fprintf(stdout, "%d\t%s\n", n, tree->name[n]);
    for(n=0; n<tree->size; n++) {
        if(tree->node[n].child == NOSUCH)
            fprintf(stdout, "%d\t%d\t%.2le\n", n, fos->fossilInt[n], timeSave[n]);
        if(tree->node[n].child == NOSUCH && (timeSave[n] != NO_TIME || fos->fossilInt[n] == NOSUCH))
            tree->time[n] = tree->maxTime;
        else
            tree->time[n] = NO_TIME;
        if(tree->node[n].child == NOSUCH)
            fprintf(stdout, "%d\t%.2le\n", n, tree->time[n]);
    }
    fprintTreeFossilInt(stdout, tree, fos);
    fprintf(stdout, "\n\n");
    tree->minTime = ui->spinBox_origin->value();
    tree->maxTime = ui->spinBox_end->value();
    ff = sampleFossilInt(fos, tree->size);
 //        fprintTreeFossil(stdout, tree, ff);
    fixTreeFossil(tree, ff);
    TypeModelParam param;
    param.birth = 2;
    param.death = 1;
    param.fossil = 0.;
Tree::fprintTreeX(stdout, tree);
fprintf(stdout, "\n");
double like = getLogLikelihoodTreeFossil(tree, ff, &param);
    fprintf(stdout, "like %.2lE\n\n", like);
fflush(stdout);
return;
TypeTree treeTmp;
//TypeFossilFeature *ff;
//ff = sampleFossilInt(fos, tree->size);
treeTmp.time = (double*) malloc(tree->size*sizeof(double));
treeTmp.size = tree->size;
treeTmp.sizeBuf = tree->sizeBuf;
treeTmp.root = tree->root;
treeTmp.node = tree->node;
treeTmp.name = NULL;
treeTmp.comment = NULL;
treeTmp.info = NULL;
treeTmp.parent = NULL;
treeTmp.minTime = tree->minTimeInt.inf+UNIF_RAND*(tree->minTimeInt.sup-tree->minTimeInt.inf);
treeTmp.maxTime = tree->maxTimeInt.inf+UNIF_RAND*(tree->maxTimeInt.sup-tree->maxTimeInt.inf);
for(int n=0; n<tree->size; n++) {
    if(tree->node[n].child == NOSUCH) {
        switch(fos->status[n]) {
            case contempNodeStatus:
                treeTmp.time[n] = treeTmp.maxTime;
            break;
            case extinctNodeStatus:
                treeTmp.time[n] = NO_TIME;
            break;
            case unknownNodeStatus:
                treeTmp.time[n] = fos->endTimeTable[fos->endTime[n]].inf+UNIF_RAND*(fos->endTimeTable[fos->endTime[n]].sup-fos->endTimeTable[fos->endTime[n]].inf);
            break;
            default:
                fprintf(stderr, "Node %d has no status\n", n);
                return;
        }
    } else
        treeTmp.time[n] = NO_TIME;
}
Tree::fprintTreeX(stdout, &treeTmp);
fprintf(stdout, "\n");
fprintf(stdout, "likex %.2lE\n", getLogLikelihoodTreeFossil(&treeTmp, ff, &param));
fflush(stdout);
free((void*)treeTmp.time);
return;
ui->actionCompute->setEnabled(false);
ui->progressBar->setEnabled(true);
ui->pushButton_stop->setEnabled(true);
estimater->start(getLogLikelihoodTreeFossil, treeView->getTree(), treeView->getFossil(), &nloptOption, ui->spinBox_samples->value());
return;
        return;
/*    estimation = new TypeEstimation[iter];
    tree->minTime = ui->spinBox_origin->value();
    tree->maxTime = ui->spinBox_end->value();
    for(i=0; i<iter && ! QThread::currentThread()->isInterruptionRequested(); i++) {
        tree->minTime = ui->spinBox_origin->value()+UNIF_RAND*(ui->spinBox_originS->value()-ui->spinBox_origin->value());
        tree->maxTime = ui->spinBox_end->value()+UNIF_RAND*(ui->spinBox_endS->value()-ui->spinBox_end->value());
        ff = sampleFossilInt(fos, tree->size);
        fprintf(stdout, "\niter %d\n", i);
//        fprintTreeFossil(stdout, tree, ff);
        fixTreeFossil(tree, ff);
//        if(!minimizeBirthDeathFossil(getLogLikelihoodTreeFossil, tree, ff, &(estimation[size]))) {
        if(!minimizeBirthDeathFossilFromTreeFossil(getLogLikelihoodTreeFossil, tree, ff, &nloptOption, &(estimation[size]))) {
            warning(QString(tr("Execution")), QString(tr("Problem during estimation")));
        } else
            size++;
         freeFossilFeature(ff);
    }
    free((void*)tree->time);
    tree->time = timeSave;
    mean.param.birth = 0.;
    mean.param.death = 0.;
    mean.param.fossil = 0.;
    mean.logLikelihood = 0.;
    std.param.birth = 0.;
    std.param.death = 0.;
    std.param.fossil = 0.;
    std.logLikelihood = 0.;
      if(size>0) {
         for(i=0; i<size; i++) {
             mean.param.birth += estimation[i].param.birth;
             mean.param.death += estimation[i].param.death;
             mean.param.fossil += estimation[i].param.fossil;
             mean.logLikelihood += estimation[i].logLikelihood;
         }
         mean.param.birth /= (double) size;
         mean.param.death /= (double) size;
         mean.param.fossil /= (double) size;
         mean.logLikelihood /= (double) size;
         if(size>1) {
             for(i=0; i<size; i++) {
                 std.param.birth += pow(estimation[i].param.birth-mean.param.birth, 2.);
                 std.param.death += pow(estimation[i].param.death-mean.param.death, 2.);
                 std.param.fossil += pow(estimation[i].param.fossil-mean.param.fossil, 2.);
                 std.logLikelihood += pow(estimation[i].logLikelihood-mean.logLikelihood, 2.);
             }
             std.param.birth /= (double) size-1.;
             std.param.death /= (double) size-1.;
             std.param.fossil /= (double) size-1.;
             std.logLikelihood /= (double) size-1.;
             std.param.birth = sqrt(std.param.birth);
             std.param.death = sqrt(std.param.death);
             std.param.fossil = sqrt(std.param.fossil);
             std.logLikelihood = sqrt(std.logLikelihood);
         }
     }
    delete[] estimation;
    setRates(mean, std);
*/
}
