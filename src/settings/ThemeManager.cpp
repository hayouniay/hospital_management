#include "ThemeManager.h"

namespace HMS {

ThemeManager& ThemeManager::instance() {
    static ThemeManager inst;
    return inst;
}

void ThemeManager::applyTheme(bool dark) {
    m_dark = dark;
    qApp->setStyleSheet(dark ? darkStylesheet() : lightStylesheet());
    emit themeChanged(dark);
}

QString ThemeManager::lightStylesheet() {
    return R"(
/* ── Global ─────────────────────────────────────────────────── */
QMainWindow, QWidget { background-color: #F0F4F8; color: #1A202C; font-family: 'Segoe UI', Arial, sans-serif; }
QDialog { background-color: #FFFFFF; }

/* ── Top bar ─────────────────────────────────────────────────── */
#topBar { background-color: #FFFFFF; border-bottom: 1px solid #E2E8F0; }
#pageTitle { font-size: 20px; font-weight: 700; color: #1A202C; }
#clockLabel { font-size: 13px; color: #718096; }

/* ── Sidebar ─────────────────────────────────────────────────── */
#sidebar { background-color: #1A365D; }
#sidebarLogo { background-color: #2A4A7F; }
#logoLabel { color: #FFFFFF; font-size: 15px; font-weight: 700; }
#versionLabel { color: #90CDF4; font-size: 10px; }
#menuSectionLabel { color: #90CDF4; font-size: 10px; font-weight: 700; letter-spacing: 1.5px; padding-top: 8px; }
#sidebar QPushButton { background-color: transparent; color: #BEE3F8; text-align: left; padding: 10px 16px; border-radius: 0; font-size: 13px; font-weight: 500; border: none; }
#sidebar QPushButton:hover { background-color: #2A4A7F; color: #FFFFFF; }
#sidebar QPushButton[active="true"] { background-color: #2B6CB0; color: #FFFFFF; border-left: 3px solid #63B3ED; }
#sidebarBottom { background-color: #112A45; padding: 8px; }
#userLabel { color: #90CDF4; font-size: 12px; }

/* ── Tables ──────────────────────────────────────────────────── */
QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; gridline-color: #EDF2F7; font-size: 13px; color: #2D3748; alternate-background-color: #F7FAFC; }
QTableWidget::item { padding: 8px 12px; }
QTableWidget::item:selected { background-color: #EBF8FF; color: #2B6CB0; }
QHeaderView::section { background-color: #F7FAFC; color: #4A5568; font-weight: 600; font-size: 12px; padding: 10px 12px; border: none; border-bottom: 2px solid #E2E8F0; }

/* ── Buttons ─────────────────────────────────────────────────── */
QPushButton { background-color: #3182CE; color: white; border: none; border-radius: 6px; padding: 8px 18px; font-size: 13px; font-weight: 600; }
QPushButton:hover { background-color: #2B6CB0; }
QPushButton:pressed { background-color: #2C5282; }
QPushButton:disabled { background-color: #CBD5E0; color: #A0AEC0; }
QPushButton#dangerBtn { background-color: #E53E3E; }
QPushButton#dangerBtn:hover { background-color: #C53030; }
QPushButton#secondaryBtn { background-color: #EDF2F7; color: #4A5568; }
QPushButton#secondaryBtn:hover { background-color: #E2E8F0; }
QPushButton#successBtn { background-color: #38A169; color: white; }
QPushButton#warningBtn { background-color: #D69E2E; color: white; }

/* ── Inputs ──────────────────────────────────────────────────── */
QLineEdit, QComboBox, QDateEdit, QTimeEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
    background-color: #FFFFFF; border: 1.5px solid #CBD5E0; border-radius: 6px;
    padding: 8px 12px; font-size: 13px; color: #2D3748; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTextEdit:focus { border-color: #3182CE; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView { background: #FFFFFF; border: 1px solid #CBD5E0; selection-background-color: #EBF8FF; color: #2D3748; }

/* ── GroupBox ────────────────────────────────────────────────── */
QGroupBox { font-size: 13px; font-weight: 600; color: #2D3748; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding: 12px; background-color: #FFFFFF; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 8px; background-color: #FFFFFF; }

/* ── Labels ──────────────────────────────────────────────────── */
QLabel { color: #2D3748; }

/* ── Tabs ────────────────────────────────────────────────────── */
QTabWidget::pane { border: 1px solid #E2E8F0; border-radius: 0 8px 8px 8px; background-color: #FFFFFF; }
QTabBar::tab { background-color: #EDF2F7; color: #718096; padding: 8px 20px; margin-right: 2px; border-radius: 6px 6px 0 0; font-size: 13px; }
QTabBar::tab:selected { background-color: #3182CE; color: white; }

/* ── ScrollBars ──────────────────────────────────────────────── */
QScrollBar:vertical { width: 8px; background: #F7FAFC; }
QScrollBar::handle:vertical { background: #CBD5E0; border-radius: 4px; min-height: 20px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { height: 8px; background: #F7FAFC; }
QScrollBar::handle:horizontal { background: #CBD5E0; border-radius: 4px; }

/* ── StatusBar ───────────────────────────────────────────────── */
QStatusBar { background-color: #2D3748; color: #CBD5E0; font-size: 12px; }
QStatusBar::item { border: none; }

/* ── MenuBar ─────────────────────────────────────────────────── */
QMenuBar { background-color: #FFFFFF; color: #2D3748; border-bottom: 1px solid #E2E8F0; }
QMenuBar::item:selected { background-color: #EBF8FF; color: #2B6CB0; }
QMenu { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; }
QMenu::item { padding: 8px 24px; color: #2D3748; }
QMenu::item:selected { background-color: #EBF8FF; color: #2B6CB0; }
QMenu::separator { height: 1px; background: #E2E8F0; margin: 4px 0; }

/* ── Tooltip ─────────────────────────────────────────────────── */
QToolTip { background-color: #2D3748; color: #FFFFFF; border: none; padding: 6px 10px; border-radius: 4px; font-size: 12px; }

/* ── Splitter ────────────────────────────────────────────────── */
QSplitter::handle { background-color: #E2E8F0; }

/* ── Dashboard banner ────────────────────────────────────────── */
#dashBanner { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2B6CB0,stop:1 #3182CE); border-radius: 12px; }
)";
}

QString ThemeManager::darkStylesheet() {
    return R"(
/* ── Global ─────────────────────────────────────────────────── */
QMainWindow, QWidget { background-color: #0F1117; color: #E2E8F0; font-family: 'Segoe UI', Arial, sans-serif; }
QDialog { background-color: #1A1D27; }

/* ── Top bar ─────────────────────────────────────────────────── */
#topBar { background-color: #1A1D27; border-bottom: 1px solid #2D3748; }
#pageTitle { font-size: 20px; font-weight: 700; color: #E2E8F0; }
#clockLabel { font-size: 13px; color: #718096; }

/* ── Sidebar ─────────────────────────────────────────────────── */
#sidebar { background-color: #0D1117; }
#sidebarLogo { background-color: #161B22; }
#logoLabel { color: #58A6FF; font-size: 15px; font-weight: 700; }
#versionLabel { color: #8B949E; font-size: 10px; }
#menuSectionLabel { color: #8B949E; font-size: 10px; font-weight: 700; letter-spacing: 1.5px; padding-top: 8px; }
#sidebar QPushButton { background-color: transparent; color: #8B949E; text-align: left; padding: 10px 16px; border-radius: 0; font-size: 13px; font-weight: 500; border: none; }
#sidebar QPushButton:hover { background-color: #161B22; color: #E2E8F0; }
#sidebar QPushButton[active="true"] { background-color: #1F6FEB22; color: #58A6FF; border-left: 3px solid #58A6FF; }
#sidebarBottom { background-color: #0D1117; padding: 8px; }
#userLabel { color: #8B949E; font-size: 12px; }

/* ── Tables ──────────────────────────────────────────────────── */
QTableWidget { background-color: #1A1D27; border: 1px solid #2D3748; border-radius: 8px; gridline-color: #2D3748; font-size: 13px; color: #E2E8F0; alternate-background-color: #161B22; }
QTableWidget::item { padding: 8px 12px; }
QTableWidget::item:selected { background-color: #1F3A5F; color: #58A6FF; }
QHeaderView::section { background-color: #161B22; color: #8B949E; font-weight: 600; font-size: 12px; padding: 10px 12px; border: none; border-bottom: 2px solid #2D3748; }

/* ── Buttons ─────────────────────────────────────────────────── */
QPushButton { background-color: #1F6FEB; color: white; border: none; border-radius: 6px; padding: 8px 18px; font-size: 13px; font-weight: 600; }
QPushButton:hover { background-color: #388BFD; }
QPushButton:pressed { background-color: #1A5CC7; }
QPushButton:disabled { background-color: #2D3748; color: #4A5568; }
QPushButton#dangerBtn { background-color: #C53030; }
QPushButton#dangerBtn:hover { background-color: #E53E3E; }
QPushButton#secondaryBtn { background-color: #2D3748; color: #E2E8F0; }
QPushButton#secondaryBtn:hover { background-color: #3D4A5C; }
QPushButton#successBtn { background-color: #276749; color: white; }
QPushButton#warningBtn { background-color: #975A16; color: white; }

/* ── Inputs ──────────────────────────────────────────────────── */
QLineEdit, QComboBox, QDateEdit, QTimeEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
    background-color: #161B22; border: 1.5px solid #2D3748; border-radius: 6px;
    padding: 8px 12px; font-size: 13px; color: #E2E8F0; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTextEdit:focus { border-color: #58A6FF; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView { background: #1A1D27; border: 1px solid #2D3748; selection-background-color: #1F3A5F; color: #E2E8F0; }

/* ── GroupBox ────────────────────────────────────────────────── */
QGroupBox { font-size: 13px; font-weight: 600; color: #E2E8F0; border: 1px solid #2D3748; border-radius: 8px; margin-top: 12px; padding: 12px; background-color: #1A1D27; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 8px; background-color: #1A1D27; }

/* ── Labels ──────────────────────────────────────────────────── */
QLabel { color: #E2E8F0; }

/* ── Tabs ────────────────────────────────────────────────────── */
QTabWidget::pane { border: 1px solid #2D3748; border-radius: 0 8px 8px 8px; background-color: #1A1D27; }
QTabBar::tab { background-color: #161B22; color: #8B949E; padding: 8px 20px; margin-right: 2px; border-radius: 6px 6px 0 0; font-size: 13px; }
QTabBar::tab:selected { background-color: #1F6FEB; color: white; }

/* ── ScrollBars ──────────────────────────────────────────────── */
QScrollBar:vertical { width: 8px; background: #0D1117; }
QScrollBar::handle:vertical { background: #2D3748; border-radius: 4px; min-height: 20px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { height: 8px; background: #0D1117; }
QScrollBar::handle:horizontal { background: #2D3748; border-radius: 4px; }

/* ── StatusBar ───────────────────────────────────────────────── */
QStatusBar { background-color: #0D1117; color: #8B949E; font-size: 12px; }
QStatusBar::item { border: none; }

/* ── MenuBar ─────────────────────────────────────────────────── */
QMenuBar { background-color: #1A1D27; color: #E2E8F0; border-bottom: 1px solid #2D3748; }
QMenuBar::item:selected { background-color: #1F3A5F; color: #58A6FF; }
QMenu { background-color: #1A1D27; border: 1px solid #2D3748; border-radius: 6px; }
QMenu::item { padding: 8px 24px; color: #E2E8F0; }
QMenu::item:selected { background-color: #1F3A5F; color: #58A6FF; }
QMenu::separator { height: 1px; background: #2D3748; margin: 4px 0; }

/* ── Tooltip ─────────────────────────────────────────────────── */
QToolTip { background-color: #2D3748; color: #E2E8F0; border: 1px solid #4A5568; padding: 6px 10px; border-radius: 4px; font-size: 12px; }

/* ── Splitter ────────────────────────────────────────────────── */
QSplitter::handle { background-color: #2D3748; }

/* ── Dashboard banner ────────────────────────────────────────── */
#dashBanner { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1A365D,stop:1 #1F6FEB); border-radius: 12px; }

/* ── Calendar ────────────────────────────────────────────────── */
QCalendarWidget { background-color: #1A1D27; color: #E2E8F0; }
QCalendarWidget QToolButton { background-color: #2D3748; color: #E2E8F0; border-radius: 4px; }
QCalendarWidget QAbstractItemView { background-color: #1A1D27; color: #E2E8F0; selection-background-color: #1F6FEB; }
)";
}

} // namespace HMS
