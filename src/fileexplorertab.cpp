#include "fileexplorertab.h"
#include "ui_fileexplorertab.h"

#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QClipboard>
#include <QUrl>
#include <QDirIterator>

FileExplorerTab::FileExplorerTab(QWidget* parent)
    : QWidget{parent},
      ui(new Ui::FileExplorerTab) {
    ui->setupUi(this);

    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setNameFilterDisables(false);
    m_fsModel->setNameFilters(QStringList() << "*.dlt" << "*.dlf" << "*.dlp" << "*.pcap" << "*.mf4");
    m_fsModel->setRootPath(QDir::rootPath());

    m_fsSortProxyModel = new SortFilterProxyModel(this);
    m_fsSortProxyModel->setSourceModel(m_fsModel);

    ui->exploreView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->exploreView->setModel(m_fsSortProxyModel);
    ui->exploreView->setSortingEnabled(true);
    ui->exploreView->sortByColumn(0, Qt::AscendingOrder);

    // hide type of file, creation date, etc
    ui->exploreView->hideColumn(1);
    ui->exploreView->hideColumn(2);
    ui->exploreView->hideColumn(3);

    // disable multiple selection
    ui->exploreView->setSelectionMode(QAbstractItemView::SingleSelection);
}

FileExplorerTab::~FileExplorerTab() {
    delete ui;
}

void FileExplorerTab::setCurrentFile(const QString& path) {
    ui->exploreView->scrollTo(m_fsSortProxyModel->mapFromSource(m_fsModel->index(path)));
}

void FileExplorerTab::on_exploreView_activated(const QModelIndex &index)
{
    QString path = getPathFromModelIndex(index);
    // send signal only for supported file extensions
    if (path.endsWith(".dlt") || path.endsWith(".dlf") || path.endsWith(".dlp") || path.endsWith(".pcap") || path.endsWith(".mf4")) {
        emit fileActivated(path);
    }
}

void FileExplorerTab::on_exploreView_customContextMenuRequested(QPoint pos) {
    auto indexes = ui->exploreView->selectionModel()->selectedIndexes();
    if (indexes.empty()) {
        return;
    }

    QMenu menu(ui->exploreView);
    QAction* action;
    auto path = getPathFromModelIndex(indexes[0]);
    if (QFileInfo(path).isFile()) {
        action = new QAction("&Open DLT/PCAP/MF4/DLF file...", this);
        connect(action, &QAction::triggered, this, [this, path]() {
            emit fileOpenRequested(path);
        });
        menu.addAction(action);

        action = new QAction("&Append DLT/PCAP/MF4/DLF file...", this);
        connect(action, &QAction::triggered, this, [this, path]() {
            emit fileAppendRequested(path);
        });
        menu.addAction(action);

        if ((!path.toLower().endsWith(".dlp")) && (5 > indexes.size())) {
            if ((path.toLower().endsWith(".dlt"))) {
                action = new QAction("&Open in new instance", this);
                connect(action, &QAction::triggered, this, [this, indexes]() {
                    auto index = indexes[0];
                    auto path = getPathFromModelIndex(index);
                    QProcess process;
                    process.setProgram(QCoreApplication::applicationFilePath());
                    process.setArguments({path});
                    process.setStandardOutputFile(QProcess::nullDevice());
                    process.setStandardErrorFile(QProcess::nullDevice());
                    qint64 pid;
                    process.startDetached(&pid);
                });
                menu.addAction(action);
            }
        }
    } else {
        action = new QAction("Open all DLT files", this);
        connect(action, &QAction::triggered, this, [this, path]() {
            QStringList files;
            QDirIterator itSh(path, QStringList() << "*.dlt", QDir::Files,
                               QDirIterator::Subdirectories);

            while (itSh.hasNext()) {
                files.append(itSh.next());
            }
            emit filesOpenRequest(files);
        });
        menu.addAction(action);

        action = new QAction("Append all PCAP/MF4 files", this);
        connect(action, &QAction::triggered, this, [this, path]() {
            QStringList files;
            QDirIterator itSh(path, QStringList() << "*.pcap" << "*.mf4", QDir::Files,
                               QDirIterator::Subdirectories);

            QStringList importFilenames;
            while (itSh.hasNext()) {
                importFilenames.append(itSh.next());
            }

            if (!importFilenames.isEmpty())
                emit filesAppendRequest(importFilenames);
        });
        menu.addAction(action);
    }
    menu.addSeparator();

    action = new QAction("&Copy paths", this);
    connect(action, &QAction::triggered, this, [this, indexes]() {
        QStringList clipboardText;
        for (auto& index : indexes) {
            if (0 == index.column()) {
                auto path = getPathFromModelIndex(index);
                clipboardText += path;
            }
        }

        QGuiApplication::clipboard()->setText(clipboardText.join("\n"));
    });
    menu.addAction(action);
    menu.addSeparator();

    action = new QAction("&Show in explorer", this);
    connect(action, &QAction::triggered, this, [this]() {
        auto index = ui->exploreView->selectionModel()->selectedIndexes()[0];
        auto path = getPathFromModelIndex(index);
#ifdef WIN32
        QProcess process;
        process.startDetached(
            QString("explorer.exe /select,%1").arg(QDir::toNativeSeparators(path)));
#else
            auto path_splitted = path.split(QDir::separator());
            path = path_splitted.mid(0, path_splitted.length()-1).join(QDir::separator());
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
#endif
    });
    menu.addAction(action);

    /* show popup menu */
    const QPoint globalPos = ui->exploreView->mapToGlobal(pos);
    menu.exec(globalPos);
}

void FileExplorerTab::on_comboBoxExplorerSortType_currentIndexChanged(int index) {
    switch (index) {
    case 1:
        m_fsSortProxyModel->changeSortingType(SortFilterProxyModel::SortType::TIMESTAMP);
        break;
    case 0:
    default:
        m_fsSortProxyModel->changeSortingType(SortFilterProxyModel::SortType::ALPHABETICALLY);
        break;
    }
}

void FileExplorerTab::on_comboBoxExplorerSortOrder_currentIndexChanged(int index) {
    m_fsSortProxyModel->changeSortingOrder(index > 0 ? Qt::DescendingOrder : Qt::AscendingOrder);
}

QString FileExplorerTab::getPathFromModelIndex(const QModelIndex& index) const {
    return m_fsModel->filePath(m_fsSortProxyModel->mapToSource(index));
}
