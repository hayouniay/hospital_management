#pragma once

#include "models/Patient.h"
#include "models/MedicalRecord.h"
#include "models/Billing.h"
#include "models/Doctor.h"

#include <QString>
#include <QWidget>
#include <QPainter>
#include <QPrinter>
#include <QColor>
#include <QStringList>
#include <expected>
#include <vector>

namespace HMS {

class PDFExporter {
public:

    // Patient Consultation Report
    static std::expected<QString, QString> exportConsultationReport(
        const Patient&                    patient,
        const std::vector<MedicalRecord>& records,
        const QString&                    outputPath,
        QWidget*                          parent = nullptr);

    // Invoice / Bill PDF
    static std::expected<QString, QString> exportBillPDF(
        const Billing& bill,
        const Patient& patient,
        const QString& outputPath,
        QWidget*       parent = nullptr);

    // File-save dialog wrappers (open dialog, then export, then open PDF)
    static std::expected<QString, QString> exportConsultationReportDialog(
        const Patient&                    patient,
        const std::vector<MedicalRecord>& records,
        QWidget*                          parent = nullptr);

    static std::expected<QString, QString> exportBillPDFDialog(
        const Billing& bill,
        const Patient& patient,
        QWidget*       parent = nullptr);

private:
    struct Theme {
        static constexpr int PAGE_MARGIN = 50;
        static constexpr int LINE_H      = 20;
        static constexpr int SECTION_GAP = 14;
        static constexpr int HEADER_H    = 90;
        static constexpr int FOOTER_H    = 40;

        static const QColor PRIMARY;
        static const QColor SECONDARY;
        static const QColor ACCENT;
        static const QColor DARK;
        static const QColor GRAY;
        static const QColor LIGHT;
        static const QColor SUCCESS;
        static const QColor DANGER;
        static const QColor WARNING;
        static const QColor WHITE;
    };

    static void drawHeader(QPainter& p, int pageW,
                           const QString& title, const QString& subtitle);
    static void drawFooter(QPainter& p, int pageW, int pageH,
                           int pageNum, int totalPages);
    static int  drawSectionTitle(QPainter& p, int x, int y, int w,
                                 const QString& title);
    static int  drawField(QPainter& p, int x, int y, int w,
                          const QString& label, const QString& value,
                          bool shade = false);
    static int  drawDivider(QPainter& p, int x, int y, int w);
    static int  drawTableHeader(QPainter& p, int x, int y, int w,
                                const QStringList& cols,
                                const QList<int>& colWidths);
    static int  drawTableRow(QPainter& p, int x, int y, int w,
                             const QStringList& vals,
                             const QList<int>& colWidths,
                             bool shade = false);
};

} // namespace HMS
