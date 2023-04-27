#ifndef MOVE_SEG_DIALOG_H
#define MOVE_SEG_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <vector>
//#include "fpoint_t.h"
#include "dxf_entity.h"

class MoveSegDialog : public QDialog {
    Q_OBJECT

    QLabel *labelCtrName {nullptr}, *labelCtrValue {nullptr}, *labelSegName {nullptr}, *labelSegValue {nullptr};
    QLabel *labelContour {nullptr};

    QComboBox *comboContour {nullptr};

    std::vector<QWidget*> m_widgets;

    static void setFontPointSize(QWidget *w, int pointSize);

public:
    QDialogButtonBox* buttonBox;

    explicit MoveSegDialog(const QStringList& contours_list, size_t ctr_num, size_t sel_segs_num, QWidget *parent = nullptr);
    ~MoveSegDialog();

    int contourNumber() const;
    void setFontPointSize(int pointSize);
};

#endif // MOVE_SEG_DIALOG_H
