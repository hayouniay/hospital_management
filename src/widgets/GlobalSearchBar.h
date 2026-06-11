#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer>
#include <QFrame>

namespace HMS {

class GlobalSearchBar : public QWidget {
    Q_OBJECT
public:
    explicit GlobalSearchBar(QWidget* parent = nullptr);
    void focusSearch();

signals:
    void navigateTo(int pageIndex);        // Switch sidebar page
    void patientSelected(qint64 id);
    void doctorSelected(qint64 id);

private slots:
    void onTextChanged(const QString& text);
    void onResultClicked(QListWidgetItem* item);
    void performSearch();

private:
    void setupUi();
    void showResults();
    void hideResults();

    QLineEdit*   m_edit{nullptr};
    QFrame*      m_popup{nullptr};
    QListWidget* m_resultList{nullptr};
    QTimer*      m_debounce{nullptr};
};

} // namespace HMS
