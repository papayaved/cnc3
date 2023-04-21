#ifndef COMBO_DELEGATE_H
#define SOMBO_DELEGATE_H

#include <QStyledItemDelegate>

//! [0]
class ComboBoxDelegate : public QStyledItemDelegate {
    Q_OBJECT

    const QString m_off = tr("Off");
    const QString m_on = tr("On");

public:
    ComboBoxDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
//! [0]

#endif
