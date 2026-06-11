#include <QApplication>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QStyleFactory>

#include "database/DatabaseManager.h"
#include "views/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Hospital Management System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MediCare");
    app.setStyle(QStyleFactory::create("Fusion"));

    // Splash screen
    QSplashScreen splash;
    splash.setStyleSheet(R"(
        QSplashScreen {
            background-color: #1A365D;
            color: white;
            font-size: 16px;
        }
    )");
    splash.showMessage("🏥  Hospital Management System\nInitializing...",
                       Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    splash.show();
    app.processEvents();

    // Initialize database
    auto& db = HMS::DatabaseManager::instance();
    auto dbResult = db.initialize("hospital.db");

    if (!dbResult) {
        QMessageBox::critical(nullptr, "Database Error",
            "Failed to initialize database:\n" + dbResult.error());
        return 1;
    }

    // Create and show main window
    HMS::MainWindow mainWindow;

    QTimer::singleShot(1200, [&]() {
        splash.finish(&mainWindow);
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
    });

    return app.exec();
}
