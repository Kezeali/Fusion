#ifndef EDITORQT_H
#define EDITORQT_H

#include <QtWidgets/QMainWindow>
#include "ui_EditorQT.h"

class EditorQT : public QMainWindow
{
	Q_OBJECT

public:
	EditorQT(QWidget *parent = 0);
	~EditorQT();

private:
	Ui::EditorQTClass ui;
};

#endif // EDITORQT_H
