ElementUndoMenu @editor_undoMenu = null;
ElementUndoMenu @editor_redoMenu = null;

void OnEditorWindowLoad(Event@ event)
{
	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	doc.GetElementById(rString("title")).SetInnerRML(doc.GetTitle());

	@editor_undoMenu = cast<ElementUndoMenu>( doc.GetElementById(rString("undo_menu")) );
	if (editor_undoMenu !is null)
		editor.attachUndoMenu(editor_undoMenu);

	@editor_redoMenu = cast<ElementUndoMenu>( doc.GetElementById(rString("redo_menu")) );
	if (editor_redoMenu !is null)
		editor.attachRedoMenu(editor_redoMenu);
}

void OnUndo(Event@ event)
{
	if (editor_undoMenu is null)
		return;
	//{
	//	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	//	@editor_undoMenu = cast<ElementUndoMenu>( doc.GetElementById(rString("undo_menu")) );
	//}

	editor.undo(editor_undoMenu.GetSelection());
}

void OnRedo(Event@ event)
{
	if (editor_redoMenu is null)
		return;
	//{
	//	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	//	@editor_redoMenu = cast<ElementUndoMenu>( doc.GetElementById(rString("redo_menu")) );
	//}

	editor.redo(editor_redoMenu.GetSelection());
}

void OnClickActualGFXCheckbox(Event@ event)
{
	bool enabled = event.GetParameter(rString("value"), false);
	editor.setDisplayActualSprites(enabled);
}

void OnEntityTypeChanged(Event@ event)
{
	string type = event.GetParameter(rString('value'), rString());
	editor.updateSuggestions(type);
	editor.setEntityType(type);
}

void OnClickPseudoCheckbox(Event@ event)
{
	bool pseudo = event.GetParameter(rString("value"), false);
	editor.setPlacePseudoEntities(pseudo);
}

void OnSelected(Event@ event)
{
	int index = event.GetParameter(rString("row_index"), -1);
	if (index == -1)
		return;

	string type = editor.getSuggestion(index);

	ElementDocument @doc = event.GetCurrentElement().GetOwnerDocument();
	ElementFormControlInput@ element = cast<ElementFormControlInput>( doc.GetElementById(rString("entity_type")) );
	if (element !is null)
	{
		element.SetValue(rString(type));
		editor.setEntityType(type);
	}
}

void SubmitSaveOrLoad(Event@ event)
{
	string filename = event.GetParameter(rString('filename'), rString());
	string type = event.GetParameter(rString('submit'), rString());
	if (type == "load")
		editor.load( filename );
	else if (type == "save")
		editor.save( filename );
	else
		editor.compile( filename );
}
