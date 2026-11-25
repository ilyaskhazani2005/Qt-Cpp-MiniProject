#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QSplitter>

// 🚀 Class jdida dyal l'alwan wst l'Éditeur (Syntax Highlighting)
class MarkdownHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    MarkdownHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}
protected:
    void highlightBlock(const QString &text) override {
        // Headings
        QTextCharFormat hFmt; hFmt.setForeground(QColor("#60A5FA")); hFmt.setFontWeight(QFont::Bold);
        QRegularExpression hRe("^#{1,6}\\s.*"); auto m = hRe.match(text);
        if (m.hasMatch()) setFormat(0, text.length(), hFmt);
        // Bold
        QTextCharFormat bFmt; bFmt.setForeground(QColor("#34D399")); bFmt.setFontWeight(QFont::Bold);
        QRegularExpression bRe("\\*\\*(.+?)\\*\\*");
        auto it = bRe.globalMatch(text);
        while (it.hasNext()) { auto n = it.next(); setFormat(n.capturedStart(), n.capturedLength(), bFmt); }
        // Italic
        QTextCharFormat iFmt; iFmt.setForeground(QColor("#A78BFA")); iFmt.setFontItalic(true);
        QRegularExpression iRe("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)");
        auto it2 = iRe.globalMatch(text);
        while (it2.hasNext()) { auto n = it2.next(); setFormat(n.capturedStart(), n.capturedLength(), iFmt); }
        // Blockquote
        QTextCharFormat qFmt; qFmt.setForeground(QColor("#94A3B8"));
        if (text.startsWith(">")) setFormat(0, text.length(), qFmt);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updatePreview();
    void openFile();
    void saveFile();

private:
    void createMenus();

    // Les fonctions jdad dyal l'outils (Toolbar)
    void wrapSelection(const QString &open, const QString &close);
    void insertHeading(int level);
    void insertLinePrefix(const QString &prefix);

    QTextEdit *editor;
    QTextBrowser *preview;

    // Les variables jdad dyal l'compteur d'mots
    QLabel *wordLabel;
    QLabel *charLabel;
};

#endif // MAINWINDOW_H