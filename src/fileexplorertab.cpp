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
    ui->exploreView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    auto rows = ui->exploreView->selectionModel()->selectedRows();
    if (indexes.empty()) {
        return;
    }

    QMenu menu(ui->exploreView);
    QAction* action;
    auto path = getPathFromModelIndex(indexes[0]);
    QStringList files = getAllFilesFromSelection(rows);
    QStringList dltFiles = files.filter(".dlt", Qt::CaseInsensitive);

    action = new QAction("&Open DLT/PCAP/MF4/DLF files...", this);
    connect(action, &QAction::triggered, this, [this, files]() {
        if (!files.isEmpty())
            emit filesOpenRequest(files);
    });
    menu.addAction(action);

    action = new QAction("&Append DLT/PCAP/MF4/DLF files...", this);
    connect(action, &QAction::triggered, this, [this, files]() {
        if (!files.isEmpty())
            emit filesAppendRequest(files);
    });
    menu.addAction(action);

    if (!dltFiles.isEmpty()) {
        action = new QAction("&Open dlt files in new instance", this);
        connect(action, &QAction::triggered, this, [dltFiles]() {
            QProcess process;
            process.setProgram(QCoreApplication::applicationFilePath());
            process.setArguments(dltFiles);
            process.setStandardOutputFile(QProcess::nullDevice());
            process.setStandardErrorFile(QProcess::nullDevice());
            qint64 pid;
            process.startDetached(&pid);
        });
        menu.addAction(action);
    }
    menu.addSeparator();

    if (rows.size() == 1) {
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
    }

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

QStringList FileExplorerTab::getMultiFilesFromSelection(const QModelIndexList& rows) const {
    QStringList files;
    for (const auto &row : rows) {
        files.append(m_fsModel->filePath(m_fsSortProxyModel->mapToSource(row)));
    }
    return files;
}

QStringList FileExplorerTab::getAllFilesFromSelection(const QModelIndexList& rows) const {
    QSet<QString> files;
    for (const auto &row : rows) {
        auto path = m_fsModel->filePath(m_fsSortProxyModel->mapToSource(row));
        if (QFileInfo(path).isFile()) {
            files.insert(path);
        } else {
            QDirIterator itSh(path, m_fsModel->nameFilters(), QDir::Files,
                              QDirIterator::Subdirectories);
            while (itSh.hasNext()) {
                files.insert(itSh.next());
            }
        }
    }
    QStringList result(files.begin(), files.end());
    result.sort();
    return result;
}
