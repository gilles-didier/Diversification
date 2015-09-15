#include "gsaveoption.h"

QList<QString> GSaveOption::imageFormat = QList<QString>() << QString("PDF") << QString("PNG") << QString("SVG") << QString("PSTricks");
QList<QString> GSaveOption::tableFormat = QList<QString>() << QString("CSV") << QString("LaTeX");
QList<QString> GSaveOption::textFormat = QList<QString>() << QString("TXT");
QList<QString> GSaveOption::treeFormat = QList<QString>() << QString("Newick") << QString("Newick with fossil tags");


