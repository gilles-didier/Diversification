#ifndef QTREEDRAWWIDGET_H
#define QTREEDRAWWIDGET_H

#include <QWidget>
#include "Tree.h"
#include "qtreedrawer.h"

class QTreeDrawWidget : public QWidget
{
    Q_OBJECT
public:
 //   explicit
    QTreeDrawWidget(QTreeDrawer *t, QWidget *parent = NULL);

    ~QTreeDrawWidget();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:

private:
QTreeDrawer *treePainter;
};

#endif // QTREEDRAWWIDGET_H
