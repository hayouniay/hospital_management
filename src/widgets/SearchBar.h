#pragma once
#include <QWidget>
namespace HMS {
class SearchBar : public QWidget {
    Q_OBJECT
public:
    explicit SearchBar(QWidget* parent = nullptr);
};
} // namespace HMS
