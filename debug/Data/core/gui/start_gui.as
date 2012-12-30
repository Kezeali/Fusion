#include "console.as"
#include "context_menu.as"

void StartGUI()
{
	Rocket::Context @context = gui.getContext();
	context.LoadMouseCursor(Rocket::String("Data/core/gui/cursor.rml"));

	InitialiseConsole();
	CreateConsole();
}
