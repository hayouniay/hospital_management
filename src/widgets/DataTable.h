#pragma once
#include <QWidget>
namespace HMS {
class DataTable : public QWidget {
    Q_OBJECT
public:
    explicit DataTable(QWidget* parent = nullptr);
};
} // namespace HMS
