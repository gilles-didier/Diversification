#ifndef DIVERSIFICATIONWINDOW_H
#define DIVERSIFICATIONWINDOW_H

#include <QMainWindow>
#include "Tree.h"
#include "FossilInt.h"
#include "Model.h"
#include "qtreedrawer.h"
#include "qtreefossildrawer.h"
#include "qtreedrawwidget.h"
#include "qscaletree.h"
#include "ghelpdialog.h"
#include "gtreefossilview.h"
#include "qtableresultmodel.h"
#include "gestimater.h"
#include "MinimizeNLOpt.h"

namespace Ui {
class DiversificationWindow;
}

class DiversificationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DiversificationWindow(QWidget *parent = 0);
    ~DiversificationWindow();

private slots:
    void openFossil();
    void openTree();
    bool saveAs();
    bool save();
    void newTree();
    bool saveImage(int type);
    void about();
    void help();
    void changeWidth();
    void compute();
    void computeThread();
    void changeTimeOrigin();
    void changeTimeOriginS();
    void changeTimeEnd();
    void changeTimeEndS();
    void changeTree();
    void setRates(TypeEstimation mean, TypeEstimation std);
    void stopCompute();
    void getOptimOptions();
    void restoreCompute();
private:
    Ui::DiversificationWindow *ui;
    TypeTree *tree;
    TypeFossilIntFeature *fossilInt;
    TypeEstimation estimationMean, estimationStd;
    GTreeFossilView *treeView;
    QTreeFossilDrawer *treeDrawer;
    QTreeDrawWidget *treeWidget;
    QScaleTree *scaleTree;
    GHelpDialog *helpDialog;
    QTableResultModel *model;
    TypeNLOptOption nloptOption;
    GEstimater *estimater;
    bool newed;
    bool maybeSave();
    void loadTree (QString inputTreeFileName);
    void loadFossil (QString input_file_name);
    void warning(QString title, QString text);
    bool saveReport();
    bool saveTable(int type);
    void writePDF(QString filename);
    void writePNG(QString filename);
    void writeSVG(QString filename);
    void writePSTricks(QString filename);
};

#endif // DIVERSIFICATIONWINDOW_H
