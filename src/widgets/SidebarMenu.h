#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <vector>

namespace HMS {

class SidebarMenu : public QWidget {
    Q_OBJECT
public:
    explicit SidebarMenu(QWidget* parent = nullptr);
    void selectItem(int index);   // public so MainWindow can call it

signals:
    void itemSelected(int index);

private:
    void buildMenu();

    struct MenuItem {
        QString icon;
        QString label;
        QPushButton* button{nullptr};
    };

    std::vector<MenuItem> m_items;
    int m_currentIndex{-1};
};

} // namespace HMS
