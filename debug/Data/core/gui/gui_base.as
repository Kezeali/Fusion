void OnWindowLoad(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(e_String("title")).SetInnerRML(doc.GetTitle());
}

void OnEditorWindowLoad(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(e_String("title")).SetInnerRML(doc.GetTitle());

	ElementUndoMenu @undoMenu = cast<ElementUndoMenu>( doc.GetElementById(e_String("undo_menu")) );
	if (undoMenu !is null)
		editor.attachUndoMenu(undoMenu);

	@undoMenu = cast<ElementUndoMenu>( doc.GetElementById(e_String("redo_menu")) );
	if (undoMenu !is null)
		editor.attachRedoMenu(undoMenu);
}
