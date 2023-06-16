#include "contour_table_model.h"
#include <QDebug>
#include "main.h"

SegmentsModel::SegmentsModel(QObject *parent) : QAbstractTableModel(parent), m_pair(nullptr) {}
SegmentsModel::SegmentsModel(const ContourPair* contour, QObject* parent) : QAbstractTableModel(parent), m_pair(contour) {}

void SegmentsModel::bind(const ContourPair* contour) { m_pair = contour; }
void SegmentsModel::unbind() { m_pair = nullptr; }

int SegmentsModel::rowCount(const QModelIndex & /*parent*/) const {
    return  m_pair ? int(m_pair->count()) : 0;
}

int SegmentsModel::columnCount(const QModelIndex & /*parent*/) const {
#if defined(STONE)
    return 1;
#else
    return 2;
#endif
}

QVariant SegmentsModel::data(const QModelIndex& index, int role) const {
    if (m_pair && index.row() >= 0 && index.column() >= 0) {
        size_t row = size_t(index.row());
        size_t col = size_t(index.column());

        const Contour* const bot = m_pair->bot();
        const Contour* const top = m_pair->top();

        if (role == Qt::DisplayRole)
            switch (col) {
            case 0:
                if (bot && row < bot->count())
                    return bot->at(row)->typeString().c_str();
                break;
            case 1:
                if (top && row < top->count())
                    return top->at(row)->typeString().c_str();
                break;
            }
    }

    return QVariant();
}

QVariant SegmentsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString(tr("XY Plane"));
        case 1:
            return QString(tr("UV Plane"));
        }
    }
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return QString::number(section + 1);
    }
    return QVariant();
}

// ******************************
ContoursModel::ContoursModel(QObject *parent) : QAbstractTableModel(parent), m_contours(nullptr) {}

ContoursModel::ContoursModel(const ContourList *contours, QObject *parent) : QAbstractTableModel(parent), m_contours(contours) {
    if (m_contours) {
        qDebug() << "New ContoursModel. " << m_contours->size();
        qDebug() << m_contours->toString().c_str();
    }
}

void ContoursModel::bind(const ContourList *contours) { m_contours = contours; }
void ContoursModel::unbind() { m_contours = nullptr; }

int ContoursModel::rowCount(const QModelIndex & /*parent*/) const {
    return (m_contours) ? int(m_contours->size()) : 0;
}

int ContoursModel::columnCount(const QModelIndex & /*parent*/) const { return 1; }

QVariant ContoursModel::data(const QModelIndex& index, int role) const {
    if (m_contours && index.row() >= 0 && index.column() >= 0) {
        size_t row = size_t(index.row());
        size_t col = size_t(index.column());

        size_t count = m_contours->size();

        if (role == Qt::DisplayRole)
            switch (col) {
            case 0:
                if (row < count) {
                    const ContourPair* pair = m_contours->at(row);
                    qDebug() << m_contours->toString().c_str();

                    if (pair)
                        return QString::fromStdString(pair->typeToString()) + " (" + QString::number(pair->count()) + ")";
                    else
                        return "No data";
                }
                break;
            }
    }

    return QVariant();
}

QVariant ContoursModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return QString::number(section + 1);
    }
    return QVariant();
}
