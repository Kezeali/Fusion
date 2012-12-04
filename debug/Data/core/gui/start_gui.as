#include "console.as"
#include "context_menu.as"

void StartGUI()
{
	Context @context = gui.getContext();
	context.LoadMouseCursor(rString("Data/core/gui/cursor.rml"));

	InitialiseConsole();
	CreateConsole();
}
