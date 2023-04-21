#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QCheckBox>
#include <set>
#include "fpoint_t.h"

class LayerDialog : public QDialog {
    Q_OBJECT

    QLabel *labelTitle, *labelXY, *labelUV, *labelLayer;
    QComboBox *comboXY, *comboUV;
    QCheckBox *checkUV;

    QGroupBox *group, *groupCenter;

    std::vector<QWidget*> m_widgets;

    static QStringList getStringList(const std::set<std::string>& layers);
    static void setFontPointSize(QWidget *w, int pointSize);

public:
    QDialogButtonBox* buttonBox;

    explicit LayerDialog(const std::set<std::string>& layers, QWidget *parent = nullptr);
    ~LayerDialog();

    bool isXY() const;
    bool isUV() const;

    std::string xyLayerName() const;
    std::string uvLayerName() const;

    void setFontPointSize(int pointSize);
};

#endif // LAYERDIALOG_H
