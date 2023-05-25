#include "Fusion3D.h"
#include <QtWidgets/QApplication>
#include "licence_api.h"

int main(int argc, char *argv[])
{
	if (isPermissionEnabled() == false)
	{
		return 0;
	}
	QApplication a(argc, argv);
	Fusion3D w;
	w.show();
	return a.exec();
}
