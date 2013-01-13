#include "stdafx.h"
#include "EditorQT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	EditorQT w;
	w.show();
	return a.exec();
}
