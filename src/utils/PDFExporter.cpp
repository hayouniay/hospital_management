#include "PDFExporter.h"

#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QColor>
#include <QDate>
#include <QTextOption>

namespace HMS {

// ── Theme colours ─────────────────────────────────────────────────────────────
const QColor PDFExporter::Theme::PRIMARY   = QColor("#2B6CB0");
const QColor PDFExporter::Theme::SECONDARY = QColor("#EBF8FF");
const QColor PDFExporter::Theme::ACCENT    = QColor("#63B3ED");
const QColor PDFExporter::Theme::DARK      = QColor("#1A202C");
const QColor PDFExporter::Theme::GRAY      = QColor("#718096");
const QColor PDFExporter::Theme::LIGHT     = QColor("#F7FAFC");
const QColor PDFExporter::Theme::SUCCESS   = QColor("#38A169");
const QColor PDFExporter::Theme::DANGER    = QColor("#E53E3E");
const QColor PDFExporter::Theme::WARNING   = QColor("#D69E2E");
const QColor PDFExporter::Theme::WHITE     = QColor("#FFFFFF");

// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────────────────────────────────────────

void PDFExporter::drawHeader(QPainter& p, int pageW, const QString& title, const QString& subtitle) {
    const int M = Theme::PAGE_MARGIN;
    const int H = Theme::HEADER_H;

    // Blue banner
    p.setBrush(Theme::PRIMARY);
    p.setPen(Qt::NoPen);
    p.drawRect(0, 0, pageW, H);

    // Hospital name (left)
    p.setPen(Theme::WHITE);
    QFont f("Arial", 18, QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(M, 12, pageW / 2, 30), Qt::AlignLeft | Qt::AlignVCenter, "🏥 MediCare HMS");

    // Date (right)
    QFont df("Arial", 9);
    p.setFont(df);
    p.drawText(QRect(pageW / 2, 14, pageW / 2 - M, 18),
               Qt::AlignRight | Qt::AlignVCenter,
               "Generated: " + QDate::currentDate().toString("MMMM d, yyyy"));

    // Dividing accent line
    p.setBrush(Theme::ACCENT);
    p.drawRect(M, H - 28, pageW - 2 * M, 2);

    // Document title
    QFont tf("Arial", 14, QFont::Bold);
    p.setFont(tf);
    p.setPen(Theme::WHITE);
    p.drawText(QRect(M, H - 26, pageW - 2 * M, 22),
               Qt::AlignLeft | Qt::AlignVCenter, title);

    // Subtitle
    if (!subtitle.isEmpty()) {
        QFont sf("Arial", 9);
        p.setFont(sf);
        p.setPen(Theme::SECONDARY);
        p.drawText(QRect(pageW / 2, H - 26, pageW / 2 - M, 22),
                   Qt::AlignRight | Qt::AlignVCenter, subtitle);
    }
}

void PDFExporter::drawFooter(QPainter& p, int pageW, int pageH, int pageNum, int totalPages) {
    const int M = Theme::PAGE_MARGIN;
    const int y = pageH - Theme::FOOTER_H;

    p.setPen(Theme::ACCENT);
    p.drawLine(M, y, pageW - M, y);

    QFont f("Arial", 8);
    p.setFont(f);
    p.setPen(Theme::GRAY);
    p.drawText(QRect(M, y + 6, 200, 20), Qt::AlignLeft,
               "MediCare Hospital Management System — Confidential");
    p.drawText(QRect(pageW - M - 100, y + 6, 100, 20), Qt::AlignRight,
               QString("Page %1 of %2").arg(pageNum).arg(totalPages));
}

int PDFExporter::drawSectionTitle(QPainter& p, int x, int y, int w, const QString& title) {
    // Coloured left bar + title background
    p.setBrush(Theme::SECONDARY);
    p.setPen(Qt::NoPen);
    p.drawRect(x, y, w, 24);
    p.setBrush(Theme::PRIMARY);
    p.drawRect(x, y, 4, 24);

    QFont f("Arial", 10, QFont::Bold);
    p.setFont(f);
    p.setPen(Theme::PRIMARY);
    p.drawText(QRect(x + 10, y, w - 10, 24),
               Qt::AlignLeft | Qt::AlignVCenter, title);
    return y + 24 + 8;
}

int PDFExporter::drawField(QPainter& p, int x, int y, int w,
                            const QString& label, const QString& value, bool shade) {
    const int LW = 150; // label column width
    const int VW = w - LW;
    const int H  = 20;

    if (shade) {
        p.setBrush(Theme::LIGHT);
        p.setPen(Qt::NoPen);
        p.drawRect(x, y, w, H);
    }

    QFont lf("Arial", 8, QFont::Bold);
    p.setFont(lf);
    p.setPen(Theme::GRAY);
    p.drawText(QRect(x + 6, y, LW - 6, H),
               Qt::AlignLeft | Qt::AlignVCenter, label + ":");

    QFont vf("Arial", 8);
    p.setFont(vf);
    p.setPen(Theme::DARK);
    p.drawText(QRect(x + LW, y, VW - 6, H),
               Qt::AlignLeft | Qt::AlignVCenter, value.isEmpty() ? "—" : value);

    return y + H;
}

int PDFExporter::drawDivider(QPainter& p, int x, int y, int w) {
    p.setPen(QPen(Theme::ACCENT, 0.5));
    p.drawLine(x, y + 4, x + w, y + 4);
    return y + 12;
}

int PDFExporter::drawTableHeader(QPainter& p, int x, int y, int /*w*/,
                                  const QStringList& cols, const QList<int>& colWidths) {
    int cx = x;
    p.setBrush(Theme::PRIMARY);
    int totalW = 0; for (int cw : colWidths) totalW += cw;
    p.setPen(Qt::NoPen);
    p.drawRect(x, y, totalW, 22);

    QFont f("Arial", 8, QFont::Bold);
    p.setFont(f);
    p.setPen(Theme::WHITE);
    for (int i = 0; i < cols.size(); ++i) {
        p.drawText(QRect(cx + 4, y, colWidths[i] - 4, 22),
                   Qt::AlignLeft | Qt::AlignVCenter, cols[i]);
        cx += colWidths[i];
    }
    return y + 22;
}

int PDFExporter::drawTableRow(QPainter& p, int x, int y, int /*w*/,
                               const QStringList& vals, const QList<int>& colWidths, bool shade) {
    int cx = x;
    int totalW = 0; for (int cw : colWidths) totalW += cw;
    if (shade) {
        p.setBrush(Theme::LIGHT);
        p.setPen(Qt::NoPen);
        p.drawRect(x, y, totalW, 20);
    }
    // Bottom border
    p.setPen(QPen(QColor("#E2E8F0"), 0.5));
    p.drawLine(x, y + 20, x + totalW, y + 20);

    QFont f("Arial", 8);
    p.setFont(f);
    p.setPen(Theme::DARK);
    for (int i = 0; i < vals.size() && i < colWidths.size(); ++i) {
        p.drawText(QRect(cx + 4, y, colWidths[i] - 8, 20),
                   Qt::AlignLeft | Qt::AlignVCenter, vals[i]);
        cx += colWidths[i];
    }
    return y + 20;
}

// ─────────────────────────────────────────────────────────────────────────────
//  CONSULTATION REPORT
// ─────────────────────────────────────────────────────────────────────────────

std::expected<QString, QString> PDFExporter::exportConsultationReport(
    const Patient& patient,
    const std::vector<MedicalRecord>& records,
    const QString& outputPath,
    QWidget* parent)
{
    Q_UNUSED(parent)

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPainter p;
    if (!p.begin(&printer)) {
        return std::unexpected("Failed to open PDF writer for: " + outputPath);
    }

    const int pageW  = p.device()->width();
    const int pageH  = p.device()->height();
    const int M      = Theme::PAGE_MARGIN;
    const int contentW = pageW - 2 * M;

    // Estimate total pages (1 header page + ~1 per 2 records)
    int totalPages = 1 + static_cast<int>(records.size() + 1) / 2;
    int pageNum    = 1;

    // ── PAGE 1: Patient Profile ───────────────────────────────────────────────
    drawHeader(p, pageW,
               "Patient Consultation Report",
               "ID: " + patient.patientUid);
    drawFooter(p, pageW, pageH, pageNum, totalPages);

    int y = Theme::HEADER_H + 20;

    // Patient profile card (shaded box)
    p.setBrush(Theme::SECONDARY);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(M, y, contentW, 140, 6, 6);
    p.setBrush(Theme::PRIMARY);
    p.drawRoundedRect(M, y, 6, 140, 3, 3);

    // Patient avatar circle
    p.setBrush(Theme::PRIMARY);
    p.setPen(Qt::NoPen);
    p.drawEllipse(M + 18, y + 14, 58, 58);
    QFont af("Arial", 20, QFont::Bold);
    p.setFont(af);
    p.setPen(Theme::WHITE);
    QString initials = (patient.firstName.isEmpty() ? QStringLiteral("?") : patient.firstName.left(1))
                     + (patient.lastName.isEmpty()  ? QStringLiteral("?") : patient.lastName.left(1));
    p.drawText(QRect(M + 18, y + 14, 58, 58), Qt::AlignCenter, initials.toUpper());

    // Patient name & ID
    QFont nf("Arial", 16, QFont::Bold);
    p.setFont(nf);
    p.setPen(Theme::PRIMARY);
    p.drawText(M + 90, y + 36, patient.fullName());

    QFont idf("Arial", 9);
    p.setFont(idf);
    p.setPen(Theme::GRAY);
    p.drawText(M + 90, y + 56, "Patient ID: " + patient.patientUid);

    // Status badge
    QColor statusColor = (patient.status == "active") ? Theme::SUCCESS : Theme::DANGER;
    p.setBrush(statusColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(M + 90, y + 66, 70, 16, 8, 8);
    QFont sf("Arial", 7, QFont::Bold);
    p.setFont(sf);
    p.setPen(Theme::WHITE);
    p.drawText(QRect(M + 90, y + 66, 70, 16), Qt::AlignCenter, patient.status.toUpper());

    // Patient details grid
    int dx = M + 20;
    int dy = y + 90;
    int hw = contentW / 2 - 10;

    auto drawPair = [&](const QString& l, const QString& v, int ox, int oy) {
        QFont lf2("Arial", 7, QFont::Bold); p.setFont(lf2); p.setPen(Theme::GRAY);
        p.drawText(ox, oy, l + ":");
        QFont vf2("Arial", 8); p.setFont(vf2); p.setPen(Theme::DARK);
        p.drawText(ox + 90, oy, v.isEmpty() ? "—" : v);
    };

    drawPair("Date of Birth", patient.dateOfBirth.toString("MMMM d, yyyy") +
             QString(" (Age %1)").arg(patient.age()), dx, dy);
    drawPair("Gender",        patient.gender,  dx + hw + 20, dy);
    dy += 16;
    drawPair("Blood Type",    patient.bloodType, dx, dy);
    drawPair("Phone",         patient.phone,   dx + hw + 20, dy);
    dy += 16;
    drawPair("Email",         patient.email,   dx, dy);
    drawPair("Insurance ID",  patient.insuranceId, dx + hw + 20, dy);

    y += 160;

    // ── Emergency contact ─────────────────────────────────────────────────────
    if (!patient.emergencyContact.isEmpty()) {
        y = drawSectionTitle(p, M, y, contentW, "Emergency Contact");
        y = drawField(p, M, y, contentW, "Contact Name",  patient.emergencyContact, false);
        y = drawField(p, M, y, contentW, "Contact Phone", patient.emergencyPhone,   true);
        y += 10;
    }

    // ── Summary stats bar ─────────────────────────────────────────────────────
    y = drawSectionTitle(p, M, y, contentW, "Consultation Summary");

    int totalRec = static_cast<int>(records.size());
    int withFollowUp = 0;
    for (const auto& r : records) if (r.followUpDate.isValid()) ++withFollowUp;
    int withPrescription = 0;
    for (const auto& r : records) if (!r.prescription.isEmpty()) ++withPrescription;

    // 3 stat boxes
    int bw = (contentW - 20) / 3;
    auto drawStatBox = [&](int bx, int by, const QString& val, const QString& lbl, const QColor& col) {
        p.setBrush(col.lighter(170));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(bx, by, bw, 50, 6, 6);
        p.setBrush(col);
        p.drawRoundedRect(bx, by, bw, 5, 2, 2);
        QFont vf3("Arial", 18, QFont::Bold); p.setFont(vf3); p.setPen(col);
        p.drawText(QRect(bx, by + 6, bw, 26), Qt::AlignCenter, val);
        QFont lf3("Arial", 7); p.setFont(lf3); p.setPen(Theme::GRAY);
        p.drawText(QRect(bx, by + 32, bw, 14), Qt::AlignCenter, lbl);
    };
    drawStatBox(M,              y, QString::number(totalRec),        "Total Consultations", Theme::PRIMARY);
    drawStatBox(M + bw + 10,    y, QString::number(withPrescription),"With Prescription",   Theme::SUCCESS);
    drawStatBox(M + 2*(bw+10),  y, QString::number(withFollowUp),   "With Follow-up",      Theme::WARNING);
    y += 65;

    // ── Medical Records ───────────────────────────────────────────────────────
    y = drawSectionTitle(p, M, y, contentW, "Consultation History");

    if (records.empty()) {
        QFont ef("Arial", 9);
        p.setFont(ef);
        p.setPen(Theme::GRAY);
        p.drawText(QRect(M, y, contentW, 30), Qt::AlignCenter, "No consultation records found.");
        y += 30;
    }

    for (int ri = 0; ri < static_cast<int>(records.size()); ++ri) {
        const auto& rec = records[ri];

        // Check space — start new page if needed (leave 200px buffer for a block)
        if (y + 200 > pageH - Theme::FOOTER_H) {
            printer.newPage();
            ++pageNum;
            drawHeader(p, pageW, "Patient Consultation Report — Continued",
                       "ID: " + patient.patientUid);
            drawFooter(p, pageW, pageH, pageNum, totalPages);
            y = Theme::HEADER_H + 20;
        }

        // Record header bar
        p.setBrush(Theme::PRIMARY.lighter(180));
        p.setPen(Qt::NoPen);
        p.drawRect(M, y, contentW, 22);
        p.setBrush(Theme::PRIMARY);
        p.drawRect(M, y, 4, 22);

        QFont rhf("Arial", 9, QFont::Bold);
        p.setFont(rhf);
        p.setPen(Theme::PRIMARY);
        p.drawText(QRect(M + 10, y, 300, 22), Qt::AlignLeft | Qt::AlignVCenter,
                   QString("Consultation #%1  —  %2")
                       .arg(ri + 1)
                       .arg(rec.createdAt.toString("MMMM d, yyyy")));

        QFont rdf("Arial", 8);
        p.setFont(rdf);
        p.setPen(Theme::GRAY);
        p.drawText(QRect(M, y, contentW - 6, 22), Qt::AlignRight | Qt::AlignVCenter,
                   "Dr. " + rec.doctorName);
        y += 22;

        bool shade = false;
        auto drawMultiField = [&](const QString& label, const QString& value) -> int {
            if (value.trimmed().isEmpty()) return y;
            // Measure text height
            QFont f8("Arial", 8);
            p.setFont(f8);
            QFontMetrics fm(f8);
            int textW = contentW - 170;
            QRect br = fm.boundingRect(QRect(0,0,textW,1000),
                                       Qt::TextWordWrap, value.trimmed());
            int textH = qMax(20, br.height() + 6);

            if (shade) {
                p.setBrush(Theme::LIGHT);
                p.setPen(Qt::NoPen);
                p.drawRect(M, y, contentW, textH);
            }
            QFont lbf("Arial", 8, QFont::Bold); p.setFont(lbf);
            p.setPen(Theme::GRAY);
            p.drawText(QRect(M + 6, y, 155, textH),
                       Qt::AlignLeft | Qt::AlignTop, label + ":");
            p.setFont(f8);
            p.setPen(Theme::DARK);
            QTextOption opt;
            opt.setWrapMode(QTextOption::WordWrap);
            p.drawText(QRectF(M + 162, y + 2, textW, textH - 2),
                       value.trimmed(), opt);
            shade = !shade;
            y += textH;
            return y;
        };

        drawMultiField("Diagnosis",       rec.diagnosis);
        drawMultiField("Symptoms",        rec.symptoms);
        drawMultiField("Prescription",    rec.prescription);
        drawMultiField("Lab Results",     rec.labResults);
        drawMultiField("Notes",           rec.notes);

        if (rec.followUpDate.isValid()) {
            // Follow-up badge
            p.setBrush(Theme::WARNING.lighter(170));
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(M, y, contentW, 22, 4, 4);
            QFont fuf("Arial", 8, QFont::Bold);
            p.setFont(fuf);
            p.setPen(Theme::WARNING.darker(130));
            p.drawText(QRect(M + 10, y, contentW - 20, 22),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       "📅  Follow-up scheduled: " + rec.followUpDate.toString("MMMM d, yyyy"));
            y += 22;
        }

        y += 12; // gap between records
        if (ri < static_cast<int>(records.size()) - 1) {
            p.setPen(QPen(Theme::ACCENT, 0.5, Qt::DashLine));
            p.drawLine(M + 20, y, M + contentW - 20, y);
            y += 10;
        }
    }

    p.end();
    return outputPath;
}

// ─────────────────────────────────────────────────────────────────────────────
//  INVOICE / BILL PDF
// ─────────────────────────────────────────────────────────────────────────────

std::expected<QString, QString> PDFExporter::exportBillPDF(
    const Billing& bill,
    const Patient& patient,
    const QString& outputPath,
    QWidget* parent)
{
    Q_UNUSED(parent)

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPainter p;
    if (!p.begin(&printer)) {
        return std::unexpected("Failed to open PDF writer for: " + outputPath);
    }

    const int pageW    = p.device()->width();
    const int pageH    = p.device()->height();
    const int M        = Theme::PAGE_MARGIN;
    const int contentW = pageW - 2 * M;

    // ── Header ────────────────────────────────────────────────────────────────
    drawHeader(p, pageW, "Invoice / Bill", bill.invoiceNumber);
    drawFooter(p, pageW, pageH, 1, 1);

    int y = Theme::HEADER_H + 24;

    // ── Invoice meta + patient side by side ───────────────────────────────────
    int halfW = contentW / 2 - 10;

    // Left: Invoice details
    p.setBrush(Theme::SECONDARY);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(M, y, halfW, 100, 6, 6);
    y = drawSectionTitle(p, M, y, halfW, "Invoice Details");
    drawField(p, M, y,       halfW, "Invoice No.",    bill.invoiceNumber, false);
    drawField(p, M, y + 20,  halfW, "Issue Date",     QDate::currentDate().toString("yyyy-MM-dd"), true);
    drawField(p, M, y + 40,  halfW, "Due Date",       bill.dueDate.isValid()
                                                        ? bill.dueDate.toString("yyyy-MM-dd") : "—", false);
    drawField(p, M, y + 60,  halfW, "Payment Method", bill.paymentMethod, true);

    // Right: Patient details
    p.setBrush(Theme::SECONDARY);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(M + halfW + 20, y - 24, halfW, 100, 6, 6);
    int rx = M + halfW + 20;
    drawSectionTitle(p, rx, y - 24, halfW, "Billed To");
    drawField(p, rx, y,       halfW, "Patient Name",  patient.fullName(),   false);
    drawField(p, rx, y + 20,  halfW, "Patient ID",    patient.patientUid,   true);
    drawField(p, rx, y + 40,  halfW, "Phone",         patient.phone,        false);
    drawField(p, rx, y + 60,  halfW, "Email",         patient.email,        true);

    y += 90;

    // ── Billing breakdown table ───────────────────────────────────────────────
    y = drawSectionTitle(p, M, y, contentW, "Billing Breakdown");

    QStringList   cols  = {"Description",      "Amount"};
    QList<int>    cw    = {contentW - 120,       120};
    y = drawTableHeader(p, M, y, contentW, cols, cw);

    y = drawTableRow(p, M, y, contentW, {"Medical Services", QString("$%1").arg(bill.amount,0,'f',2)}, cw, false);
    if (bill.discount > 0)
        y = drawTableRow(p, M, y, contentW, {"Discount Applied", QString("-$%1").arg(bill.discount,0,'f',2)}, cw, true);
    if (bill.tax > 0)
        y = drawTableRow(p, M, y, contentW, {"Tax / VAT",   QString("$%1").arg(bill.tax,0,'f',2)}, cw, false);
    if (!bill.notes.isEmpty())
        y = drawTableRow(p, M, y, contentW, {bill.notes,    ""}, cw, true);

    y += 8;

    // ── Totals box ────────────────────────────────────────────────────────────
    int boxW = 280;
    int boxX = M + contentW - boxW;

    auto drawTotalRow = [&](const QString& label, const QString& value,
                             bool bold, const QColor& textCol = PDFExporter::Theme::DARK) {
        QFont f("Arial", bold ? 10 : 9, bold ? QFont::Bold : QFont::Normal);
        p.setFont(f);
        p.setPen(textCol);
        p.drawText(QRect(boxX + 8, y, boxW / 2, 22),
                   Qt::AlignLeft | Qt::AlignVCenter, label);
        p.drawText(QRect(boxX + boxW / 2, y, boxW / 2 - 8, 22),
                   Qt::AlignRight | Qt::AlignVCenter, value);
        y += 22;
    };

    // Subtotal divider
    p.setPen(QPen(Theme::ACCENT, 0.5));
    p.drawLine(boxX, y, boxX + boxW, y);
    y += 4;

    drawTotalRow("Subtotal:",  QString("$%1").arg(bill.amount,0,'f',2),      false);
    if (bill.discount > 0)
        drawTotalRow("Discount:", QString("-$%1").arg(bill.discount,0,'f',2), false, Theme::SUCCESS);
    if (bill.tax > 0)
        drawTotalRow("Tax:",      QString("$%1").arg(bill.tax,0,'f',2),       false);

    // Total row — full coloured background
    p.setBrush(Theme::PRIMARY);
    p.setPen(Qt::NoPen);
    p.drawRect(boxX, y, boxW, 28);
    QFont tf("Arial", 12, QFont::Bold);
    p.setFont(tf);
    p.setPen(Theme::WHITE);
    p.drawText(QRect(boxX + 8, y, boxW / 2, 28),  Qt::AlignLeft  | Qt::AlignVCenter, "TOTAL:");
    p.drawText(QRect(boxX + boxW/2, y, boxW/2-8, 28), Qt::AlignRight | Qt::AlignVCenter,
               QString("$%1").arg(bill.total,0,'f',2));
    y += 32;

    drawTotalRow("Amount Paid:", QString("$%1").arg(bill.paidAmount,0,'f',2), false, Theme::SUCCESS);

    double balance = bill.balance();
    QColor balColor = balance > 0 ? Theme::DANGER : Theme::SUCCESS;
    // Balance highlight box
    p.setBrush(balColor.lighter(180));
    p.setPen(Qt::NoPen);
    p.drawRect(boxX, y, boxW, 24);
    QFont bf("Arial", 10, QFont::Bold); p.setFont(bf); p.setPen(balColor);
    p.drawText(QRect(boxX + 8, y, boxW/2, 24),  Qt::AlignLeft  | Qt::AlignVCenter,
               balance > 0 ? "Balance Due:" : "Overpaid:");
    p.drawText(QRect(boxX+boxW/2, y, boxW/2-8, 24), Qt::AlignRight | Qt::AlignVCenter,
               QString("$%1").arg(qAbs(balance),0,'f',2));
    y += 36;

    // ── Payment Status badge ──────────────────────────────────────────────────
    QColor statusBg = (bill.status=="paid")    ? Theme::SUCCESS
                    : (bill.status=="partial")  ? Theme::WARNING
                    : (bill.status=="pending")  ? Theme::DANGER
                    : Theme::GRAY;

    p.setBrush(statusBg);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(M, y, 140, 30, 6, 6);
    QFont sbf("Arial", 11, QFont::Bold);
    p.setFont(sbf);
    p.setPen(Theme::WHITE);
    p.drawText(QRect(M, y, 140, 30), Qt::AlignCenter,
               bill.status.toUpper());
    y += 44;

    // ── Notes ─────────────────────────────────────────────────────────────────
    if (!bill.notes.isEmpty()) {
        y = drawSectionTitle(p, M, y, contentW, "Notes");
        p.setBrush(Theme::LIGHT);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(M, y, contentW, 40, 4, 4);
        QFont nf("Arial", 8);
        p.setFont(nf);
        p.setPen(Theme::DARK);
        p.drawText(QRect(M + 8, y + 4, contentW - 16, 32),
                   Qt::AlignLeft | Qt::TextWordWrap, bill.notes);
        y += 48;
    }

    // ── Thank you footer ──────────────────────────────────────────────────────
    p.setBrush(Theme::PRIMARY);
    p.setPen(Qt::NoPen);
    p.drawRect(M, pageH - Theme::FOOTER_H - 40, contentW, 30);
    QFont tyf("Arial", 9);
    p.setFont(tyf);
    p.setPen(Theme::WHITE);
    p.drawText(QRect(M, pageH - Theme::FOOTER_H - 40, contentW, 30),
               Qt::AlignCenter,
               "Thank you for choosing MediCare Hospital. Get well soon!");

    p.end();
    return outputPath;
}

// ─────────────────────────────────────────────────────────────────────────────
//  DIALOG WRAPPERS
// ─────────────────────────────────────────────────────────────────────────────

std::expected<QString, QString> PDFExporter::exportConsultationReportDialog(
    const Patient& patient,
    const std::vector<MedicalRecord>& records,
    QWidget* parent)
{
    QString defaultName = QString("Consultation_%1_%2.pdf")
        .arg(patient.patientUid)
        .arg(QDate::currentDate().toString("yyyyMMdd"));

    QString path = QFileDialog::getSaveFileName(
        parent, "Save Consultation Report", defaultName,
        "PDF Files (*.pdf)");

    if (path.isEmpty()) return std::unexpected("Cancelled");
    if (!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";

    auto result = exportConsultationReport(patient, records, path, parent);
    if (result) QDesktopServices::openUrl(QUrl::fromLocalFile(*result));
    return result;
}

std::expected<QString, QString> PDFExporter::exportBillPDFDialog(
    const Billing& bill,
    const Patient& patient,
    QWidget* parent)
{
    QString defaultName = QString("Invoice_%1_%2.pdf")
        .arg(bill.invoiceNumber)
        .arg(QDate::currentDate().toString("yyyyMMdd"));

    QString path = QFileDialog::getSaveFileName(
        parent, "Save Invoice PDF", defaultName,
        "PDF Files (*.pdf)");

    if (path.isEmpty()) return std::unexpected("Cancelled");
    if (!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";

    auto result = exportBillPDF(bill, patient, path, parent);
    if (result) QDesktopServices::openUrl(QUrl::fromLocalFile(*result));
    return result;
}

} // namespace HMS
