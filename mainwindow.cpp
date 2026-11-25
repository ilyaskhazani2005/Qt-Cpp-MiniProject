#include "mainwindow.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStatusBar>
#include <QRegularExpression>
#include <QToolBar>
#include <QLabel>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    editor  = new QTextEdit(this);
    preview = new QTextBrowser(this);
    new MarkdownHighlighter(editor->document());

    // ── Pane wrappers with colored header bars ──────────────────────────
    auto makePaneHeader = [](const QString &dot, const QString &label) {
        QWidget *h = new QWidget;
        h->setFixedHeight(32);
        h->setStyleSheet("background:#F1F5F9; border-bottom:1px solid #E2E8F0;");
        QHBoxLayout *hl = new QHBoxLayout(h); hl->setContentsMargins(12,0,12,0); hl->setSpacing(8);
        QLabel *d = new QLabel; d->setFixedSize(8,8);
        d->setStyleSheet(QString("background:%1; border-radius:4px;").arg(dot));
        QLabel *l = new QLabel(label);
        l->setStyleSheet("color:#64748B; font-size:11px; font-weight:600; letter-spacing:0.05em; text-transform:uppercase;");
        hl->addWidget(d); hl->addWidget(l); hl->addStretch();
        return h;
    };

    QWidget *editorPane  = new QWidget; QVBoxLayout *el = new QVBoxLayout(editorPane);  el->setContentsMargins(0,0,0,0); el->setSpacing(0);
    QWidget *previewPane = new QWidget; QVBoxLayout *pl = new QVBoxLayout(previewPane); pl->setContentsMargins(0,0,0,0); pl->setSpacing(0);
    el->addWidget(makePaneHeader("#F59E0B", "MARKDOWN")); el->addWidget(editor);
    pl->addWidget(makePaneHeader("#10B981", "APERÇU"));  pl->addWidget(preview);

    // ── Splitter ────────────────────────────────────────────────────────
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(editorPane);
    splitter->addWidget(previewPane);
    splitter->setSizes({500, 500});
    splitter->setHandleWidth(4);
    splitter->setStyleSheet("QSplitter::handle { background:#E2E8F0; } QSplitter::handle:hover { background:#3B82F6; }");
    setCentralWidget(splitter);

    // ── Global stylesheet ───────────────────────────────────────────────
    this->setStyleSheet(R"(
        QMainWindow { background:#0F172A; }
        QMenuBar { background:#1E293B; border-bottom:1px solid #334155; }
        QMenuBar::item { background:transparent; color:#94A3B8; padding:6px 16px; font-weight:600; }
        QMenuBar::item:selected { background:#334155; color:#F1F5F9; border-radius:6px; }
        QMenu { background:#1E293B; color:#CBD5E1; border:1px solid #334155; border-radius:8px; padding:4px; }
        QMenu::item { padding:8px 24px; border-radius:4px; }
        QMenu::item:selected { background:#3B82F6; color:#fff; }
        QTextEdit {
            background:#FFFFFF; color:#1E293B;
            border:none;
            font-family:'Consolas','Cascadia Code','Courier New',monospace;
            font-size:14px; padding:20px;
            selection-background-color:#BFDBFE; selection-color:#1E3A8A;
        }
        QTextBrowser {
            background:#FFFFFF; color:#1E293B;
            border:none;
            font-family:'Segoe UI','Inter',sans-serif;
            font-size:14px; padding:20px;
        }
        QToolBar {
            background:#1E293B; border-bottom:1px solid #334155;
            padding:4px 8px; spacing:2px;
        }
        QToolButton {
            background:#334155; color:#CBD5E1;
            border:none; border-radius:6px;
            padding:4px 10px; font-weight:600; font-size:12px;
        }
        QToolButton:hover { background:#3B82F6; color:#fff; }
        QToolButton:pressed { background:#2563EB; }
        QStatusBar { background:#1E293B; color:#94A3B8; border-top:1px solid #334155; font-size:12px; padding:0 12px; }
    )");

    // ── Formatting Toolbar ──────────────────────────────────────────────
    QToolBar *tb = addToolBar("Formatting");
    tb->setMovable(false);

    auto addBtn = [&](const QString &label, auto slot) {
        QAction *a = tb->addAction(label);
        connect(a, &QAction::triggered, this, slot);
    };

    addBtn("B",  [this]{ wrapSelection("**", "**"); });
    addBtn("I",  [this]{ wrapSelection("*",  "*");  });
    tb->addSeparator();
    addBtn("H1", [this]{ insertHeading(1); });
    addBtn("H2", [this]{ insertHeading(2); });
    addBtn("H3", [this]{ insertHeading(3); });
    tb->addSeparator();
    addBtn("—",  [this]{ editor->insertPlainText("\n---\n"); });
    addBtn(">", [this]{ insertLinePrefix("> "); });
    addBtn("[ ]",[this]{ insertLinePrefix("- [ ] "); });

    // ── Connections ─────────────────────────────────────────────────────
    connect(editor, &QTextEdit::textChanged, this, &MainWindow::updatePreview);

    // ── Status bar ──────────────────────────────────────────────────────
    wordLabel = new QLabel("0 Mots");
    charLabel = new QLabel("0 Caractères");
    for (auto *l : {wordLabel, charLabel}) {
        l->setStyleSheet("background:#334155; color:#94A3B8; padding:2px 10px; border-radius:12px; font-size:11px; font-weight:600; margin:3px 2px;");
        statusBar()->addWidget(l);
    }
    statusBar()->addPermanentWidget(new QLabel("Groupe 17"));

    connect(editor, &QTextEdit::textChanged, this, [=]() {
        QString t = editor->toPlainText();
        int w = t.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).count();
        wordLabel->setText(QString::number(w) + " Mots");
        charLabel->setText(QString::number(t.length()) + " Caractères");
    });

    // ── Window ──────────────────────────────────────────────────────────
    resize(1100, 720);
    setWindowTitle("Gestionnaire de Notes Markdown — Groupe 17");
    createMenus();

    editor->setPlainText(
        "# 👋 Merhaba bik f l'Éditeur Markdown\n\n"
        "Had l'application tssaybat b **C++** w **Qt6**.\n\n"
        "### Chno tqder ddir biha?\n"
        "- Tkteb l'Markdown f liser.\n"
        "- Tchouf natija f l'blassa (Rendu instantané) f limen.\n"
        "- T-sauvegarder w t7el les fichiers dyalek b shoula.\n\n"
        "> *Projet POO - Groupe 17*"
        );
}

// ── Helper methods ───────────────────────────────────────────────────────
void MainWindow::wrapSelection(const QString &open, const QString &close) {
    QTextCursor c = editor->textCursor();
    QString sel = c.selectedText();
    c.insertText(open + (sel.isEmpty() ? "texte" : sel) + close);
}

void MainWindow::insertHeading(int level) {
    QTextCursor c = editor->textCursor();
    c.movePosition(QTextCursor::StartOfLine);
    c.insertText(QString(level, '#') + " ");
    editor->setTextCursor(c);
}

void MainWindow::insertLinePrefix(const QString &prefix) {
    QTextCursor c = editor->textCursor();
    c.movePosition(QTextCursor::StartOfLine);
    c.insertText(prefix);
    editor->setTextCursor(c);
}

// ── The rest stays the same ──────────────────────────────────────────────
MainWindow::~MainWindow() {}

void MainWindow::updatePreview() {
    preview->setMarkdown(editor->toPlainText());
}

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu("Fichier");
    QAction *openAct = new QAction("Ouvrir un fichier...", this);
    openAct->setShortcut(QKeySequence::Open);
    QAction *saveAct = new QAction("Enregistrer sous...", this);
    saveAct->setShortcut(QKeySequence::Save);
    fileMenu->addAction(openAct); fileMenu->addAction(saveAct);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
}

void MainWindow::openFile() {
    QString f = QFileDialog::getOpenFileName(this, "Ouvrir", "", "Markdown (*.md);;Tous (*.*)");
    if (f.isEmpty()) return;
    QFile file(f); if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) return;
    editor->setPlainText(QTextStream(&file).readAll());
}

void MainWindow::saveFile() {
    QString f = QFileDialog::getSaveFileName(this, "Enregistrer", "", "Markdown (*.md)");
    if (f.isEmpty()) return;
    QFile file(f); if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) return;
    QTextStream(&file) << editor->toPlainText();
}