#ifndef MODETABLEMODEL_H
#define MODETABLEMODEL_H

#include <QAbstractTableModel>
#include <deque>
#include "program_param.h"

class ModeTableModel : public QAbstractTableModel
{
    Q_OBJECT

    std::deque<GeneratorMode>& m_modes;

public:
//    explicit ModeTableModel(QObject *parent = nullptr);
    ModeTableModel(std::deque<GeneratorMode>& modes, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
};

#endif // MODETABLEMODEL_H
