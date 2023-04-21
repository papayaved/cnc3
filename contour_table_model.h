#ifndef CONTOURMODEL_H
#define CONTOURMODEL_H

#include <QAbstractTableModel>
#include "contour_list.h"

class ContourTableModel : public QAbstractTableModel {
    Q_OBJECT

    const ContourPair* m_pair;
public:
    explicit ContourTableModel(QObject* parent = nullptr);
    ContourTableModel(const ContourPair* contour, QObject *parent = nullptr);
    void bind(const ContourPair* contour);
    void unbind();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

class ContoursModel : public QAbstractTableModel {
    Q_OBJECT

    const ContourList* m_contours;
public:
    ContoursModel(QObject *parent = nullptr);
    ContoursModel(const ContourList* contours, QObject *parent = nullptr);
    void bind(const ContourList* contours);
    void unbind();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // CONTOURMODEL_H
