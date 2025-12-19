#include "ui/Gembling.hpp"
#include <qcontainerfwd.h>
#include <qcursor.h>
#include <qframe.h>
#include <QVBoxLayout>
#include <QLabel>
#include <qlabel.h>
#include <qlayoutitem.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <QRegularExpression>
#include <qpushbutton.h>
#include <qurl.h>
#include <ui/MainWindow.hpp>
#include <QMessageBox>

namespace Gemspace {
    Gembling::Gembling(MainWindow* parent) : QFrame() {
        mainWindow = parent;
        layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 5, 10, 5);
        layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        setLayout(layout);
    }

    void Gembling::parseSite(const QString& site) {
        this->setUpdatesEnabled(false);
        layout->setEnabled(false);
        layout->setSpacing(0);
        bool isPre = false;
        bool isImage = false;

        currentUrl = site.toStdString();

        QStringList lines = site.split('\n');

        if (lines.first().contains("image")) {
            return;
        } else {
            qDebug() << lines.first();
        }

        for (const QString& line : lines) {

            if (line.contains("text/gemini")) continue;

            if (line.startsWith("```")) {
                isPre = !isPre;

                continue;
            }

            if (isPre) {
                auto code = new QLabel(line, this);
                code->setWordWrap(true);
                code->setStyleSheet(
                    "background-color: #303030; "
                    "font-family: 'Monospace', 'Courier New'; "
                    "color: #aaaaaa; "
                    "padding: 2px; "
                    "border-radius: 0px;"
                );
                QFont font = code->font();
                font.setPointSize(11);
                code->setFont(font);
                objects.push_back(code);
                layout->addWidget(code);
                continue;
            }

            QString trimmedLine = line.trimmed();

            if (trimmedLine.isEmpty()) {
                layout->addSpacing(10);
                continue;
            }

            if (trimmedLine.startsWith("###")) {
                QLabel* obj = new QLabel(this);
                obj->setWordWrap(true);
                obj->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                QFont font = obj->font();
                trimmedLine = trimmedLine.replace("###", "").trimmed();
                font.setBold(true);
                font.setPointSize(13);
                obj->setFont(font);
                obj->setText(trimmedLine);
                objects.push_back(obj);
                layout->addWidget(obj);
            }
            else if (trimmedLine.startsWith("##")) {
                QLabel* obj = new QLabel(this);
                obj->setWordWrap(true);
                obj->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                QFont font = obj->font();
                trimmedLine = trimmedLine.replace("##", "").trimmed();
                font.setBold(true);
                font.setPointSize(15);
                obj->setText(trimmedLine);
                obj->setFont(font);
                objects.push_back(obj);
                layout->addWidget(obj);
            }
            else if (trimmedLine.startsWith("#")) {
                QLabel* obj = new QLabel(this);
                obj->setWordWrap(true);
                QFont font = obj->font();
                trimmedLine = trimmedLine.replace("#", "").trimmed();
                font.setBold(true);
                font.setPointSize(17);
                obj->setText(trimmedLine);
                obj->setFont(font);
                objects.push_back(obj);
                layout->addWidget(obj);
            }
            else if (trimmedLine.startsWith("=>")) {
                QPushButton* obj = new QPushButton(this);
                obj->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

                trimmedLine = trimmedLine.replace("=>", "").trimmed();
                QStringList parts = trimmedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                if (!parts.empty()) {
                    QString rawUrl = parts.first();
                    parts.removeFirst();
                    QString linkText = parts.isEmpty() ? rawUrl : parts.join(" ");

                    obj->setText(linkText);
                    obj->setToolTip(rawUrl);
                    obj->setCursor(Qt::PointingHandCursor);
                    obj->setStyleSheet(
                        "QPushButton { "
                        "  color: #58a6ff; text-align: left; border: none; background: none; "
                        "  font-size: 11pt; text-decoration: underline; padding: 2px 0px; "
                        "}"
                        "QPushButton:hover { color: #1f6feb; }"
                    );

                    connect(obj, &QPushButton::clicked, [this, rawUrl] {
                        QString baseStr = this->mainWindow->searchInput->text();

                        if (baseStr.startsWith("gemini:") && !baseStr.startsWith("gemini://")) {
                            baseStr.replace("gemini:", "gemini://");
                        } else if (!baseStr.contains("://")) {
                            baseStr.prepend("gemini://");
                        }

                        QUrl baseUrl(baseStr);
                        QUrl clickedUrl(rawUrl);

                        QUrl finalUrl = baseUrl.resolved(clickedUrl);

                        QString finalStr = finalUrl.toString();
                        if (finalStr.startsWith("gemini:") && !finalStr.startsWith("gemini://")) {
                            finalStr.replace("gemini:", "gemini://");
                        }

                        if(!finalStr.startsWith("http")) {
                            this->mainWindow->searchInput->setText(finalStr);
                            this->mainWindow->onUrlEntered();
                        } else {
                            QMessageBox::warning(this, "Error", "Gemspace only supports gemini:// URLs");
                        }
                    });

                    layout->addSpacing(2);
                    objects.push_back(obj);
                    layout->addWidget(obj);
                    layout->addSpacing(2);
                }
            }
            else if (trimmedLine.startsWith("*")) {
                trimmedLine = "  •  " + trimmedLine.mid(1).trimmed();
                auto bullet = new QLabel(trimmedLine, this);
                bullet->setStyleSheet("margin-left: 10px;");
                bullet->setWordWrap(true);
                objects.push_back(bullet);
                QFont font = bullet->font();
                font.setPointSize(11);
                bullet->setFont(font);
                layout->addWidget(bullet);
            }
            else if (trimmedLine.startsWith(">")) {
                auto quote = new QLabel(trimmedLine.mid(1).trimmed(), this);
                quote->setWordWrap(true);
                quote->setStyleSheet("margin-left: 10px; font-style: italic;");
                objects.push_back(quote);
                layout->addWidget(quote);
            }
            else {
                auto text = new QLabel(trimmedLine, this);
                QFont font = text->font();
                font.setPointSize(11);
                text->setFont(font);
                text->setWordWrap(true);
                objects.push_back(text);
                layout->addSpacing(5);
                layout->addWidget(text);
                layout->addSpacing(5);
            }
        } // Конец цикла for

        if (isImage) {
            /*auto text = new QLabel(site, this);
            text->setWordWrap(true);
            objects.push_back(text);
            layout->addSpacing(5);
            layout->addWidget(text);
            layout->addSpacing(5);*/
        }

        layout->addStretch(1);
        layout->setEnabled(true);
        this->setUpdatesEnabled(true);
    }

    void Gembling::clearObjects() {
        if (layout->count() > 0) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
        }
        objects.clear();
    }


    Gembling::~Gembling() {

    }
}
