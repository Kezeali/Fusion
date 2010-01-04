#include "console.as"
#include "context_menu.as"

void StartGUI()
{
	Context @context = gui.getContext();
	context.LoadMouseCursor(e_String("core/gui/cursor.rml"));

	InitialiseConsole();
	CreateConsole();
}
