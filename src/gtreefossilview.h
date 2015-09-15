#ifndef GTREEFOSSILVIEW_H
#define GTREEFOSSILVIEW_H
#include "gtreewidget.h"
#include "FossilInt.h"
#include <QVector>

class GFossil;

class GTreeFossilView : public GTreeWidget
{
    Q_OBJECT

public:
    GTreeFossilView(TypeTree *t = NULL, TypeFossilIntFeature *f = NULL, double min = 0.,double max = 1., QWidget *parent = 0);
    ~GTreeFossilView();
    void setTreeFossil(TypeTree *t, TypeFossilIntFeature *fo);
    void setTree(TypeTree *t);
    void setFossil(TypeFossilIntFeature *fo);
    void addNewLeaf(GNode *n);
    void removeNode(GNode *n);
    void saveTree(QString filename);
    QString saveSubtree(int n);
    TypeFossilIntFeature *getFossil();
    virtual void nodeContextMenu(GNode *n, QPoint pos);
protected:
    TypeFossilIntFeature *fos;
    QVector<GFossil*> fossil;
    void addFossil(GNode *n);
    void editFossil(GNode *n);
    void editNode(GNode *n);
    virtual void layoutTree();
};


#endif // GTREEFOSSILVIEW_H
