cmake_minimum_required(VERSION 3.22)
project(HospitalMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    PrintSupport
    Charts
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(SOURCES
    src/main.cpp

    # Database
    src/database/DatabaseManager.cpp

    # Models
    src/models/Patient.cpp
    src/models/Doctor.cpp
    src/models/Appointment.cpp
    src/models/Registration.cpp
    src/models/Department.cpp
    src/models/MedicalRecord.cpp
    src/models/Billing.cpp

    # Auth
    src/auth/AuthController.cpp
    src/auth/LoginDialog.cpp

    # Settings
    src/settings/AppSettings.cpp
    src/settings/ThemeManager.cpp

    # Core controllers
    src/core/HospitalController.cpp
    src/core/PatientController.cpp
    src/core/AppointmentController.cpp
    src/core/BillingController.cpp
    src/core/DoctorController.cpp
    src/core/RegistrationController.cpp
    src/core/DepartmentController.cpp
    src/core/MedicalRecordController.cpp

    # Views
    src/views/MainWindow.cpp
    src/views/DashboardView.cpp
    src/views/PatientView.cpp
    src/views/DoctorView.cpp
    src/views/AppointmentView.cpp
    src/views/RegistrationView.cpp
    src/views/BillingView.cpp
    src/views/MedicalRecordView.cpp
    src/views/DepartmentView.cpp
    src/views/ReportsView.cpp
    src/views/PatientTimelineView.cpp
    src/views/AuditLogView.cpp
    src/views/SettingsDialog.cpp

    # Widgets
    src/widgets/PatientCard.cpp
    src/widgets/SearchBar.cpp
    src/widgets/StatusBadge.cpp
    src/widgets/SidebarMenu.cpp
    src/widgets/StatCard.cpp
    src/widgets/DataTable.cpp
    src/widgets/ToastNotification.cpp
    src/widgets/GlobalSearchBar.cpp

    # Utils
    src/utils/QRCodeGenerator.cpp
    src/utils/PDFExporter.cpp
    src/utils/Validator.cpp
    src/utils/DateTimeHelper.cpp
)

set(HEADERS
    # Database
    src/database/DatabaseManager.h

    # Models
    src/models/Patient.h
    src/models/Doctor.h
    src/models/Appointment.h
    src/models/Registration.h
    src/models/Department.h
    src/models/MedicalRecord.h
    src/models/Billing.h

    # Auth
    src/auth/AuthController.h
    src/auth/LoginDialog.h
    src/settings/AppSettings.h
    src/settings/ThemeManager.h

    # Core
    src/core/HospitalController.h
    src/core/PatientController.h
    src/core/AppointmentController.h
    src/core/BillingController.h
    src/core/DoctorController.h
    src/core/RegistrationController.h
    src/core/DepartmentController.h
    src/core/MedicalRecordController.h

    # Views
    src/views/MainWindow.h
    src/views/DashboardView.h
    src/views/PatientView.h
    src/views/DoctorView.h
    src/views/AppointmentView.h
    src/views/RegistrationView.h
    src/views/BillingView.h
    src/views/MedicalRecordView.h
    src/views/DepartmentView.h
    src/views/ReportsView.h
    src/views/PatientTimelineView.h
    src/views/AuditLogView.h
    src/views/SettingsDialog.h

    # Widgets
    src/widgets/PatientCard.h
    src/widgets/SearchBar.h
    src/widgets/StatusBadge.h
    src/widgets/SidebarMenu.h
    src/widgets/StatCard.h
    src/widgets/DataTable.h
    src/widgets/ToastNotification.h
    src/widgets/GlobalSearchBar.h

    # Utils
    src/utils/QRCodeGenerator.h
    src/utils/PDFExporter.h
    src/utils/Validator.h
    src/utils/DateTimeHelper.h
)

qt_add_executable(HospitalMS
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(HospitalMS PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Charts
)

target_include_directories(HospitalMS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(WIN32)
    set_target_properties(HospitalMS PROPERTIES WIN32_EXECUTABLE TRUE)
    add_custom_command(TARGET HospitalMS POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                --no-translations "$<TARGET_FILE:HospitalMS>"
        COMMENT "Running windeployqt..." VERBATIM)
endif()

include(GNUInstallDirs)
install(TARGETS HospitalMS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
