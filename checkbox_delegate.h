#ifndef CHECK_DELEGATE_H
#define CHECK_DELEGATE_H

#include <QStyledItemDelegate>

//! [0]
class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CheckBoxDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
//! [0]

#endif
