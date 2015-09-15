#ifndef QSCALETREE_H
#define QSCALETREE_H

#include <QWidget>
#include "qtreedrawer.h"

class QScaleTree : public QWidget
{
    Q_OBJECT
public:
    explicit QScaleTree(QTreeDrawer *p, QWidget *parent = 0);
    ~QScaleTree();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QTreeDrawer *treePainter;
};

#endif // QSCALETREE_H
