#pragma once
#include <QWidget>
namespace HMS {
class StatusBadge : public QWidget {
    Q_OBJECT
public:
    explicit StatusBadge(QWidget* parent = nullptr);
};
} // namespace HMS
