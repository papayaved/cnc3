#include "combobox_delegate.h"

#include <QComboBox>

//! [0]
ComboBoxDelegate::ComboBoxDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
//! [0]

//! [1]
QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const {
    QComboBox *editor = new QComboBox(parent);
    editor->setFrame(false);

    editor->addItem(m_off);
    editor->addItem(m_on);

    return editor;
}
//! [1]

//! [2]
void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString value = index.model()->data(index, Qt::DisplayRole).toString();

    if (value.trimmed().toLower() == m_on.toLower())
        comboBox->setCurrentIndex(1);
    else
        comboBox->setCurrentIndex(0);
}
//! [2]

//! [3]
void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QComboBox *comboBox = static_cast<QComboBox*>(editor);

    model->setData(index, comboBox->currentIndex() != 0, Qt::EditRole);
}
//! [3]

//! [4]
void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const {
    editor->setGeometry(option.rect);
}
//! [4]
